#pragma once

#include <string>
#include <vector>

std::vector<std::string> split_string(const std::string &text, const std::string &separator, bool keep_empty = false);

std::pair<std::string, std::string> split_once(const std::string &text, const std::string &separator);

std::string to_lowercase(std::string text);

// URL decode helper for application/x-www-form-urlencoded
std::string url_decode(const std::string &encoded);

std::string trim(std::string text);