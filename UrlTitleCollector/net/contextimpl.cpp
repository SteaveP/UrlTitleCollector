#include "contextimpl.h"

#include <iostream>

#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>


#include "url.h"
#include "httpsconnection.h"

namespace nc
{
namespace net
{
	
AsioContext::impl::impl()
try
	: resolver_(io_service_), ctx_(boost::asio::ssl::context::sslv23_client)
{
	boost::system::error_code error;
	ctx_.load_verify_file("ca-bundle.crt", error);
	if (error)
	{
		std::cerr << "load of ssl certification file failed! disable verifying\n";
		ctx_.set_verify_mode(boost::asio::ssl::verify_none);
	}
	else
		ctx_.set_verify_mode(boost::asio::ssl::verify_peer);
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

	// TODO transform to lower case before compare
	bool useHttps = true; //url.getProtocol() == "https" || url.getProtocol().empty();
	const char* port = useHttps ? "443" : "80";

	boost::asio::ip::tcp::resolver::query query(url.getHost(), port);
	boost::asio::ip::tcp::resolver::iterator iterator = resolver_.resolve(query);

	if (useHttps)
		return boost::dynamic_pointer_cast<nc::IConnection>(
			boost::make_shared<HttpsConnection>(io_service_, ctx_, iterator));
	else
		// TODO support http protocol
		throw std::exception("http connection not implemented yet");
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