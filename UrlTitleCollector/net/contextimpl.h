#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

// TODO this is circular include and dependency?
#include "context.h"

namespace nc
{
namespace net
{

class AsioContext::impl
{
	friend class AsioContext;

public:
	impl();
	boost::asio::io_service& get_io_service() { return io_service_; }

	boost::shared_ptr<IConnection> create_connection(const Url& url);

	void run_loop();

private:
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::resolver resolver_;
	boost::asio::ssl::context ctx_;
};

}
}