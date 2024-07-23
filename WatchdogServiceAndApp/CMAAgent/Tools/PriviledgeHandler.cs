using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Security.Principal;

namespace CMAAgent.Tools
{
    public class PriviledgeHandler
    {
        private const int ERROR_SUCCESS = 0;
        private const string SE_ASSIGNPRIMARYTOKEN_NAME = "SeAssignPrimaryTokenPrivilege";
        private const string SE_INCREASE_QUOTA_NAME = "SeIncreaseQuotaPrivilege";

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern bool OpenProcessToken(IntPtr processHandle, uint desiredAccess, out IntPtr tokenHandle);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetCurrentProcess();

        [DllImport("advapi32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool LookupPrivilegeValue(string lpSystemName, string lpName, out LUID lpLuid);

        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        private static extern bool AdjustTokenPrivileges(
            IntPtr tokenHandle,
            bool disableAllPrivileges,
            ref TOKEN_PRIVILEGES newState,
            uint bufferLength,
            IntPtr previousState,
            IntPtr returnLength);

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct LUID
        {
            public uint LowPart;
            public int HighPart;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct LUID_AND_ATTRIBUTES
        {
            public LUID Luid;
            public uint Attributes;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct TOKEN_PRIVILEGES
        {
            public uint PrivilegeCount;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1)]
            public LUID_AND_ATTRIBUTES[] Privileges;
        }

        private const uint TOKEN_ADJUST_PRIVILEGES = 0x0020;
        private const uint TOKEN_QUERY = 0x0008;
        private const uint SE_PRIVILEGE_ENABLED = 0x00000002;

        public static void AdjustRequiredPriviledges()
        {
            try
            {
                GrantPrivilege(SE_ASSIGNPRIMARYTOKEN_NAME);
                GrantPrivilege(SE_INCREASE_QUOTA_NAME);

                LogWriter.WriteLog("Privileges granted successfully.");
            }
            catch (Exception ex)
            {
                LogWriter.WriteLog($"Error: {ex.Message}");
            }
        }

        private static void GrantPrivilege(string privilege)
        {
            IntPtr tokenHandle;
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, out tokenHandle))
            {
                throw new InvalidOperationException("OpenProcessToken failed with error code: " + Marshal.GetLastWin32Error());
            }

            try
            {
                LUID luid;
                if (!LookupPrivilegeValue(null, privilege, out luid))
                {
                    throw new InvalidOperationException("LookupPrivilegeValue failed with error code: " + Marshal.GetLastWin32Error());
                }

                TOKEN_PRIVILEGES tp = new TOKEN_PRIVILEGES
                {
                    PrivilegeCount = 1,
                    Privileges = new LUID_AND_ATTRIBUTES[1]
                };
                tp.Privileges[0].Luid = luid;
                tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                if (!AdjustTokenPrivileges(tokenHandle, false, ref tp, 0, IntPtr.Zero, IntPtr.Zero))
                {
                    throw new InvalidOperationException("AdjustTokenPrivileges failed with error code: " + Marshal.GetLastWin32Error());
                }

                if (Marshal.GetLastWin32Error() != ERROR_SUCCESS)
                {
                    throw new InvalidOperationException("AdjustTokenPrivileges failed with error code: " + Marshal.GetLastWin32Error());
                }
            }
            finally
            {
                CloseHandle(tokenHandle);
            }
        }

        [DllImport("kernel32.dll")]
        private static extern bool CloseHandle(IntPtr handle);
    }
}
