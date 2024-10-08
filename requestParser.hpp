#ifndef requestParser_hpp
#define requestParser_hpp

#include <string>
#include <vector>

class RequestParser {
private:
  std::vector<std::string> tokens;

public:
  RequestParser(std::string, std::string);
  std::vector<std::string> getTokens();
};

#endif // !parser_hpp
