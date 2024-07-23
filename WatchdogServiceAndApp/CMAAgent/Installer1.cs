using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration.Install;
using System.Diagnostics;
using System.Linq;
using System.ServiceProcess;
using System.Threading.Tasks;

namespace CMAAgent
{
    [RunInstaller(true)]
    public partial class Installer1 : System.Configuration.Install.Installer
    {
        private ServiceInstaller serviceInstaller;
        private ServiceProcessInstaller processInstaller;

        public Installer1()
        {
            InitializeComponent();
            processInstaller = new ServiceProcessInstaller();
            serviceInstaller = new ServiceInstaller();

            // Service will run under system account
            processInstaller.Account = ServiceAccount.LocalSystem;

            // Service Name
            serviceInstaller.ServiceName = "CMAAgent";
            serviceInstaller.StartType = ServiceStartMode.Automatic;

            // Add installers to collection. Order is not important
            Installers.Add(serviceInstaller);
            Installers.Add(processInstaller);

        }

        public override void Install(System.Collections.IDictionary stateSaver)
        {
            base.Install(stateSaver);
            SetRecoveryOptions(serviceInstaller.ServiceName);
            SetServiceStartType(serviceInstaller.ServiceName);
        }

        private void SetRecoveryOptions(string serviceName)
        {
            ExecuteCommand($"sc failure \"{serviceName}\" reset= 0 actions= restart/60000/restart/60000/restart/60000");
        }

        private void SetServiceStartType(string serviceName)
        {
            ExecuteCommand($"sc config \"{serviceName}\" start= auto");
        }

        private void ExecuteCommand(string command)
        {
            var processInfo = new ProcessStartInfo("cmd.exe", "/c " + command)
            {
                CreateNoWindow = true,
                UseShellExecute = false
            };

            var process = Process.Start(processInfo);
            process.WaitForExit();
        }

    }
}
