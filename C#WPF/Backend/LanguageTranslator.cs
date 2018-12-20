using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace BLFClient.Backend
{
    public enum LangIdEnum
    {
        LangIdUnk = -1,
        LangIdEnglish = 0,
        LangIdItalian = 1,
        LangIdGerman = 2,
    }

    public class LangStringStore
    {
        public long StringId;      // we will translate a string based on this id
        public long Context;       // just in case the string does not translate exactly due to context
        public string Str;         // the string we intend to use or to search based on it
        public LangIdEnum LangId;        // Translation is based on language Ids
    }

    public class LanguageTranslator
    {
        List<LangStringStore> LangStrings = new List<LangStringStore>();
        HashSet<long> SkippedIds = new HashSet<long>();

        public LanguageTranslator()
        {
        }

        private void CheckDuplicateAdd(LangStringStore t)
        {
            //are we skipping this string Id for some reason ?
            foreach( long it in SkippedIds)
                if(it == t.StringId)
                {
                    return;
                }
            //should we skip this string as already present ?
            foreach (LangStringStore it in LangStrings)
            {
                if (t.LangId == it.LangId && t.Context == it.Context && t.Str == it.Str && t.LangId == LangIdEnum.LangIdEnglish)
                {
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Duplicate translation for string: " + t.Str + ". Skipping id " + t.StringId.ToString() + " Assign a context for it ?");
                    SkippedIds.Add(t.StringId);
                    return;
                }
                if (t.StringId == it.StringId && t.Context == it.Context && t.LangId == it.LangId)
                {
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Duplicate StringId for string: " + t.Str + ". Skipping " + t.StringId.ToString() + " Assign a new id for it ?");
                    return;
                }
            }
            LangStrings.Add(t);
        }

        public void LoadDBFromFile(string FileName)
        {
            //ditch old content if there was any
            LangStrings.Clear();
            // file name is normal ?
            if (FileName == null || FileName.Length == 0)
                return;
            // can't load unexisting file
            if (File.Exists(FileName) == false)
                return;
            SkippedIds.Clear();
            //load the data we will use to translate the UI
            var lines = File.ReadLines(FileName);
            foreach (var line in lines)
            {
                if (line.Length <= 1 || line[0] == '/' || line[0] == '\n')
                    continue;
                try
                {
                    LangStringStore t = new LangStringStore();
                    int ValEnd1 = line.IndexOf('\t');
                    t.StringId = Int32.Parse(line.Substring(0, ValEnd1 - 0));
                    int ValEnd2 = line.IndexOf('\t', ValEnd1 + 1);
                    t.Context = Int32.Parse(line.Substring(ValEnd1 + 1, ValEnd2 - ValEnd1 - 1));
                    int ValEnd3 = line.IndexOf('\t', ValEnd2 + 1);
                    t.LangId = (LangIdEnum)Int32.Parse(line.Substring(ValEnd2 + 1, ValEnd3 - ValEnd2 - 1));
                    t.Str = line.Substring(ValEnd3 + 1);
                    CheckDuplicateAdd(t);
                }
                catch { }
            }
        }

        public LangIdEnum GetLangId()
        {
            switch (Globals.IniFile.GetConfig("Options", "Language", "Unk"))
            {
                case "ENG":
                    return LangIdEnum.LangIdEnglish;
                case "ITA":
                    return LangIdEnum.LangIdItalian;
                case "GER":
                    return LangIdEnum.LangIdGerman;
            }
            return LangIdEnum.LangIdUnk;
        }

        public string GetTranslation(string what, long Context = 0, LangIdEnum ToLang = LangIdEnum.LangIdUnk)
        {
            if (ToLang == LangIdEnum.LangIdUnk)
                ToLang = GetLangId();
            if (ToLang == LangIdEnum.LangIdUnk)
                return what;
            long SearchedStrId = -1;
            //search for the ID of this string
            foreach(LangStringStore t in LangStrings)
                if( t.LangId == LangIdEnum.LangIdEnglish && t.Str == what)
                {
                    SearchedStrId = t.StringId;
                    break;
                }
            //could not find this string in our DB
            if (SearchedStrId == -1)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "No english match. Could not translate string : " + what);
                return what;
            }
            // search the string with ID and lang we need
            foreach (LangStringStore t in LangStrings)
                if (t.StringId == SearchedStrId && t.LangId == ToLang && (Context <= 0 || t.Context == Context))
                    return t.Str;
            //could not find the translation
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "No " + ToLang.ToString() + " match. Could not translate string : " + what);
            return what;
        }

        private void TranslateOneElement(DependencyObject innerChild, LangIdEnum NewLang)
        {
            Label childTypeLabel = innerChild as Label;
            if (childTypeLabel != null && childTypeLabel.Content != null)
            {
                childTypeLabel.Content = GetTranslation(childTypeLabel.Content.ToString(), -1, NewLang);
                return;
            }
            Button childButton = innerChild as Button;
            if (childButton != null && childButton.Content != null)
            {
                childButton.Content = GetTranslation(childButton.Content.ToString(), -1, NewLang);
                return;
            }
            CheckBox childTypeCheckBox = innerChild as CheckBox;
            if (childTypeCheckBox != null && childTypeCheckBox.Content != null)
            {
                childTypeCheckBox.Content = GetTranslation(childTypeCheckBox.Content.ToString(), -1, NewLang);
                return;
            }
            RadioButton childTypeRadio = innerChild as RadioButton;
            if (childTypeRadio != null && childTypeRadio.Content != null)
            {
                childTypeRadio.Content = GetTranslation(childTypeRadio.Content.ToString(), -1, NewLang);
                return;
            }
            TextBlock childTypeTextBlock = innerChild as TextBlock;
            if (childTypeTextBlock != null && childTypeTextBlock.Text != null)
            {
                childTypeTextBlock.Text = GetTranslation(childTypeTextBlock.Text.ToString(), -1, NewLang);
                return;
            }
/*            TextBox childTypeTextbox = innerChild as TextBox;
            if (childTypeTextbox != null)
            {
                childTypeTextbox.Text = GetTranslation(childTypeTextbox.Text.ToString(), -1, NewLang);
                return;
            }*/
            HeaderedItemsControl childMenu = innerChild as HeaderedItemsControl;
            if(childMenu != null && childMenu.Header != null)
            {
                childMenu.Header = GetTranslation(childMenu.Header.ToString(), -1, NewLang);
                return;
            }
            GroupBox childGroup = innerChild as GroupBox;
            if (childGroup != null && childGroup.Header != null)
            {
                childGroup.Header = GetTranslation(childGroup.Header.ToString(), -1, NewLang);
                return;
            }
            Window childWindow = innerChild as Window;
            if (childWindow != null && childWindow.Title != null)
            {
                childWindow.Title = GetTranslation(childWindow.Title.ToString(), -1, NewLang);
                return;
            }
        }

        public static IEnumerable<T> FindVisualChildren<T>(DependencyObject depObj) where T : DependencyObject
        {
            if (depObj != null)
            {
                for (int i = 0; i < VisualTreeHelper.GetChildrenCount(depObj); i++)
                {
                    DependencyObject child = VisualTreeHelper.GetChild(depObj, i);
                    if (child != null && child is T)
                    {
                        yield return (T)child;
                    }

                    foreach (T childOfChild in FindVisualChildren<T>(child))
                    {
                        yield return childOfChild;
                    }
                }
            }
        }

        public static IEnumerable<T> FindLogicalChildren<T>(DependencyObject depObj) where T : DependencyObject
        {
            if (depObj != null)
            {
                foreach (object rawChild in LogicalTreeHelper.GetChildren(depObj))
                {
                    if (rawChild is DependencyObject)
                    {
                        DependencyObject child = (DependencyObject)rawChild;
                        if (child is T)
                        {
                            yield return (T)child;
                        }

                        foreach (T childOfChild in FindLogicalChildren<T>(child))
                        {
                            yield return childOfChild;
                        }
                    }
                }
            }
        }

        public static IEnumerable<DependencyObject> FindLogicalChildren<T1,T2,T3,T4,T5,T6,T7>(DependencyObject depObj)
        {
            if (depObj != null)
            {
                foreach (object rawChild in LogicalTreeHelper.GetChildren(depObj))
                {
                    if (rawChild is DependencyObject)
                    {
                        DependencyObject child = (DependencyObject)rawChild;
                        if ((child is T1) || (child is T2) || (child is T3) || (child is T4) || (child is T5) || (child is T6) || (child is T7))
                        {
                            yield return (DependencyObject)child;
                        }
                        foreach (DependencyObject childOfChild in FindLogicalChildren<T1,T2,T3,T4,T5,T6,T7>(child))
                        {
                            yield return childOfChild;
                        }
                    }
                }
            }
        }

        public void TranslateUIComponent(DependencyObject UIComponent)
        {
            LangIdEnum NewLang = GetLangId();
            if (NewLang == LangIdEnum.LangIdEnglish || NewLang == LangIdEnum.LangIdUnk)
                return;

            long StartTime = Environment.TickCount;

            //maybe this is a root object and does not have children
            if (UIComponent is DependencyObject)
                TranslateOneElement(UIComponent, NewLang);

            foreach (DependencyObject innerChild in FindLogicalChildren<Label, Button, CheckBox, GroupBox, RadioButton, HeaderedItemsControl, TextBlock>(UIComponent))
                TranslateOneElement(innerChild, NewLang);

            long Endtime = Environment.TickCount;
            if(Endtime - StartTime > 10)
                Globals.Logger.LogString(LogManager.LogLevels.LogLevelDebug, "Translation of component took strangely long " + (Endtime - StartTime) + " ms for " + UIComponent.ToString());
        }
    }
}
