#include <windows.h>
#include <stdio.h>
#include <comutil.h>
#include <atlcomcli.h>
#include <netfw.h>
#include "ConfigHandler.h"
#include "Logger.h"

#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )

int stricmpp(const char* s1_Wide, const char* s2)
{
    while (s1_Wide[0] != 0 && s2[0] != 0 && tolower(s1_Wide[0]) == tolower(s2[0]))
    {
        s1_Wide +=2;
        s2++;
    }
    if (s1_Wide[0] == 0 && s2[0] == 0)
        return 0;
    return -1;
}

int CheckIfOurExe(INetFwRule* FwRule, const char *ImgName)
{
    BSTR bstrVal;
    if (SUCCEEDED(FwRule->get_ApplicationName(&bstrVal)) && bstrVal != NULL && bstrVal[0] != 0)
    {
//        wprintf(L"Application Name: %s\n", bstrVal);
        if (stricmpp((const char*)bstrVal, ImgName) == 0)
        {
            return 1;
        }
    }

/*    VARIANT_BOOL bEnabled;
    if (SUCCEEDED(FwRule->get_Enabled(&bEnabled)))
    {
        if (bEnabled)
            wprintf(L"Enabled:          TRUE\n");
        else
            wprintf(L"Enabled:          FALSE\n");
    }*/
    return 0;
}


