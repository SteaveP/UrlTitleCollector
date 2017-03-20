#include "httpconnection.h"

#include <iostream>
#include <boost/bind.hpp>

namespace nc
{
namespace net
{

namespace asio = boost::asio;

HttpConnection::HttpConnection(
	asio::io_service& io_service,
	asio::ip::tcp::resolver::iterator endpoint_iterator)
: socket_(io_service), strand_(io_service), connection_point_(endpoint_iterator), connected_(false)
{}

HttpConnection::~HttpConnection()
{
	close();
}

void HttpConnection::connect(TConnectionCallback connectionCallback, TAnyPtr data)
{
	if (data)
		setData(data);

	asio::async_connect(socket_, connection_point_,
		strand_.wrap(boost::bind(&HttpConnection::handle_connect, getptr(), connectionCallback,
			asio::placeholders::error)));
}

void HttpConnection::close()
{
	if (connected_)
	{
		socket_.close();
		connected_ = false;
	}
}

void HttpConnection::async_read(TReadCallback callback)
{
	asio::async_read(socket_, buffer_reply_, asio::transfer_at_least(static_cast<size_t>(1)),
		strand_.wrap(boost::bind(&HttpConnection::handle_read, getptr(), callback,
			asio::placeholders::error,
			asio::placeholders::bytes_transferred)));
}

void HttpConnection::async_write(const char* message, size_t message_size, TWriteCallback callback)
{
	assert(message);
	if (message == nullptr)
		return;

	std::ostream ostr(&buffer_request_);
	ostr.write(message, message_size);

	asio::async_write(socket_, buffer_request_,
		strand_.wrap(boost::bind(&HttpConnection::handle_write, getptr(), callback,
			asio::placeholders::error,
			asio::placeholders::bytes_transferred)));
}

void HttpConnection::handle_connect(TConnectionCallback connectionCallback, const boost::system::error_code& error)
{
	bool user_error = connectionCallback(error) == false;

	if (/*error || */user_error)
	{
		std::cerr << "Connect failed: " << error.message() << "\n";
		close();
	}
}

void HttpConnection::handle_write(TWriteCallback writeCallback, const boost::system::error_code& error, 
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

void HttpConnection::handle_read(TReadCallback readCallback, const boost::system::error_code& error, 
	size_t bytes_transferred)
{
	const char* bufPtr = asio::buffer_cast<const char*>(buffer_reply_.data());

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