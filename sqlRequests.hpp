#ifndef sqlRequest_hpp
#define sqlRequest_hpp

#include <pqxx/pqxx>
#include <string>
#include <vector>

class SqlRequest {
private:
  void login(int, std::string);
  void addUser(int, std::string);
  void removeUser(int, std::string);

  pqxx::connection connection;

public:
  SqlRequest(std::string);
  void requestsDistribution(std::vector<std::string>);
};

#endif // !sqlRequest_hpp
