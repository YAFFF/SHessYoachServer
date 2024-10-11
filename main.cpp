#include "requestParser.hpp"
#include "sqlRequests.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

void do_session(tcp::socket socket) {
  SqlRequest sqlRequests("dbname=SHessYoachDB user=postgres password=120804 "
                         "hostaddr=0.0.0.0 port=5432");
  try {

    std::cout << "New connection from " << socket.remote_endpoint() << "\n";
    websocket::stream<tcp::socket> ws{std::move(socket)};

    ws.set_option(
        websocket::stream_base::decorator([](websocket::response_type &res) {
          res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING) +
                                           " websocket-server-sync");
        }));

    ws.accept();

    for (;;) {
      beast::flat_buffer buffer;

      ws.read(buffer);
      std::string message = beast::buffers_to_string(buffer.data());
      std::cout << "Received message: " << message << "\n";

      std::string response = "";
      std::vector<std::string> tokens = RequestParser(message, ";").getTokens();
      if (!tokens.empty()) {
        if (tokens[0] == "/sql") {
          sqlRequests.requestsDistribution(tokens);
        }
      }

      std::cout << "Sending response: " << response << "\n";
      ws.text(ws.got_text());
      ws.write(net::buffer(response));
    }
  } catch (beast::system_error const &se) {
    if (se.code() != websocket::error::closed)
      std::cerr << "Error: " << se.code().message() << std::endl;
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}

int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: websocket-server-sync <address> <port>\n"
                << "Example:\n"
                << "    websocket-server-sync 0.0.0.0 8080\n";
      return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

    net::io_context ioc{1};

    tcp::acceptor acceptor{ioc, {address, port}};
    for (;;) {
      tcp::socket socket{ioc};

      acceptor.accept(socket);

      std::cout << "Accepted connection from " << socket.remote_endpoint()
                << "\n";
      std::thread(&do_session, std::move(socket)).detach();
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
