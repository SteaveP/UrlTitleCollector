#pragma once

#include "iconnection.h"
#include "../net/url.h"

#include "iurltitlecollector.h"
#include "ihttpprovider.h"
#include "htmlparser.h"

#include <boost/system/error_code.hpp>

namespace nc
{

class UrlTitleExtractor
{
public:
	explicit UrlTitleExtractor(const net::Url& url, int index, IUrlTitleCollector* titleCollector = nullptr, 
		IHttpProvider* httpProvider = nullptr, IHtmlParser* htmlParser = nullptr, 
		IConnectionFactory* connectionFactory = nullptr, IConnection* connection = nullptr);
	explicit UrlTitleExtractor(net::Url&& url, int index, IUrlTitleCollector* titleCollector = nullptr, 
		IHttpProvider* httpProvider = nullptr, IHtmlParser* htmlParser = nullptr, 
		IConnectionFactory* connectionFactory = nullptr, IConnection* connection = nullptr);

	~UrlTitleExtractor();
	
	bool on_connected(const boost::system::error_code& error);

private:
	bool on_write(const boost::system::error_code& error, size_t bytes_transferred);
	bool on_read_header(const char* buffer, const boost::system::error_code& error, size_t bytes_transferred);

	bool on_read_body(const char* buffer, const boost::system::error_code& error, size_t bytes_transferred);
	bool on_skip_body(const char* buffer, const boost::system::error_code& error, size_t bytes_transferred);

	bool redirect();

	bool build_and_send_request(const std::string& url_text);
	void send_request(const std::string& request);

	bool dispatch_reply(const HttpReplyHeader& replyHeader);
	bool parse_title();

private:
	IConnection* connection_;
	IConnectionFactory* connectionFactory_;
	IUrlTitleCollector* urlTitleCollector_;
	IHttpProvider* httpProvider_;
	IHtmlParser* htmlParser_;
	size_t recieved_body_size_;
	
	std::string buffer_;

	HttpReplyHeader reply_header_;
	net::Url url_;
	int index_;
};

}
