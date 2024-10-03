#include "server.hpp"
#include <iostream>

WebSocketServer::WebSocketServer()
    : acceptor_(ioc_) {
    tcp::endpoint endpoint{tcp::v4(), 8080};
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(net::socket_base::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
}

void WebSocketServer::run() {
    accept();
    ioc_.run();
}

void WebSocketServer::accept() {
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(
            &WebSocketServer::handle_accept,
            this));
}

void WebSocketServer::handle_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
        std::cerr << "Accept error: " << ec.message() << std::endl;
        return;
    }

    // Create the WebSocket session and run it
    websocket::stream<beast::tcp_stream> ws(std::move(socket));
    ws.async_accept(
        net::bind_executor(
            net::make_strand(ioc_),
            [this, &ws](beast::error_code ec) {
                if (ec) {
                    std::cerr << "WebSocket accept error: " << ec.message() << std::endl;
                    return;
                }

                // Create a buffer for reading
                beast::flat_buffer buffer;

                // Echo back the message
                ws.async_read(
                    buffer,
                    net::bind_executor(
                        net::make_strand(ioc_),
                        [this, &ws, &buffer](beast::error_code ec, std::size_t bytes_transferred) {
                            if (ec) {
                                std::cerr << "WebSocket read error: " << ec.message() << std::endl;
                                return;
                            }

                            ws.async_write(
                                buffer.data(),
                                net::bind_executor(
                                    net::make_strand(ioc_),
                                    [this, &ws](beast::error_code ec, std::size_t bytes_transferred) {
                                        if (ec) {
                                            std::cerr << "WebSocket write error: " << ec.message() << std::endl;
                                            return;
                                        }

                                        // Close the WebSocket connection
                                        ws.async_close(
                                            websocket::close_code::normal,
                                            net::bind_executor(
                                                net::make_strand(ioc_),
                                                [this](beast::error_code ec) {
                                                    if (ec) {
                                                        std::cerr << "WebSocket close error: " << ec.message() << std::endl;
                                                        return;
                                                    }

                                                    // Accept the next connection
                                                    accept();
                                                }));
                                    }));
                        }));
            }));
}
