#include <map>
#include <string>

#include "include/Cookies.h"
#include "include/Headers.h"

Cookies::Cookies(Headers headers)
{
    this->headers = headers;
}

Cookies::~Cookies() {}

void Cookies::set(std::string key, Cookie value)
{
    records[key] = value;
}

Cookie Cookies::get(std::string key)
{
    return records[key];
}

std::string Cookies::to_string()
{
    std::string result = "";

    for (auto &record : records)
    {
        result += record.first + ": " + record.second.value + "\r\n";
    }

    return result;
}