using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace BLFClient.Backend
{
    public class StyleManager
    {
        static ResourceDictionary skin = null;
        static int PhoneNumberUsesGradient = 0;
        static int StatusBarUsesGradient = 0;
        static Color GradientFadeToColor;
        public static void RuntimeSkinning_Loaded(object sender, RoutedEventArgs e)
        {
            if (Globals.AppVars.SkinFileName == "")
                return;
            Window WND = sender as Window;
            if (WND == null)
                WND = Application.Current.MainWindow;
            if (WND == null)
                return;
            //get rid of old styles
            WND.Resources.Clear();
            WND.Resources.MergedDictionaries.Clear();
            //load the new style
            object t = Application.LoadComponent(new Uri("/BLFClient;component/Skins/" + Globals.AppVars.SkinFileName, UriKind.Relative));
            skin = t as ResourceDictionary;
            //apply the new style
            WND.Resources.MergedDictionaries.Add(skin);
            PhoneNumberUsesGradient = 0;
            StatusBarUsesGradient = 0;
            GradientFadeToColor = Color.FromRgb(0,0,0);
        }

        public static bool PhoneNumberHasGradientBackground()
        {
            if (skin == null)
                return false;
            if (PhoneNumberUsesGradient == 1)
                return true;
            if (PhoneNumberUsesGradient == 2)
                return false;
            Style MyBorderStyle = skin["ExtensionCellBorder"] as Style;
            if (MyBorderStyle == null)
            {
                PhoneNumberUsesGradient = 2;
                return false;
            }
            foreach (var setter in MyBorderStyle.Setters.OfType<Setter>())
            {
                if (setter.Value is System.Windows.Media.LinearGradientBrush)
                {
                    PhoneNumberUsesGradient = 1;
                    return true;
                }
            }
            PhoneNumberUsesGradient = 2;
            return false;
        }

        public static bool StatusBarHasGradientBackground()
        {
            if (skin == null)
                return false;
            if (StatusBarUsesGradient == 1)
                return true;
            if (StatusBarUsesGradient == 2)
                return false;
            Style MyBorderStyle = skin["StatusbarItem"] as Style;
            if (MyBorderStyle == null)
            {
                StatusBarUsesGradient = 2;
                return false;
            }
            foreach (var setter in MyBorderStyle.Setters.OfType<Setter>())
            {
                if (setter.Value is System.Windows.Media.LinearGradientBrush)
                {
                    StatusBarUsesGradient = 1;
                    return true;
                }
            }
            StatusBarUsesGradient = 2;
            return false;
        }

        public static Color GetGradientFadeToColor()
        {
            if (skin == null)
                return GradientFadeToColor;
            if (GradientFadeToColor.R != 0)
                return GradientFadeToColor;
            Style MyBorderStyle = skin["GradientColorFadeTo"] as Style;
            if (MyBorderStyle == null)
            {
                GradientFadeToColor.R = 1;
                return GradientFadeToColor;
            }
            foreach (var setter in MyBorderStyle.Setters.OfType<Setter>())
            {
                if (setter.Value is System.Windows.Media.SolidColorBrush)
                {
                    GradientFadeToColor = (setter.Value as System.Windows.Media.SolidColorBrush).Color;
                    return GradientFadeToColor;
                }
                return GradientFadeToColor;
            }
            GradientFadeToColor.R = 1;
            return GradientFadeToColor;
        }
    }
}
