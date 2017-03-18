#include "httpsconnection.h"

#include <iostream>
#include <boost/bind.hpp>

namespace nc
{
namespace net
{

HttpsConnection::HttpsConnection(
	boost::asio::io_service& io_service,
	boost::asio::ssl::context& context,
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
: socket_(io_service, context), strand_(io_service), connection_point_(endpoint_iterator), connected_(false)
{}

HttpsConnection::~HttpsConnection()
{
	close();
}

void HttpsConnection::connect(TConnectionCallback connectionCallback, TAnyPtr data)
{
	if (data)
		setData(data);

	boost::asio::async_connect(socket_.lowest_layer(), connection_point_,
		strand_.wrap(boost::bind(&HttpsConnection::handle_connect, getptr(), connectionCallback,
			boost::asio::placeholders::error)));
}

void HttpsConnection::close()
{
	if (connected_)
	{
		socket_.lowest_layer().close();
		connected_ = false;
	}
}

void HttpsConnection::async_read(TReadCallback callback)
{
	boost::asio::async_read(socket_, buffer_reply_, boost::asio::transfer_at_least(static_cast<size_t>(1)),
		strand_.wrap(boost::bind(&HttpsConnection::handle_read, getptr(), callback,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)));
}

void HttpsConnection::async_write(const char* message, size_t message_size, TWriteCallback callback)
{
	assert(message);
	if (message == nullptr)
		return;

	std::ostream ostr(&buffer_request_);
	ostr.write(message, message_size);

	boost::asio::async_write(socket_, buffer_request_,
		strand_.wrap(boost::bind(&HttpsConnection::handle_write, getptr(), callback,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)));
}

void HttpsConnection::handle_connect(TConnectionCallback connectionCallback, const boost::system::error_code& error)
{
	if (error)
	{
		if (bool user_error = connectionCallback(error) == false)
		{
			std::cerr << "Connect failed: " << error.message() << "\n";
			return;
		}
	}

	connected_ = true;

	socket_.async_handshake(boost::asio::ssl::stream_base::client,
		strand_.wrap(boost::bind(&HttpsConnection::handle_handshake, getptr(), connectionCallback,
			boost::asio::placeholders::error)));
}

void HttpsConnection::handle_handshake(TConnectionCallback connectionCallback, const boost::system::error_code& error)
{
	bool user_error = connectionCallback(error) == false;
	
	if (/*error || */user_error)
	{
		std::cerr << "Handshake failed: " << error.message() << "\n";
		close();
	}
}

void HttpsConnection::handle_write(TWriteCallback writeCallback, const boost::system::error_code& error, 
	size_t bytes_transferred)
{
	bool user_error = writeCallback(error, bytes_transferred) == false;

	buffer_request_.commit(bytes_transferred);
	buffer_request_.consume(bytes_transferred);

	if (/*error || */user_error)
	{
		std::cerr << "Write failed: " << error.message() << "\n";
		close();
	}
}

void HttpsConnection::handle_read(TReadCallback readCallback, const boost::system::error_code& error, 
	size_t bytes_transferred)
{
	const char* bufPtr = boost::asio::buffer_cast<const char*>(buffer_reply_.data());

	bool user_error = readCallback(bufPtr, error, bytes_transferred) == false;

	buffer_reply_.consume(bytes_transferred);

	if (/*error || */user_error)
	{
		std::cerr << "Read failed: " << error.message() << "\n";
		close();
	}
}

}
}