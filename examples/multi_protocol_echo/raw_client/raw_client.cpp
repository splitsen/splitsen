
// That code  is the copy of asio/example/cpp03/echo/blocking_tcp_echo_client.cpp
// Only handshake stage has been added

#include <boost/config.hpp>
#if defined(BOOST_WINDOWS) && !defined(_WIN32_WINNT) 
# define _WIN32_WINNT 0x05010000
#endif

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <boost/asio/buffer.hpp>

#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>

using boost::asio::ip::tcp;

enum { max_length = 1024 };

int main(int argc, char* argv[])
{
    using namespace std;

    try
    {
        // Check command line arguments.
        if (argc != 2)
        {
            cerr << "Usage: mpe_raw_client <address> <port>\n";
            cerr << "       default to 0.0.0.0 7777\n";
        }

        string address = argc != 3 ? boost::asio::ip::host_name() : argv[1];
        string port = argc != 3 ? "7777" : argv[2];

        cout << "Server address: " << address << endl;
        cout << "Server port: " << port << endl;

        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        tcp::socket s(io_service);
        boost::asio::connect(s, 
            resolver.resolve({ address.c_str(), port.c_str() }));

        // send the handshake
        static const string handshake("{21BB758F-BC6A-42C4-92B1-6B5332CEAEBF}");
        boost::asio::write(s,
            boost::asio::buffer(handshake.c_str(), handshake.length()));

        char request[max_length];
        std::cout << "Enter message: ";
        std::cin.getline(request, max_length);
        size_t request_length = strlen(request);
        boost::asio::write(s, boost::asio::buffer(request, request_length));

        char reply[max_length];
        size_t reply_length = boost::asio::read(s,
            boost::asio::buffer(reply, request_length));
        std::cout << "Reply is: ";
        std::cout.write(reply, reply_length);
        std::cout << "\n";
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}