// Instantiate INetFwPolicy2
HRESULT WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2)
{
    HRESULT hr = S_OK;

    hr = CoCreateInstance(
        __uuidof(NetFwPolicy2),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwPolicy2),
        (void**)ppNetFwPolicy2);

    if (FAILED(hr))
    {
        sLog.Log(LL_Info, __FILE__, __LINE__, "CoCreateInstance for INetFwPolicy2 failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

Cleanup:
    return hr;
}

int CheckSkipFirewallAdd(const char *ImgName)
{
    HRESULT hrComInit = S_OK;
    HRESULT hr = S_OK;

    ULONG cFetched = 0;
    CComVariant var;

    IUnknown* pEnumerator;
    IEnumVARIANT* pVariant = NULL;

    INetFwPolicy2* pNetFwPolicy2 = NULL;
    INetFwRules* pFwRules = NULL;
    INetFwRule* pFwRule = NULL;

    long fwRuleCount;

    // Initialize COM.
    hrComInit = CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    // Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
    // initialized with a different mode. Since we don't care what the mode is,
    // we'll just use the existing mode.
    if (hrComInit != RPC_E_CHANGED_MODE)
    {
        if (FAILED(hrComInit))
        {
            sLog.Log(LL_Info, __FILE__, __LINE__, "CoInitializeEx failed: 0x%08lx\n", hrComInit);
            goto Cleanup;
        }
    }

    // Retrieve INetFwPolicy2
    hr = WFCOMInitialize(&pNetFwPolicy2);
    if (FAILED(hr))
        goto Cleanup;
 
    // Retrieve INetFwRules
    hr = pNetFwPolicy2->get_Rules(&pFwRules);
    if (FAILED(hr))
    {
        sLog.Log(LL_Info, __FILE__, __LINE__, "get_Rules failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Obtain the number of Firewall rules
    hr = pFwRules->get_Count(&fwRuleCount);
    if (FAILED(hr))
    {
        sLog.Log(LL_Info, __FILE__, __LINE__, "get_Count failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    //sLog.Log(LL_Info, __FILE__, __LINE__, "The number of rules in the Windows Firewall are %d\n", fwRuleCount);

    // Iterate through all of the rules in pFwRules
    pFwRules->get__NewEnum(&pEnumerator);

    if (pEnumerator)
        hr = pEnumerator->QueryInterface(__uuidof(IEnumVARIANT), (void**)&pVariant);

    while (SUCCEEDED(hr) && hr != S_FALSE)
    {
        var.Clear();
        hr = pVariant->Next(1, &var, &cFetched);

        if (S_FALSE != hr)
        {
            if (SUCCEEDED(hr))
                hr = var.ChangeType(VT_DISPATCH);
            if (SUCCEEDED(hr))
                hr = (V_DISPATCH(&var))->QueryInterface(__uuidof(INetFwRule), reinterpret_cast<void**>(&pFwRule));

            if (SUCCEEDED(hr) && CheckIfOurExe(pFwRule, ImgName))
               return 1;
        }
    }

Cleanup:

    // Release pFwRule
    if (pFwRule != NULL)
        pFwRule->Release();

    // Release INetFwPolicy2
    if (pNetFwPolicy2 != NULL)
        pNetFwPolicy2->Release();

    // Uninitialize COM.
    if (SUCCEEDED(hrComInit))
        CoUninitialize();

    return 0;
}

void AddFirewallRule_(const char *ImgName,int Port)
{
    HRESULT hrComInit = S_OK;
    HRESULT hr = S_OK;

    INetFwPolicy2* pNetFwPolicy2 = NULL;
    INetFwRules* pFwRules = NULL;
    INetFwRule* pFwRule = NULL;

    long CurrentProfilesBitMask = 0;

    char ExeName[MAX_PATH];
    int NameLen = (int)strlen(ImgName);
    //find last slash
    for (int i = NameLen - 1; i >= 0; i--)
        if (ImgName[i] == '\\')
        {
            i++;
            int j;
            for (j = 0; ImgName[i + j] != 0; j++)
                ExeName[j] = ImgName[i + j];
            ExeName[j] = 0;
            break;
        }
    BSTR bstrRuleName = SysAllocString(CA2W(ExeName));
    BSTR bstrRuleDescription = SysAllocString(L"Allow network traffic to SOCKS 5 tunnel");
    BSTR bstrRuleGroup = SysAllocString(CA2W(ExeName));
    BSTR bstrRuleApplication = SysAllocString(CA2W(ImgName));
    char PortStr[10];
    sprintf_s(PortStr, sizeof(PortStr), "%d", Port);
    BSTR bstrRuleLPorts = SysAllocString(CA2W(PortStr));

    // Initialize COM.
    hrComInit = CoInitializeEx(0,COINIT_APARTMENTTHREADED);

    // Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
    // initialized with a different mode. Since we don't care what the mode is,
    // we'll just use the existing mode.
    if (hrComInit != RPC_E_CHANGED_MODE)
    {
        if (FAILED(hrComInit))
        {
            sLog.Log(LL_Info, __FILE__, __LINE__, "CoInitializeEx failed: 0x%08lx\n", hrComInit);
            goto Cleanup;
        }
    }

    // Retrieve INetFwPolicy2
    hr = WFCOMInitialize(&pNetFwPolicy2);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // Retrieve INetFwRules
    hr = pNetFwPolicy2->get_Rules(&pFwRules);
    if (FAILED(hr))
    {
        sLog.Log(LL_Info, __FILE__, __LINE__, "get_Rules failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Retrieve Current Profiles bitmask
/*    hr = pNetFwPolicy2->get_CurrentProfileTypes(&CurrentProfilesBitMask);
    if (FAILED(hr))
    {
        printf("get_CurrentProfileTypes failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // When possible we avoid adding firewall rules to the Public profile.
    // If Public is currently active and it is not the only active profile, we remove it from the bitmask
    if ((CurrentProfilesBitMask & NET_FW_PROFILE2_PUBLIC) && (CurrentProfilesBitMask != NET_FW_PROFILE2_PUBLIC))
        CurrentProfilesBitMask ^= NET_FW_PROFILE2_PUBLIC; */

    CurrentProfilesBitMask = NET_FW_PROFILE2_ALL;

    // Create a new Firewall Rule object.
    hr = CoCreateInstance(
        __uuidof(NetFwRule),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwRule),
        (void**)&pFwRule);
    if (FAILED(hr))
    {
        sLog.Log(LL_Info, __FILE__, __LINE__, "CoCreateInstance for Firewall Rule failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Populate the Firewall Rule object
    pFwRule->put_Name(bstrRuleName);
    pFwRule->put_Description(bstrRuleDescription);
    pFwRule->put_ApplicationName(bstrRuleApplication);
    pFwRule->put_Protocol(NET_FW_IP_PROTOCOL_ANY);
    pFwRule->put_LocalPorts(bstrRuleLPorts);
    pFwRule->put_Direction(NET_FW_RULE_DIR_IN);
    pFwRule->put_Grouping(bstrRuleGroup);
    pFwRule->put_Profiles(CurrentProfilesBitMask);
    pFwRule->put_Action(NET_FW_ACTION_ALLOW);
    pFwRule->put_Enabled(VARIANT_TRUE);

    // Add the Firewall Rule
    hr = pFwRules->Add(pFwRule);
    if (FAILED(hr))
    {
        sLog.Log(LL_Info, __FILE__, __LINE__, "Firewall Rule Add failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

Cleanup:

    // Free BSTR's
    SysFreeString(bstrRuleName);
    SysFreeString(bstrRuleDescription);
    SysFreeString(bstrRuleGroup);
    SysFreeString(bstrRuleApplication);
    SysFreeString(bstrRuleLPorts);

    // Release the INetFwRule object
    if (pFwRule != NULL)
        pFwRule->Release();

    // Release the INetFwRules object
    if (pFwRules != NULL)
        pFwRules->Release();

    // Release the INetFwPolicy2 object
    if (pNetFwPolicy2 != NULL)
        pNetFwPolicy2->Release();

    // Uninitialize COM.
    if (SUCCEEDED(hrComInit))
        CoUninitialize();
}

void AddFirewallRule()
{
    char ImagePath[MAX_PATH];
    int bytes = GetModuleFileName(NULL, ImagePath, sizeof(ImagePath));
    if (CheckSkipFirewallAdd(ImagePath) == 0)
    {
        sLog.Log(LL_Info, __FILE__, __LINE__, "Adding firewall rule");
        AddFirewallRule_(ImagePath, GetOurProxyPort());
    }
//    else
//        sLog.Log(LL_Info, __FILE__, __LINE__, "Firewall rule already added");
}