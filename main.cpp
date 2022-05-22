#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <iostream>
#include <asio/ts/internet.hpp>
#include <utility>
#include <thread>

std::vector<char> vBuffer(20 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket) {
    socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
        [&](std::error_code ec, std::size_t length) {
            if (!ec) {
                std::cout << "\n\nRead " << length << " bytes\n\n";

                for (int i = 0; i < length; i++)
                    std::cout << vBuffer[i];

                GrabSomeData(socket);
            }
        });
}

int main () {
    std::cout << "program starts" << std::endl;

    asio::error_code ec;

    asio::io_context context;
    asio::io_context::work idle_work(context);
    std::thread thr_context = std::thread([&]() {context.run(); });

    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("74.125.141.94", ec), 80);
    asio::ip::tcp::socket socket(context);

    socket.connect(endpoint, ec);

    if (!ec) {
        std::cout << "connected" << std::endl;
    }
    else {
        std::cout << "failed to connect to address" << ec.message() << std::endl;
    }

    if (socket.is_open()) {
        GrabSomeData(socket);

        std::string sRequest = 
            "GET /index.html HTTP/1.1.\r\n"
            "HOST: google.ca\r\n"
            "Connection: close\r\n\r\n";

        socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(20000ms);

        context.stop();
        if (thr_context.joinable()) thr_context.join();
    }

    std::cout << "program ends peacefully" << std::endl;
    return 0;
}