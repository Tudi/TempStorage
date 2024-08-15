using FileExplorer;
using System;
using System.Collections.Generic;
using System.Drawing;
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
        private Dictionary<string, ListViewItem> previousDeviceDetails = new Dictionary<string, ListViewItem>();

        public Form1()
        {
            InitializeComponent();
            InitializeTimer();
            InitializeButtons();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            GetUSBDevices();
        }

        private void InitializeTimer()
        {
            timer = new Timer();
            timer.Interval = Int32.Parse(ConfigReader.GetConfigValue("DeviceStatusRefreshInterval"));
            timer.Tick += new EventHandler(Timer_Tick);
            timer.Start();
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
            RefreshDeviceList();
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

        private void Timer_Tick(object sender, EventArgs e)
        {
            RefreshDeviceList();
        }

        private void RefreshDeviceList()
        {
            listView1.Items.Clear(); // Clear the existing items
            GetUSBDevices();         // Refresh the list
        }

        private void GetUSBDevices()
        {
            var searcher = new ManagementObjectSearcher(@"Select * From Win32_PnPEntity Where PNPDeviceID Like '%USB%'");
            var currentDeviceIds = new HashSet<string>();

            foreach (var device in searcher.Get())
            {
                var mo = (ManagementObject)device;
                string name = mo["Name"]?.ToString() ?? "Unknown";
                string Manufacturer = mo["Manufacturer"]?.ToString() ?? "Unknown";

                if (Manufacturer.ToLower() != "Apple, Inc.".ToLower())
                {
                    continue;
                }
//                if (name.Contains("Apple iPhone") || name.Contains("Apple iPad"))
                if (name.ToLower() != "Apple Mobile Device USB Composite Device".ToLower())
                {
                    continue;
                }

                string deviceId = mo["DeviceID"]?.ToString() ?? "Unknown";
                string serialNumber = GetSerialNumber(mo);
                var item = new ListViewItem(new[] { name, Manufacturer, serialNumber, deviceId });

                // Check if the device is new or existing
                if (previousDeviceDetails.ContainsKey(deviceId))
                {
                    if (previousDeviceDetails[deviceId].BackColor != Color.Green)
                    {
                        Plugable_DeviceManager.OnDevicePluggedIn(serialNumber);
                    }
                    item.BackColor = Color.Green; // Existing device
                }
                else
                {
                    item.BackColor = Color.Yellow; // New device
                }

                listView1.Items.Add(item);
                currentDeviceIds.Add(deviceId);
                previousDeviceDetails[deviceId] = item;
            }

            // Highlight removed devices in red
            var removedDeviceIds = new HashSet<string>(previousDeviceDetails.Keys);
            removedDeviceIds.ExceptWith(currentDeviceIds);

            foreach (var deviceId in removedDeviceIds)
            {
                if (previousDeviceDetails.TryGetValue(deviceId, out var item))
                {
                    item.BackColor = Color.Red;
                    listView1.Items.Add(item);
                }
            }

            // Update previousDeviceDetails to only keep current devices
            foreach (var deviceId in removedDeviceIds)
            {
                Plugable_DeviceManager.OnDeviceUnPlugged(previousDeviceDetails[deviceId].SubItems[2].Text);
                previousDeviceDetails.Remove(deviceId);
            }
        }

        private string GetSerialNumber(ManagementObject device)
        {
            // Try different properties that might hold the serial number
            string serial = "";
            try
            {
                serial = device["SerialNumber"]?.ToString();
            }
            catch (Exception)
            {
            }
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
