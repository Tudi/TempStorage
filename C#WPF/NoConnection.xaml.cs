﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace BLFClient
{
    /// <summary>
    /// Interaction logic for NoConnection.xaml
    /// </summary>
    public partial class NoConnection : Window
    {
        public NoConnection()
        {
            InitializeComponent();
            this.Owner = App.Current.MainWindow;
            Globals.MultilangManager.TranslateUIComponent(this.Message);
            this.Left = this.Owner.Left + this.Owner.Width / 2 - this.Width / 2;
            this.Top = this.Owner.Top + this.Owner.Height / 2 - this.Height / 2;
        }
    }
}
