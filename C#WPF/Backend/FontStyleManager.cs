using IniParser.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace BLFClient.Backend
{
    public class FontSettings
    {
        public int Index;   //found in config file
//        public System.Windows.Media.FontFamily FontFamily;
        public double FontSize;
        public double FontWeight;
//        public FontStyle FontStyle_;
        public double Width;
        public double Height;
        public double Escapement;
        public double Orientation;
        public int Italic;
        public int Underline;
        public int StrikeOut;
        public int Charset;
        public double OutPrecision;
        public double ClipPrecision;
        public double Quality;
        public double PitchAndFamily;
        public string FaceName;
        /*
        Font #0=//////////////
       Height #0=-12
       Width #0=0
       Escapement #0=0
       Orientation #0=0
       Weight #0=400
       Italic #0=0
       Underline #0=0
       StrikeOut #0=0
       CharSet #0=238
       OutPrecision #0=3
       ClipPrecision #0=0
       Quality #0=0
       PitchAndFamily #0=0
       FaceName #0=Arial
       */
       public bool Load(KeyDataCollection IndexCardCollection,int pIndex)
       {
            //no font section
            if (IndexCardCollection == null)
                return false;

            Index = pIndex;

            int DataSectionsFound = 0;

            if (IndexCardCollection["Font #" + Index] != null)
                DataSectionsFound++;
            if (IndexCardCollection["Height #" + Index] != null)
            {
                Height = Double.Parse(IndexCardCollection["Height #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["Width #" + Index] != null)
            {
                Width = Double.Parse(IndexCardCollection["Width #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["Escapement #" + Index] != null)
            {
                Escapement = Double.Parse(IndexCardCollection["Escapement #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["Orientation #" + Index] != null)
            {
                Orientation = Double.Parse(IndexCardCollection["Orientation #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["Weight #" + Index] != null)
            {
                FontWeight = Double.Parse(IndexCardCollection["Weight #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["Italic #" + Index] != null)
            {
                Italic = Int32.Parse(IndexCardCollection["Italic #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["Underline #" + Index] != null)
            {
                Italic = Int32.Parse(IndexCardCollection["Underline #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["StrikeOut #" + Index] != null)
            {
                Italic = Int32.Parse(IndexCardCollection["StrikeOut #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["CharSet #" + Index] != null)
            {
                Charset = Int32.Parse(IndexCardCollection["CharSet #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["OutPrecision #" + Index] != null)
            {
                OutPrecision = Double.Parse(IndexCardCollection["OutPrecision #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["ClipPrecision #" + Index] != null)
            {
                ClipPrecision = Double.Parse(IndexCardCollection["ClipPrecision #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["Quality #" + Index] != null)
            {
                Quality = Double.Parse(IndexCardCollection["Quality #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["PitchAndFamily #" + Index] != null)
            {
                PitchAndFamily = Double.Parse(IndexCardCollection["PitchAndFamily #" + Index]);
                DataSectionsFound++;
            }
            if (IndexCardCollection["FaceName #" + Index] != null)
            {
                FaceName = IndexCardCollection["FaceName #" + Index];
                DataSectionsFound++;
            }
            if (IndexCardCollection["Size #" + Index] != null)
            {
                FontSize = Double.Parse(IndexCardCollection["Size #" + Index]);
            }

            if (DataSectionsFound != 15)
                return false;

            return true;
        }

        public void Save()
        {
            Globals.Config.SetConfig("Fonts", "Font #" + Index, "//////////////");
            Globals.Config.SetConfig("Fonts", "Height #" + Index, Height.ToString());
            Globals.Config.SetConfig("Fonts", "Width #" + Index, Width.ToString());
            Globals.Config.SetConfig("Fonts", "Escapement #" + Index, Escapement.ToString());
            Globals.Config.SetConfig("Fonts", "Orientation #" + Index, Orientation.ToString());
            Globals.Config.SetConfig("Fonts", "Weight #" + Index, FontWeight.ToString());
            Globals.Config.SetConfig("Fonts", "Italic #" + Index, Italic.ToString());
            Globals.Config.SetConfig("Fonts", "Underline #" + Index, Underline.ToString());
            Globals.Config.SetConfig("Fonts", "StrikeOut #" + Index, StrikeOut.ToString());
            Globals.Config.SetConfig("Fonts", "CharSet #" + Index, Charset.ToString());
            Globals.Config.SetConfig("Fonts", "OutPrecision #" + Index, OutPrecision.ToString());
            Globals.Config.SetConfig("Fonts", "ClipPrecision #" + Index, ClipPrecision.ToString());
            Globals.Config.SetConfig("Fonts", "Quality #" + Index, Quality.ToString());
            Globals.Config.SetConfig("Fonts", "PitchAndFamily #" + Index, PitchAndFamily.ToString());
            Globals.Config.SetConfig("Fonts", "FaceName #" + Index, FaceName.ToString());
            Globals.Config.SetConfig("Fonts", "Size #" + Index, FontSize.ToString());
        }

        public bool Compare(FontSettings fs)
        {
            if (fs.Height != Height)
                return false;
            if (fs.Width != Width)
                return false;
            if (fs.Escapement != Escapement)
                return false;
            if (fs.Orientation != Orientation)
                return false;
            if (fs.StrikeOut != StrikeOut)
                return false;
            if (fs.OutPrecision != OutPrecision)
                return false;
            if (fs.ClipPrecision != ClipPrecision)
                return false;
            if (fs.Quality != Quality)
                return false;
            if (fs.PitchAndFamily != PitchAndFamily)
                return false;
            if (fs.FontSize != FontSize)
                return false;
            if (fs.FaceName != FaceName)
                return false;
            if (fs.FontWeight != FontWeight)
                return false;
            if (fs.Italic != Italic)
                return false;
            if (fs.Charset != Charset)
                return false;
            if (fs.FontWeight != FontWeight)
                return false;
            return true;
        }
    }

    public class FontStyleManager
    {
        List<FontSettings> Settings;

        public FontStyleManager()
        {
            Settings = new List<FontSettings>();
        }

        /// <summary>
        /// Loader will make sure to not load bad data. Parse config into internal structures
        /// </summary>
        public void LoadFontSettingsFromConfig()
        {
            KeyDataCollection IndexCardCollection = Globals.Config.GetConfigSection("Fonts");
            //load all possible grid cells ( phone numbers )
            int SettingIndexLoading = 0;
            do
            {
                FontSettings fs = new FontSettings();
                if (fs.Load(IndexCardCollection, SettingIndexLoading) == false)
                    break;
                Settings.Add(fs);
                //try to load more
                SettingIndexLoading++;
            } while (true);
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Loaded " + SettingIndexLoading + " Font settings");
        }

        /// <summary>
        /// Extensions will use font settings based on an ID. This will fetch and return
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        public FontSettings GetFontSettingByIndex(int index)
        {
            foreach (var a in Settings)
                if (a.Index == index)
                    return a;
            return null;
        }

        public int InsertNewSetting(FontSettings fs)
        {
            //do we have this setting already ?
            foreach (var a in Settings)
                if (a.Compare(fs) == true)
                {
                    fs.Index = a.Index;
                    return a.Index;
                }
            //create a new unused index for it
            int MaxIndex = Settings.Count + 1;
            for( int i=0;i<MaxIndex;i++)
            {
                bool IsAlreadyUsed = false;
                foreach (var a in Settings)
                    if (a.Index == i)
                    {
                        IsAlreadyUsed = true;
                        break;
                    }
                if(IsAlreadyUsed == false)
                {
                    fs.Index = i;
                    fs.Save();
                    Settings.Add(fs);
                    Globals.Config.SetConfig("Options", "NumberofFonts", Settings.Count.ToString());
                    
                    return i;
                }
            }
            return -1;
        }

        public void ClearAllFonts()
        {
            Globals.Config.SetConfig("Options", "NumberofFonts", "0");
            Globals.Config.RemoveSection("Fonts");
            Settings.Clear();
        }

        public void SaveAllFonts()
        {
            Globals.Config.RemoveSection("Fonts");
            Globals.Config.SetConfig("Options", "NumberofFonts", Settings.Count.ToString());
            foreach(FontSettings fs in Settings)
                fs.Save();
        }
    }
}
