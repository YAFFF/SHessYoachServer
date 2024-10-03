#include "server.hpp"
#include <iostream>

int main() {
  try {
    WebSocketServer server;
    server.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
