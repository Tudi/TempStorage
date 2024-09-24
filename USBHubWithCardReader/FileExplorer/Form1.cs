using FileExplorer;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Management;
using System.Windows.Forms;

namespace USBDeviceInfoAPI
{
    public partial class Form1 : Form
    {
        private Timer timer;
        private Button btnRefresh;
        private Button btnStartStop;
        private bool isTimerRunning = true;

        public Form1()
        {
            InitializeComponent();
            InitializeTimer();
            InitializeButtons();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            RefreshDeviceList(sender, e);
        }

        private void InitializeButtons()
        {
            // Initialize Refresh Button
            btnRefresh = new Button();
            btnRefresh.Text = "Refresh";
            btnRefresh.Location = new System.Drawing.Point(12, 450);
            btnRefresh.Click += new EventHandler(BtnRefresh_Click);
            this.Controls.Add(btnRefresh);

            // Initialize Start/Stop Button
            btnStartStop = new Button();
            btnStartStop.Text = "Stop Auto-Refresh";
            btnStartStop.Location = new System.Drawing.Point(100, 450);
            btnStartStop.Click += new EventHandler(BtnStartStop_Click);
            this.Controls.Add(btnStartStop);
        }

        private void BtnRefresh_Click(object sender, EventArgs e)
        {
            RefreshDeviceList(sender, e);
        }

        private void BtnStartStop_Click(object sender, EventArgs e)
        {
            if (isTimerRunning)
            {
                timer.Stop();
                btnStartStop.Text = "Start Auto-Refresh";
            }
            else
            {
                timer.Start();
                btnStartStop.Text = "Stop Auto-Refresh";
            }
            isTimerRunning = !isTimerRunning;
        }

        private void InitializeTimer()
        {
            timer = new Timer();
            timer.Interval = ConfigReader.GetConfigValueInt("DeviceStatusRefreshInterval", 1000);
            timer.Tick += new EventHandler(RefreshDeviceList);
            timer.Start();
        }

        private Dictionary<string, ListViewItem> previousDeviceDetails = new Dictionary<string, ListViewItem>();
        private void RefreshDeviceList(object sender, EventArgs e)
        {
            var searcher = new ManagementObjectSearcher(@"Select * From Win32_PnPEntity Where PNPDeviceID Like '%USB%'");
            var currentDeviceIds = new HashSet<string>();

            foreach (var device in searcher.Get())
            {
                var mo = (ManagementObject)device;
                string name = mo["Name"]?.ToString() ?? "Unknown";
                string Manufacturer = mo["Manufacturer"]?.ToString() ?? "Unknown";

                if (!Manufacturer.Equals("Apple, Inc.", StringComparison.OrdinalIgnoreCase))
                {
                    continue;
                }
                if (!name.Equals("Apple Mobile Device USB Composite Device", StringComparison.OrdinalIgnoreCase))
                {
                    continue;
                }

                string deviceId = mo["DeviceID"]?.ToString() ?? "Unknown";
                string serialNumber = GetSerialNumber(mo);

                // Check if the device is new or existing
                if (previousDeviceDetails.TryGetValue(deviceId, out var existingItem))
                {
                    if (existingItem.BackColor != Color.Green)
                    {
                        existingItem.BackColor = Color.Green;
                        Plugable_DeviceManager.OnDevicePluggedIn(serialNumber);
                    }
                }
                else
                {
                    var item = new ListViewItem(new[] { name, Manufacturer, serialNumber, deviceId });
                    item.BackColor = Color.Yellow; // New device
                    listView1.Items.Add(item);
                    previousDeviceDetails[deviceId] = item;
                }

                currentDeviceIds.Add(deviceId);
            }

            // Highlight removed devices in red
            var removedDeviceIds = new HashSet<string>(previousDeviceDetails.Keys);
            removedDeviceIds.ExceptWith(currentDeviceIds);

            foreach (var deviceId in removedDeviceIds)
            {
                if (previousDeviceDetails.TryGetValue(deviceId, out var item))
                {
                    if (item.BackColor != Color.Red)
                    {
                        item.BackColor = Color.Red;
                    }
                    else
                    {
                        Plugable_DeviceManager.OnDeviceUnPlugged(previousDeviceDetails[deviceId].SubItems[2].Text);
                        previousDeviceDetails.Remove(deviceId);
                        listView1.Items.Remove(item);
                    }
                }
            }
        }

        private string GetSerialNumber(ManagementObject device)
        {
            // Try different properties that might hold the serial number
//            string serial = "";
            string serial = device?.Properties.Cast<PropertyData>().Any(p => p.Name == "SerialNumber") == true
    ? device["SerialNumber"]?.ToString() ?? string.Empty
    : string.Empty;
/*            try
            {
                serial = device["SerialNumber"]?.ToString();
            }
            catch (Exception)
            {
            }*/
            if (string.IsNullOrEmpty(serial))
            {
                try
                {
                    serial = device["DeviceID"]?.ToString();

                    // Split the deviceId string by the backslash character
                    string[] parts = serial.Split('\\');

                    // The last part of the split result is the serial number
                    serial = parts[parts.Length - 1];

                    // Remove '&' characters from the serial number. This is a generic case and not expected for Apple devices
                    serial = serial.Replace("&", string.Empty);

                    // Add a hyphen after the 8th character
                    if (serial.Length > 8)
                    {
                        serial = serial.Insert(8, "-");
                    }
                    return serial ?? "Unknown";
                }
                catch (Exception)
                {
                    return "Unknown";
                }
            }
            return "Unknown";
        }
    }
}
