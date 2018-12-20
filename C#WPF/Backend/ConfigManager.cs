using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using IniParser;
using IniParser.Model;
using IniParser.Parser;

namespace BLFClient.Backend
{
    //should be able to load / edit / save config files
    public class ConfigManager
    {
        IniData LoadedConfig = null;
        string LoadedFileName = null;

        ~ConfigManager()
        {
        }

        public void SetDefaultInitFileName(string NewName)
        {
            //why would you feed us bad input ?
            if (NewName == null)
                return;
            //unless otherwise, use this name when saving without a special reason
            LoadedFileName = NewName;
        }

        public bool LoadIni(string Filename)
        {
            if (File.Exists(Filename) == false)
                return false;
            LoadedFileName = Filename;
            FileIniDataParser ConfigLoader = new FileIniDataParser();
            LoadedConfig = ConfigLoader.ReadFile(Filename);
            return true;
        }

        public void SaveIni(string FileName = null)
        {
            var parser = new FileIniDataParser();
            if (LoadedConfig == null)
                LoadedConfig = new IniData();
            if (FileName == null)
            {
                if (LoadedFileName != null)
                    parser.WriteFile(LoadedFileName, LoadedConfig);
            }
            else
                parser.WriteFile(FileName, LoadedConfig);
        }

        public string GetConfig(string section, string Config, string Default = null)
        {
            //check if section exists
            if (LoadedConfig == null || LoadedConfig.Sections.Any(x => x.SectionName == section) == false || LoadedConfig[section][Config] == null)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogLevelDebug, "Could not find config " + LoadedFileName + ":" + section + ":" + Config + " = " + Default);
                return Default;
            }
            return LoadedConfig[section][Config];
        }

        public int GetConfigInt(string section, string Config, int DefaultValue)
        {
            string val = GetConfig(section, Config, null);
            if (val == null)
                return DefaultValue;
            try
            {
                return Int32.Parse(val);
            }
            catch (Exception e)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Config value integer expected, got something else : [" + section + "][" + Config + "]=" + val + " Exception : " + e.ToString());
                return DefaultValue;
            }
        }

        public double GetConfigNum(string section, string Config, double DefaultValue)
        {
            string val = GetConfig(section, Config, null);
            if (val == null)
                return DefaultValue;
            try
            {
                return Double.Parse(val);
            }
            catch (Exception e)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Config value integer expected, got something else : [" + section + "][" + Config + "]=" + val + " Exception : " + e.ToString());
                return DefaultValue;
            }
        }

        public void SetConfig(string section, string Config, string Value)
        {
            if (LoadedConfig.Sections.Any(x => x.SectionName == section) == false )
                LoadedConfig.Sections.AddSection(section);

            if (LoadedConfig[section].Any(x => x.KeyName == Config) == false )
                LoadedConfig[section].AddKey(Config, Value);
            else
                LoadedConfig[section][Config] = Value;
        }

        public void RemoveConfig(string section, string Config)
        {
            if (LoadedConfig.Sections.Any(x => x.SectionName == section) == false)
                return;
            LoadedConfig[section].RemoveKey(Config);
            if (LoadedConfig[section].Count == 0)
                LoadedConfig.Sections.RemoveSection(section);
        }

        public KeyDataCollection GetConfigSection(string section)
        {
            if (LoadedConfig.Sections.Any(x => x.SectionName == section) == false)
                return null;
            return LoadedConfig[section];
        }

        public void RenameSection(string OldSectionName, string NewSectionName)
        {
            if (LoadedConfig.Sections.Any(x => x.SectionName == OldSectionName) == false)
                return;
            //clone the old
            SectionData sd = LoadedConfig.Sections.GetSectionData(OldSectionName);
            //add the new
            LoadedConfig.Sections.SetSectionData(NewSectionName, sd);
            //delete the old
            LoadedConfig.Sections.RemoveSection(OldSectionName);
        }

        public void RemoveSection(string SectionName)
        {
            LoadedConfig.Sections.RemoveSection(SectionName);
        }

        public void RemoveSectionValue(string SectionName, string val)
        {
            KeyDataCollection SectionCollection = GetConfigSection(SectionName);
            if (SectionCollection == null)
                return;
            //iterate through all the index card names
            foreach (KeyData key in SectionCollection)
            {
                if (key.Value == val)
                    SectionCollection.RemoveKey(key.KeyName);
            }
        }

        public void CloseOpenedConfig()
        {
            LoadedFileName = null;
            LoadedConfig = null;
            LoadedConfig = new IniData();
        }

        public string GetIniName()
        {
            return LoadedFileName;
        }
    }
}
