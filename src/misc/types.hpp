#pragma once
#include <deque>
#include <functional>
#include <string>
#include <unordered_map>

using paramDeque = std::deque<std::string>;
using commandStringFunction = std::function<std::string(paramDeque)>;
using commandStringFunctionMap =
    std::unordered_map<std::string, commandStringFunction>;