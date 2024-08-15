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
        }
        else
        {
            throw new FileNotFoundException($"Configuration file '{filePath}' not found.");
        }
    }

    public static string GetConfigValue(string key)
    {
        if (_settings.TryGetValue(key, out string value))
        {
            return value;
        }
        throw new KeyNotFoundException($"Key '{key}' not found in the configuration file.");
    }
}
