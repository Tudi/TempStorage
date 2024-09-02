#include "stdafx.h"
#include <UserEnv.h>
#include <AclAPI.h>
#include <Sddl.h>
#include <memory>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

HRESULT DLPLaunch(const wchar_t* pProgram, 
const wchar_t* pCmdLine, 
FeatureFlags::FeatureFlag flags
                      )
{
	const wstring program(pProgram) ;
	wchar_t* cmdLine(_wcsdup(pCmdLine));
	
	const wstring& str = program;
	OutputDebugStringW((L"[WPLauncher] " + wstring(program)+ L"\n").c_str());
	OutputDebugStringW((L"[WPLauncher] " + wstring(cmdLine) + L"\n").c_str());

	STARTUPINFOEX si = {0};
	PROCESS_INFORMATION pi;
	
	TCHAR buffer[2048];
	::ZeroMemory(&buffer,sizeof buffer);
	::ZeroMemory(&si, sizeof si);
	::ZeroMemory(&pi, sizeof pi);

//setup environment variables for new process
	vector<wstring> variables;
	StringCbPrintf(buffer, sizeof buffer, _T("WPDLP_FLAGS=%d"), flags);
	variables.emplace_back(buffer);

	TCHAR currentDirectory[2048];
	GetCurrentDirectory(sizeof currentDirectory /sizeof currentDirectory[0], currentDirectory);
	TCHAR path[2048];
	GetEnvironmentVariable(_T("PATH"), path, sizeof path);
	StringCbPrintf(buffer, sizeof buffer, _T("PATH=%s;%s"), currentDirectory, path);
	variables.emplace_back(buffer);
	
	LPVOID envBlock;
	AddEnvironmentVariables(variables, envBlock);
	vector<wstring> vb(variables.begin(), variables.end());
	for (wstring v : vb)
	{
		auto pos = v.find(_T('='));
		auto name = v.substr(0, pos);
		if (GetEnvironmentVariable(name.c_str(), buffer, sizeof buffer / sizeof buffer[0]))
		{
			auto value = v.substr(pos + 1);
			SetEnvironmentVariable(name.c_str(), value.c_str());
			vb.erase(std::remove(vb.begin(), vb.end(), v), vb.end());
		}
	}

	size_t variablesSize = 0;
	for (wstring v : vb)
	{
		variablesSize += v.size() + 1;
	}

	PWCHAR oldEnvBlock = ::GetEnvironmentStrings();

	PWCHAR endOfOldEnvBlock = oldEnvBlock;
	while (*endOfOldEnvBlock)
	{
		endOfOldEnvBlock += _tcslen(endOfOldEnvBlock) + 1;
	}

	unsigned long long oldEnvBlockSize = abs(endOfOldEnvBlock - oldEnvBlock) * sizeof TCHAR;
	unsigned long long envBlockSize = oldEnvBlockSize + variablesSize * sizeof TCHAR + sizeof TCHAR;

	LPVOID newEnvBlock = static_cast<LPVOID>(::LocalAlloc(0, envBlockSize));
	::ZeroMemory(newEnvBlock, envBlockSize);

	memcpy_s(newEnvBlock, oldEnvBlockSize, oldEnvBlock, oldEnvBlockSize);

	char* endOfNewEnvBlock = static_cast<char*>(newEnvBlock) + oldEnvBlockSize;
	for (wstring v : vb)
	{
		const auto vSize = v.size() * sizeof TCHAR;
		memcpy_s(endOfNewEnvBlock, vSize, v.c_str(), vSize);
		endOfNewEnvBlock += vSize + sizeof TCHAR;
	}
	if (!::FreeEnvironmentStrings(oldEnvBlock))
	{
		return FALSE;
	}
  
  //setup process attributes
	{
		BOOL bbool_;
		SIZE_T attributeListLength = 0;
		DWORD count = 0;
		if (flags & FeatureFlags::BNO_ISOLATION) {
      count++;
    }
		if (flags & FeatureFlags::APP_CONTAINER) count++;
		DWORD attributesCount = count;
		if (!InitializeProcThreadAttributeList(nullptr, attributesCount, 0, &attributeListLength) && GetLastError() !=
			ERROR_INSUFFICIENT_BUFFER)
		{
			bool_ = FALSE;
			goto bool__initialization_finish;
		}
		si.lpAttributeList = static_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(LocalAlloc(0, attributeListLength));
		if (si.lpAttributeList == nullptr)
		{
			bool_ = FALSE;
			goto bool__initialization_finish;
		}
		if (!InitializeProcThreadAttributeList(si.lpAttributeList, attributesCount, 0, &attributeListLength))
		{
			bool_ = FALSE;
			goto bool__initialization_finish;
		}
		if (flags & FeatureFlags::APP_CONTAINER)
		{
			//setup appcontainer profile
			HRESULT hr = S_OK;
			do
			{
					PSID appContainerSid;
					hr = ::CreateAppContainerProfile(appContainerInfo.Name.c_str(), appContainerInfo.Name.c_str(), appContainerInfo.Name.c_str(), nullptr,
						0, &appContainerSid);
					if (FAILED(hr))
					{
						// see if AppContainer SID already exists
						hr = ::DeriveAppContainerSidFromAppContainerName(appContainerInfo.Name.c_str(), &appContainerSid);
						if (FAILED(hr))
						{
							break;
						}
					}

					PWSTR str;
					::ConvertSidToStringSid(appContainerSid, &str);

					PWSTR path;
					if (SUCCEEDED(::GetAppContainerFolderPath(str, &path)))
					{
						::CoTaskMemFree(path);
					}


					// build process attributes
					// for simplicity (for now), have just one  capabilities
					auto* sc = new SECURITY_CAPABILITIES{ nullptr };//todo free
					sc->AppContainerSid = appContainerSid;

					::LocalFree(str);

					if (!SetSecurityCapabilities(appContainerSid, appContainerInfo.Capabilities, sc))
					{
						hr = E_FAIL;
						break;
					}

					if (!::UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES, sc,
						sizeof(SECURITY_CAPABILITIES), nullptr, nullptr))
					{
						OutputDebugString(_T("UpdateProcThreadAttribute PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES failed"));
						hr = E_FAIL;
						break;
					}


					for (wstring f : appContainerInfo.AllowedPaths)
					{
						if (!f.empty())
						{
							if (!AllowNamedObjectAccess(appContainerSid, const_cast<PWSTR>(f.c_str()), SE_FILE_OBJECT, FILE_ALL_ACCESS))
							{
								OutputDebugString(L"AllowNamedObjectAccess failed");
							}
						}
					}

					for (wstring r : appContainerInfo.AllowedRegistry)
					{
						if (!r.empty())
						{
							if (!AllowNamedObjectAccess(appContainerSid, const_cast<PWSTR>(r.c_str()), SE_REGISTRY_WOW64_32KEY,
								KEY_ALL_ACCESS))
							{
								OutputDebugString(_T("AllowNamedObjectAccess failed"));
							}
						}
					}

			} while (false);
			if (FAILED(hr))
			{
				InitializeProcThreadAttributeList(si.lpAttributeList, --attributesCount, 0, &attributeListLength);
			}
		}
		bool_ = TRUE;
	bool__initialization_finish:
		if (!bool_)
		{
			OutDebugString(_T("SetProcessAttributes failed"));
		}
	}
  
  //create process
	si.StartupInfo.cb = sizeof si;
	if (!::CreateProcess(program.c_str(),
	                                    cmdLine,
	                                    nullptr,
	                                    nullptr,
	                                    FALSE,
	                                    CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT,
	                                    envBlock,
	                                    nullptr,
	                                    reinterpret_cast<LPSTARTUPINFOW>(&si),
	                                    &pi
	))
	{
		const auto rc = ::GetLastError();

		LPWSTR message_buffer = nullptr;
		const size_t size = ::FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&message_buffer), 0, nullptr);

		wstring errorMessage(message_buffer, size);
    	::OutputDebugStringW(errorMessage.c_str());
		//Free the buffer.
		::LocalFree(message_buffer);
		
		::DeleteProcThreadAttributeList(si.lpAttributeList);
		::LocalFree(si.lpAttributeList);
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	::CloseHandle(pi.hProcess);
	::ResumeThread(pi.hThread);

	::DeleteProcThreadAttributeList(si.lpAttributeList);
	::LocalFree(si.lpAttributeList);
	return S_OK;
}

