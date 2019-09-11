using System;
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
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Drawing;
using System.Windows.Threading;
using System.Runtime.InteropServices;
using System.IO;

namespace CopyFilesHere
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        Point GetMousePos(bool Relative=false)
        {
            Mouse.Capture(Application.Current.MainWindow, CaptureMode.SubTree);

            // Position of the mouse relative to the window
            var position = Mouse.GetPosition(Application.Current.MainWindow);

            Point ret;
            // Add the window position
            if (Relative == false && Application.Current != null && Application.Current.MainWindow != null) 
                ret = new Point(position.X + Application.Current.MainWindow.Left, position.Y + Application.Current.MainWindow.Top);
            else
                ret = new Point(position.X, position.Y);

//            Mouse.Capture(null);

            return ret;
        }

        string SourceDirectory = null;
        string DestinationDirectory = null;
        List<string> FilesToCopy = new List<string>();
        public MainWindow()
        {
            InitializeComponent();

            bool SkipFirst = true;
            foreach (string arg in Environment.GetCommandLineArgs())
            {
                if(SkipFirst == true)
                {
                    SkipFirst = false;
                    continue;
                }
                DestinationDirectory = arg;
            }

            //load source directory from config file
            try
            {
                SourceDirectory = File.ReadAllText(System.AppContext.BaseDirectory + "SourceDir.txt", Encoding.UTF8);
            }
            catch
            {
                MessageBox.Show("Could not load config file");
            }

            if (SourceDirectory == null)
                SourceDirectory = System.AppContext.BaseDirectory;

            this.Loaded += UserControl_Loaded;
        }

        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            Point MPos = GetMousePos();
            this.Left = MPos.X - ActionButton.ActualWidth / 2;
            this.Top = MPos.Y - ActionButton.ActualHeight / 2; 

            App.Current.MainWindow.Focus();
            this.Show();/**/

            Task.Factory.StartNew(() =>
            {
                while (true)
                {
                    this.Dispatcher.Invoke(
                        DispatcherPriority.SystemIdle,
                        new Action(() =>
                        {
                            Point curm = GetMousePos();
                            var pos = App.Current.MainWindow;
                            if (pos == null)
                                return;
                            if (curm.X < pos.Left || pos.Left + this.ActualWidth < curm.X)
                            {
                                Close();
                                return;
                            }
                            if (curm.Y < pos.Top || pos.Top + this.ActualHeight < curm.Y)
                            {
                                Close();
                                return;
                            }
                        }));
                }
            });/**/

            //periodically scan the input directory
            try
            {
                FileList.Children.Clear();
                DirectoryInfo d = new DirectoryInfo(SourceDirectory);
                FileInfo[] Files = d.GetFiles();
                double TotalHeight = ActionButton.ActualHeight;
                foreach (FileInfo file in Files)
                {
                    Label l = new Label();
                    l.Width = ActionButton.ActualWidth;
                    l.Content = file.Name;
                    FileList.Children.Add(l);
                    FilesToCopy.Add(file.Name);
                    TotalHeight += ActionButton.ActualHeight;
                }
                this.Height = TotalHeight;
            }
            catch
            {
                MessageBox.Show("Could not load source dir content");
            }
        }

        private void ActionButton_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (SourceDirectory == null || DestinationDirectory == null || FilesToCopy.Count == 0 || SourceDirectory == DestinationDirectory)
            {
                MessageBox.Show("Source directory is empty ?");
                return;
            }

            string ErrorList = "";
            foreach (var f in FilesToCopy)
            {
                var source = System.IO.Path.Combine(SourceDirectory, f);
                var destination = System.IO.Path.Combine(DestinationDirectory, f);
                try
                {
                    File.Copy(source, destination);
                }
                catch(Exception ex)
                {
                    ErrorList += "Could not copy " + source + " to " + destination + "\n";
                    ErrorList += "Error " + ex.ToString() + "\n";
                }
            }
            if(ErrorList != "" )
                MessageBox.Show(ErrorList);
            Close();
        }

        private void Window_MouseLeave(object sender, MouseEventArgs e)
        {
            Close();
        }

        private void Window_LostFocus(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
