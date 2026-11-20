#pragma once

#include <fstream>
#include <sstream>
#include <string>

inline std::string ReadProgramDataFile()
{
    const std::string header = __FILE__;
    const std::string folder = header.substr(0, header.find_last_of("\\/") + 1);
    std::ifstream file(folder + "..\\Fresh-Random-Civ-Picker_CPPGUI\\data.json", std::ios::binary);
    std::stringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

inline std::string ProgramDataFileWith(const std::string& original, const std::string& replacement)
{
    std::string data = ReadProgramDataFile();
    const size_t found = data.find(original);
    if (found != std::string::npos) data.replace(found, original.size(), replacement);
    return data;
}
