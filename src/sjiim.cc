#include <iostream>
#include <cstdio>
#include <boost/asio.hpp>

int main()
{
    using namespace boost::asio;
    using ip::udp;
    io_service io_service;

    udp::socket socket(io_service, udp::endpoint(udp::v4(), 5555));

    while(1)
    {
    	char recv_buf[8192];
		boost::system::error_code error;
    	udp::endpoint remote_endpoint;

		int len = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint, 0, error);

		if(error && error != boost::asio::error::message_size)
			throw boost::system::system_error(error);

		printf("%.*s\n", len, recv_buf);
    }
}