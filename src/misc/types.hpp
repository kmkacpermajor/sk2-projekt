#pragma once
#include <string>
#include <deque>
#include <functional>
#include <map>

using paramDeque = std::deque<std::string>;
using commandFunction = std::function<std::string(paramDeque)>;
using commandFunctionMap = std::map<std::string, commandFunction>;
