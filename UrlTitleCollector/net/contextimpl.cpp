#include "contextimpl.h"

#include <iostream>

#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>
#include <boost/algorithm/string/case_conv.hpp>


#include "url.h"
#include "httpsconnection.h"
#include "httpconnection.h"

namespace nc
{
namespace net
{

namespace asio = boost::asio;
	
AsioContext::impl::impl()
try
	: resolver_(io_service_), ctx_(asio::ssl::context::sslv23_client)
{
	boost::system::error_code error;
	ctx_.load_verify_file("ca-bundle.crt", error);
	if (error)
	{
		std::cerr << "load of ssl certification file failed! disable verifying\n";
		ctx_.set_verify_mode(asio::ssl::verify_none);
	}
	else
		ctx_.set_verify_mode(asio::ssl::verify_peer);
}
catch (...)
{
	// TODO
	throw;
}


boost::shared_ptr<nc::IConnection> AsioContext::impl::create_connection(const Url& url)
{
	if (url.isValid() == false)
		return boost::shared_ptr<nc::IConnection>();

	std::string protocol = boost::to_lower_copy(url.getProtocol());
	bool useHttps = protocol == "https" || protocol.empty();
	const char* port = useHttps ? "443" : "80";

	asio::ip::tcp::resolver::query query(url.getHost(), port);
	asio::ip::tcp::resolver::iterator iterator = resolver_.resolve(query);

	if (useHttps)
		return boost::make_shared<HttpsConnection>(io_service_, ctx_, iterator);
	else
		return boost::make_shared<HttpConnection>(io_service_, iterator);
}

void AsioContext::impl::run_loop()
{
	try
	{
		io_service_.run();
	}
	catch (std::exception& e)
	{
		std::cerr << boost::this_thread::get_id() << " exception: " << e.what() << "\n";
	}
}

}
}