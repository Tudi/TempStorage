using System;
using System.Collections.Generic;
using System.IO;

public static class ConfigReader
{
    private static Dictionary<string, string> _settings;

    static ConfigReader()
    {
        _settings = new Dictionary<string, string>();
        try
        {
            LoadConfiguration("config.ini");
        }
        catch
        {
            try
            {
                LoadConfiguration("../config.ini");
            }
            catch
            {
                try
                {
                    LoadConfiguration("../../config.ini");
                }
                catch
                {
                    LoadConfiguration("../../../config.ini");
                }
            }
        }
    }

    private static void LoadConfiguration(string filePath)
    {
        if (File.Exists(filePath))
        {
            LogWriter.WriteLog("ConfigReader : reading config file : " + filePath);

            foreach (var line in File.ReadAllLines(filePath))
            {
                if (!string.IsNullOrWhiteSpace(line) && line.Contains('='))
                {
                    int separatorIndex = line.IndexOf('=');
                    if (separatorIndex > 0)
                    {
                        var key = line.Substring(0, separatorIndex).Trim();
                        var value = line.Substring(separatorIndex + 1).Trim();

                        if (!_settings.ContainsKey(key))
                        {
                            _settings[key] = value;
                        }
                    }
                }
            }

            LogWriter.WriteLog("ConfigReader : done loading " + _settings.Count.ToString() + " config settings ");
        }
        else
        {
            throw new FileNotFoundException($"Configuration file '{filePath}' not found.");
        }
    }

    public static string GetConfigValue(string key, string defaultVal = null)
    {
        if (_settings.TryGetValue(key, out string value))
        {
            return value;
        }
        if (defaultVal != null)
        {
            return defaultVal;
        }
        throw new KeyNotFoundException($"Key '{key}' not found in the configuration file.");
    }
    public static int GetConfigValueInt(string key, int defaultVal)
    {
        if (_settings.TryGetValue(key, out string value))
        {
            if (int.TryParse(value, out int int_out))
            {
                return int_out;
            }
        }
        return defaultVal;
    }
}
