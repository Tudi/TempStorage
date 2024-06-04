#pragma once

#include <map>
#include <string>

// Define a struct to represent an INI file
typedef struct IniFile {
    std::map<std::string, std::map<std::string, std::string>> sections;
}IniFile;

/// <summary>
/// Get the content of an INI file and store it in a map of maps
/// </summary>
/// <param name="filename"></param>
/// <returns></returns>
IniFile parseIniFile(const std::string& filename);

/// <summary>
/// Write a map of maps as an INI file. 
/// If you loaded this ini file, all comments and empty lines will be lost
/// </summary>
/// <param name="ini"></param>
/// <param name="filename"></param>
void writeIniFile(const IniFile& ini, const std::string& filename);
