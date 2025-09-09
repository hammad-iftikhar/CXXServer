#include "include/Headers.h"
#include <string>
#include <sys/socket.h>

Headers::Headers()
{
    records = std::map<std::string, std::string>();
}

Headers::~Headers()
{
}

void Headers::set(std::string key, std::string value)
{
    records[key] = value;
}

std::string Headers::get(std::string key)
{
    return records[key];
}

std::string Headers::to_string()
{
    std::string result = "";

    for (auto &record : records)
    {
        result += record.first + ": " + record.second + "\r\n";
    }

    return result;
}