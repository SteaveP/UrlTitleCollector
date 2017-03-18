#pragma once

#include "iconnection.h"
#include "../net/url.h"

#include "iurltitlecollector.h"

#include <boost/system/error_code.hpp>

namespace nc
{

class UrlTitleExtractor
{
public:
	explicit UrlTitleExtractor(const net::Url& url, int index, IUrlTitleCollector* titleCollector = nullptr, IConnection* connection = nullptr);
	explicit UrlTitleExtractor(net::Url&& url, int index, IUrlTitleCollector* titleCollector = nullptr, IConnection* connection = nullptr);

	~UrlTitleExtractor();
	
	bool on_connected(const boost::system::error_code& error);

private:
	bool on_write(const boost::system::error_code& error, size_t bytes_transferred);
	bool on_read_header(const char* buffer, const boost::system::error_code& error, size_t bytes_transferred);
	bool on_read_body(const char* buffer, const boost::system::error_code& error, size_t bytes_transferred);

	void send_request(const std::string& request);
	std::string make_request_header(const net::Url& url, bool close_connection = false);

private:
	IConnection* connection_;
	IUrlTitleCollector* urlTitleCollector_;
	size_t recieved_content_size_;

	net::Url url_;
	int index_;
};

}
