#pragma once
#include <algorithm>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

inline std::string &rtrim(std::string &s, const char *t = " \t\n\r\f\v") {
  s.erase(s.find_last_not_of(t) + 1);
  s.erase(std::find(s.begin(), s.end(), '\0'), s.end());
  return s;
}

inline std::string &ltrim(std::string &s, const char *t = " \t\n\r\f\v") {
  s.erase(0, s.find_first_not_of(t));
  return s;
}

inline std::string &trim(std::string &s, const char *t = " \t\n\r\f\v") {
  return ltrim(rtrim(s, t), t);
}

inline bool isStringInVector(std::vector<std::string> v, std::string s) {
  if (std::find(v.begin(), v.end(), s) != v.end()) return true;
  return false;
}

template <typename... Args>
inline std::string string_format(const std::string &format, Args... args) {
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
  if (size_s <= 0) {
    throw std::exception("Error during formatting.");
  }
  auto size = static_cast<size_t>(size_s);
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(),
                     buf.get() + size - 1);  // We don't want the '\0' inside
}