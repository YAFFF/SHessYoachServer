#include "sqlRequests.hpp"
#include <iostream>
#include <vector>

SqlRequest::SqlRequest(std::string host, std::string user, std::string password,
                       std::string dataBase, int port)
    : host(host), user(user), password(password), dataBase(dataBase),
      port(port) {}

void SqlRequest::requestsDistribution(std::vector<std::string> requestTokens) {
  if (requestTokens[1] == "Login") {
    login(std::stoi(requestTokens[2]), requestTokens[3]);
  }
}

void SqlRequest::login(int requestID, std::string data) {
  std::cout << "SqlRequest Login\nID: " << requestID << "\nData: " << data
            << '\n';
}
