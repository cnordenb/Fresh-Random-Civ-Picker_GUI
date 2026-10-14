#pragma once

#include <fstream>
#include <sstream>
#include <string>

// The data file the program ships with (data.json is the only place its editions, DLCs and civs are written down),
// found from this header's place in the repository, whatever folder the tests run in.
inline std::string ReadProgramDataFile()
{
    const std::string header = __FILE__;
    const std::string folder = header.substr(0, header.find_last_of("\\/") + 1);
    std::ifstream file(folder + "..\\Fresh-Random-Civ-Picker_CPPGUI\\data.json", std::ios::binary);
    std::stringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

// The data file with its first occurrence of one text replaced by another, to make a file with one problem.
inline std::string ProgramDataFileWith(const std::string& original, const std::string& replacement)
{
    std::string data = ReadProgramDataFile();
    const size_t found = data.find(original);
    if (found != std::string::npos) data.replace(found, original.size(), replacement);
    return data;
}
