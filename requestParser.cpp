#include "requestParser.hpp"
#include <cstddef>
#include <string>
#include <vector>

RequestParser::RequestParser(std::string s, std::string delimiter) {
  size_t pos = 0;
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    tokens.push_back(token);
    s = s.substr(pos + delimiter.length());
  }
  tokens.push_back(s);
}

std::vector<std::string> RequestParser::getTokens() { return tokens; }
