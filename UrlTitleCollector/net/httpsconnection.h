#pragma once

#include "../data/iconnection.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace nc
{

namespace net
{
	
class HttpsConnection : public IConnection, public boost::enable_shared_from_this<HttpsConnection>
{
public:
	HttpsConnection(
		boost::asio::io_service& io_service,
		boost::asio::ssl::context& context,
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator
	);

	virtual ~HttpsConnection();

	virtual void connect(TConnectionCallback connectionCallback, TAnyPtr data = TAnyPtr()) override;
	virtual void close() override;

	virtual void setData(TAnyPtr data) override { data_ = data; }

	virtual void async_read(TReadCallback callback) override;
	virtual void async_write(const char* message, size_t message_size, TWriteCallback callback) override;

private:
	void handle_connect(TConnectionCallback connectionCallback, const boost::system::error_code& error);
	void handle_handshake(TConnectionCallback connectionCallback, const boost::system::error_code& error);
	void handle_write(TWriteCallback writeCallback, const boost::system::error_code& error, size_t bytes_transferred);
	void handle_read(TReadCallback readCallback, const boost::system::error_code& error, size_t bytes_transferred);

	boost::shared_ptr<HttpsConnection> getptr() { return shared_from_this(); }

private:
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
	boost::asio::streambuf buffer_request_;
	boost::asio::streambuf buffer_reply_;

	boost::asio::ip::tcp::resolver::iterator connection_point_;

	bool connected_;

	TAnyPtr data_;
};

}
}