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
using System.Windows.Shapes;

namespace BLFClient
{
    /// <summary>
    /// Interaction logic for IndexCardRename.xaml
    /// </summary>
    public partial class IndexCardRename : Window
    {
        long EditedGUID;
        public IndexCardRename(IndexCard sender)
        {
            InitializeComponent();
            EditedGUID = sender.GetGUID();
            this.IndexCardNewName.Text = sender.GetName();
            this.IndexCardNewName.Focus();
            this.IndexCardNewName.SelectAll();

            //if we push enter we presume we pushed button "ok"
            RoutedCommand firstSettings = new RoutedCommand();
            firstSettings.InputGestures.Add(new KeyGesture(Key.Enter, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(firstSettings, Button_AcceptRename));
            RoutedCommand SecondSettings = new RoutedCommand();
            SecondSettings.InputGestures.Add(new KeyGesture(Key.Escape, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(SecondSettings, Button_DiscardRename));

            Globals.MultilangManager.TranslateUIComponent(this);
            this.Owner = App.Current.MainWindow;
            this.Left = this.Owner.Left + this.Owner.Width / 2 - this.Width / 2;
            this.Top = this.Owner.Top + this.Owner.Height / 2 - this.Height / 2;
        }

        private void Button_AcceptRename(object sender, RoutedEventArgs e)
        {
            //check for forbidden strings
            if(this.IndexCardNewName.Text == "Options" || this.IndexCardNewName.Text == "Folders" || this.IndexCardNewName.Text == "Fonts" || this.IndexCardNewName.Text == "Windowposition" )
            {
                MessageBox.Show(Globals.MultilangManager.GetTranslation("Error : Name is already used"), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            //check if name is already used. This would cause issues when saving
            IndexCard RenameWho = Globals.FolderManager.IndexCardGet(EditedGUID);
            IndexCard AlreadyTaken = Globals.FolderManager.IndexCardGet(this.IndexCardNewName.Text);
            if (AlreadyTaken != null && AlreadyTaken != RenameWho)
            {
                MessageBox.Show(Globals.MultilangManager.GetTranslation("Error : Name is already used"), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            if(RenameWho!=null)
                RenameWho.SetName(this.IndexCardNewName.Text);
            this.Close();
        }

        private void Button_DiscardRename(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
