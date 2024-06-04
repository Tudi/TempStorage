#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "InitFileHandler.h"

// Function to parse an INI file
IniFile parseIniFile(const std::string& filename) {
    IniFile ini;
    std::ifstream file(filename);

    if (!file.is_open()) {
        return ini;
    }

    std::string currentSection;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue; // Skip empty lines

        // comment line
        if (line[0] == '#') {
            continue;
        }
        else if (line[0] == '[' && line[line.length() - 1] == ']') {
            currentSection = line.substr(1, line.length() - 2);
        }
        else {
            size_t equalsPos = line.find('=');
            if (equalsPos != std::string::npos) {
                std::string key = line.substr(0, equalsPos);
//                std::transform(key.begin(), key.end(), key.begin(),
//                    [](unsigned char c) { return (unsigned char)std::tolower(c); });
                std::string value = line.substr(equalsPos + 1);
                ini.sections[currentSection][key] = value;
            }
        }
    }

    return ini;
}

//IniFile ini;
//ini.sections["section1"]["key1"] = "value1";
void writeIniFile(const IniFile& ini, const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    for (const auto& sectionPair : ini.sections) {
        file << "[" << sectionPair.first << "]\n";

        for (const auto& keyValuePair : sectionPair.second) {
            file << keyValuePair.first << "=" << keyValuePair.second << "\n";
        }

        file << "\n"; // Add a blank line between sections
    }

    file.close();
}
