#include <string>

#include "utils.hpp"

std::vector<std::string> split_string(const std::string &text, const std::string &separator, bool keep_empty)
{
    std::vector<std::string> tokens;
    if (separator.empty())
    {
        tokens.push_back(text); // no valid separator
        return tokens;
    }

    size_t start = 0, end;

    while ((end = text.find(separator, start)) != std::string::npos)
    {
        if (keep_empty || end > start)
        {
            tokens.push_back(text.substr(start, end - start));
        }
        start = end + separator.length();
    }

    if (start < text.size())
    {
        tokens.push_back(text.substr(start));
    }
    else if (keep_empty && !text.empty())
    {
        tokens.push_back("");
    }

    return tokens;
}

// Split string by first occurence only
std::pair<std::string, std::string> split_once(const std::string &text, const std::string &separator)
{
    size_t pos = text.find(separator);
    if (pos == std::string::npos)
    {
        // Separator not found then return whole string as first part
        return {text, ""};
    }

    std::string left = text.substr(0, pos);
    std::string right = text.substr(pos + separator.size());
    return {left, right};
}