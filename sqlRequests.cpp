#include "sqlRequests.hpp"
#include "requestParser.hpp"
#include <iostream>
#include <string>
#include <vector>

SqlRequest::SqlRequest(std::string dbAccess) {
  try {
    connection = pqxx::connection(dbAccess);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}

void SqlRequest::requestsDistribution(std::vector<std::string> requestTokens) {
  if (requestTokens[1] == "Login") {
    login(std::stoi(requestTokens[2]), requestTokens[3]);
  } else if (requestTokens[1] == "AddUser") {
    addUser(std::stoi(requestTokens[2]), requestTokens[3]);
  } else if (requestTokens[1] == "RemoveUser") {
    removeUser(std::stoi(requestTokens[2]), requestTokens[3]);
  }
}

void SqlRequest::login(int requestID, std::string data) {
  std::cout << "SqlRequest Login\nID: " << requestID << "\nData: " << data
            << '\n';

  std::vector<std::string> tokens = RequestParser(data, ",").getTokens();
  if (connection.is_open()) {
    std::cout << "Opened database successfully: " << connection.dbname()
              << '\n';

    pqxx::work work(connection);
    pqxx::result result =
        work.exec("SELECT login, password FROM users WHERE login = '" +
                  tokens[0] + "' AND password = '" + tokens[1] + "'");
    if (result.size() > 0) {
      std::cout << "Login successful.\n";
    } else {
      std::cout << "Login failed. Invalid login or password.\n";
    }
  } else {
    std::cerr << "Can't open database" << std::endl;
  }
}

void SqlRequest::addUser(int requestID, std::string data) {
  std::cout << "SqlRequest AddUser\nID: " << requestID << "\nData: " << data
            << '\n';

  std::vector<std::string> tokens = RequestParser(data, ",").getTokens();
  if (connection.is_open()) {
    std::cout << "Opened database successfully: " << connection.dbname()
              << '\n';
    pqxx::work work(connection);

    pqxx::result result =
        work.exec("SELECT login FROM users WHERE login = '" + tokens[0] + "'");
    if (result.size() == 0) {
      work.exec("INSERT INTO users (login, password) VALUES ('" + tokens[0] +
                "', '" + tokens[1] + "')");
      work.commit();

      std::cout << "User added successfully.\n";
    } else {
      std::cout << "Login already exists.\n";
    }
  } else {
    std::cerr << "Can't open database\n";
  }
}

void SqlRequest::removeUser(int requestID, std::string data) {
  std::cout << "SqlRequest RemoveUser\nID: " << requestID << "\nData: " << data
            << '\n';

  std::vector<std::string> tokens = RequestParser(data, ",").getTokens();
  if (connection.is_open()) {
    std::cout << "Opened database successfully: " << connection.dbname()
              << '\n';
    pqxx::work work(connection);

    pqxx::result result =
        work.exec("SELECT login FROM users WHERE login = '" + tokens[0] + "'");
    if (result.size() > 0) {
      work.exec("DELETE FROM users WHERE login = '" + tokens[0] + "'");
      work.commit();
      std::cout << "User deleted successfully.\n";
    } else {
      std::cout << "Login does not exist.\n";
    }
  } else {
    std::cerr << "Can't open database\n";
  }
}
