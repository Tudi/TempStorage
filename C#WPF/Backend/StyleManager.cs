using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace BLFClient.Backend
{
    public class StyleManager
    {
        public static void RuntimeSkinning_Loaded(object sender, RoutedEventArgs e)
        {
            if (Globals.AppVars.SkinFileName == "")
                return;
            Window WND = sender as Window;
            if (WND == null)
                return;
            //get rid of old styles
            WND.Resources.Clear();
            WND.Resources.MergedDictionaries.Clear();
            //load the new style
            object t = Application.LoadComponent(new Uri("/BLFClient;component/Skins/" + Globals.AppVars.SkinFileName, UriKind.Relative));
            ResourceDictionary skin = t as ResourceDictionary;
            //apply the new style
            WND.Resources.MergedDictionaries.Add(skin);
        }
    }
}
