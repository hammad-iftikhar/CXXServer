#pragma once

#include "Headers.h"
#include <map>
#include <optional>
#include <string>

struct Cookie
{
    std::string name;
    std::string value;
    std::string domain;
    std::string path;
    int expires;
    bool httpOnly{false};
    std::optional<int> maxAge;
    bool secure{false};
    bool sameSite{false};
};

class Cookies
{
private:
    std::map<std::string, Cookie> records;
    Headers headers;

public:
    explicit Cookies(Headers headers);
    ~Cookies();
    void set(std::string key, Cookie value);
    Cookie get(std::string key);
    std::string to_string();
};
