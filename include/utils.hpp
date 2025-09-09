#pragma once

#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> split_string(const std::string &text, const std::string &separator, bool keep_empty = false);

std::pair<std::string, std::string> split_once(const std::string &text, const std::string &separator);