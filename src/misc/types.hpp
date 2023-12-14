#pragma once
#include <deque>
#include <functional>
#include <map>
#include <string>

using paramDeque = std::deque<std::string>;
using commandStringFunction = std::function<std::string(paramDeque)>;
using commandVoidFunction = std::function<void(paramDeque)>;
using commandStringFunctionMap = std::map<std::string, commandStringFunction>;
using commandVoidFunctionMap = std::map<std::string, commandVoidFunction>;