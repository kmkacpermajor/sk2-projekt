#pragma once
#include <string>
#include <deque>
#include <functional>
#include <map>

using paramDeque = std::deque<std::string>;
using commandStringFunction = std::function<std::string(paramDeque)>;
using commandVoidFunction = std::function<void(paramDeque)>;
using commandStringFunctionMap = std::map<std::string, commandStringFunction>;
using commandVoidFunctionMap = std::map<std::string, commandVoidFunction>;