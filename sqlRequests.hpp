#ifndef sqlRequest_hpp
#define sqlRequest_hpp

#include <string>
#include <vector>

class SqlRequest {
private:
  std::string host, user, password, dataBase;
  int port;
  void login(int, std::string);

public:
  SqlRequest(std::string, std::string, std::string, std::string, int);
  void requestsDistribution(std::vector<std::string>);
};

#endif // !sqlRequest_hpp
