using NetFwTypeLib;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CradleWindowsAgent.Firewall
{
    public class FirewallManager
    {
        private INetFwPolicy2 firewallPolicy;

        public FirewallManager()
        {
            firewallPolicy = (INetFwPolicy2)Activator.CreateInstance(Type.GetTypeFromProgID("HNetCfg.FwPolicy2"));
        }

        public void EnsureFirewallEnabled()
        {
            if (!IsFirewallEnabled())
            {
                EnableFirewall();
            }
        }

        private bool IsFirewallEnabled()
        {
            // Check if the firewall is enabled for all profiles
            return firewallPolicy.FirewallEnabled[NET_FW_PROFILE_TYPE2_.NET_FW_PROFILE2_DOMAIN] &&
                   firewallPolicy.FirewallEnabled[NET_FW_PROFILE_TYPE2_.NET_FW_PROFILE2_PRIVATE] &&
                   firewallPolicy.FirewallEnabled[NET_FW_PROFILE_TYPE2_.NET_FW_PROFILE2_PUBLIC];
        }

        private void EnableFirewall()
        {
            // Enable the firewall for all profiles
            firewallPolicy.FirewallEnabled[NET_FW_PROFILE_TYPE2_.NET_FW_PROFILE2_DOMAIN] = true;
            firewallPolicy.FirewallEnabled[NET_FW_PROFILE_TYPE2_.NET_FW_PROFILE2_PRIVATE] = true;
            firewallPolicy.FirewallEnabled[NET_FW_PROFILE_TYPE2_.NET_FW_PROFILE2_PUBLIC] = true;
        }

        public void BlockAllOutboundTraffic()
        {
            INetFwRule blockAllRule = (INetFwRule)Activator.CreateInstance(Type.GetTypeFromProgID("HNetCfg.FwRule"));

            blockAllRule.Action = NET_FW_ACTION_.NET_FW_ACTION_BLOCK;
            blockAllRule.Direction = NET_FW_RULE_DIRECTION_.NET_FW_RULE_DIR_OUT;
            blockAllRule.Enabled = true;
            blockAllRule.InterfaceTypes = "All";
            blockAllRule.Name = "Block all outbound traffic";
            blockAllRule.Profiles = (int)NET_FW_PROFILE_TYPE2_.NET_FW_PROFILE2_ALL;

            firewallPolicy.Rules.Add(blockAllRule);
        }

        public void AllowOutboundTrafficToIP(string ipAddress)
        {
            INetFwRule allowRule = (INetFwRule)Activator.CreateInstance(Type.GetTypeFromProgID("HNetCfg.FwRule"));

            allowRule.Action = NET_FW_ACTION_.NET_FW_ACTION_ALLOW;
            allowRule.Direction = NET_FW_RULE_DIRECTION_.NET_FW_RULE_DIR_OUT;
            allowRule.Enabled = true;
            allowRule.RemoteAddresses = ipAddress;
            allowRule.InterfaceTypes = "All";
            allowRule.Name = $"Allow outbound traffic to {ipAddress}";
            allowRule.Profiles = (int)NET_FW_PROFILE_TYPE2_.NET_FW_PROFILE2_ALL;

            firewallPolicy.Rules.Add(allowRule);
        }

        public void AllowOutboundTrafficToCurrentDNS()
        {
            var dnsAddresses = GetCurrentDnsAddresses()
                .Where(ip => !ip.ToString().Contains("%")) // Filter out addresses with zone indices
                .ToArray();

            foreach (var dns in dnsAddresses)
            {
                AllowOutboundTrafficToIP(dns.ToString());
            }
        }

        private IPAddress[] GetCurrentDnsAddresses()
        {
            return NetworkInterface.GetAllNetworkInterfaces()
                .Where(ni => ni.OperationalStatus == OperationalStatus.Up)
                .SelectMany(ni => ni.GetIPProperties().DnsAddresses)
                .ToArray();
        }

        public void AllowOutboundTrafficToUrl(string url)
        {
            try
            {
                var hostAddresses = Dns.GetHostAddresses(url)
                    .Where(ip => !ip.ToString().Contains("%")) // Filter out addresses with zone indices
                    .ToArray();

                foreach (var ipAddress in hostAddresses)
                {
                    AllowOutboundTrafficToIP(ipAddress.ToString());
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Failed to resolve IP addresses for URL {url}: {ex.Message}");
            }
        }

        public void RemoveFirewallRules()
        {
            foreach (INetFwRule rule in firewallPolicy.Rules)
            {
                if (rule.Name == "Block all outbound traffic" || rule.Name.StartsWith("Allow outbound traffic to"))
                {
                    firewallPolicy.Rules.Remove(rule.Name);
                }
            }
        }
    }

}
