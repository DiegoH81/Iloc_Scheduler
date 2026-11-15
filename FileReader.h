#ifndef FILE_READER_H
#define FILE_READER_H

#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>

class FileReader
{
public:
    FileReader(std::filesystem::path in_path):
        main_path(in_path)
    {}

    std::vector<std::string> get_lines(const std::string& in_file_name)
    {
        std::filesystem::path current_path = main_path / in_file_name;

        std::vector<std::string> output;
        std::string current_buffer;

        std::ifstream file(current_path);

        if (!file.is_open())
            return output;

        while(std::getline(file, current_buffer))
        {
            if (current_buffer.empty())
                continue;

            output.push_back(current_buffer);
        }

        return output;
    }
    
private:
    std::filesystem::path main_path;
};

#endif