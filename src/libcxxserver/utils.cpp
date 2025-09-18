#include <string>

#include "include/utils.h"

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

std::string to_lowercase(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return text;
}

// Decode percent-encoded strings and '+' to space
std::string url_decode(const std::string &encoded)
{
    std::string result;
    result.reserve(encoded.size());
    for (size_t i = 0; i < encoded.size(); ++i)
    {
        char c = encoded[i];
        if (c == '+')
        {
            result.push_back(' ');
        }
        else if (c == '%' && i + 2 < encoded.size())
        {
            auto hex_to_int = [](char ch) -> int
            {
                if (ch >= '0' && ch <= '9')
                    return ch - '0';
                if (ch >= 'A' && ch <= 'F')
                    return ch - 'A' + 10;
                if (ch >= 'a' && ch <= 'f')
                    return ch - 'a' + 10;
                return -1;
            };
            int hi = hex_to_int(encoded[i + 1]);
            int lo = hex_to_int(encoded[i + 2]);
            if (hi >= 0 && lo >= 0)
            {
                char decoded = static_cast<char>((hi << 4) | lo);
                result.push_back(decoded);
                i += 2;
            }
            else
            {
                // invalid percent-encoding, keep as-is
                result.push_back(c);
            }
        }
        else
        {
            result.push_back(c);
        }
    }
    return result;
}

std::string trim(std::string text)
{
    size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start])))
        start++;
    size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])))
        end--;
    return text.substr(start, end - start);
};