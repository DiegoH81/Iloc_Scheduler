#ifndef HELPERS_H
#define HELPERS_H

#include <vector>
#include <string>
#include <sstream>

namespace help
{
    std::vector<std::string> get_lines_from_string(const std::string& input_text)
    {
        std::vector<std::string> output;
        std::stringstream ss(input_text);
        std::string current_buffer;

        while (std::getline(ss, current_buffer))
        {
            if (current_buffer.empty())
                continue;

            output.push_back(current_buffer);
        }

        return output;
    }

    void replace(std::string& in_str, char in_char, char to_replace = ' ')
    {
        for (auto& c : in_str)
        {
            if (c == in_char)
                c = to_replace;
        }
    }

    std::vector<std::string> parse_string(const std::string& in_string)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(in_string);
        std::string token;

        while (ss >> token)
            tokens.push_back(token);

        return tokens;
    }
}

#endif