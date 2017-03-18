#pragma once

#include <string>
#include <boost/shared_ptr.hpp>

namespace nc
{

namespace net
{
class Url;
}

class HttpReplyHeader
{
public:
	HttpReplyHeader();
	HttpReplyHeader(size_t code, size_t content_length, bool is_connection_closed, std::string location, std::string header_text);

	size_t getCode() const { return code_; }
	size_t getContentLength() const { return content_length_; }
	bool isConnectionClosed() const { return connection_closed_; }

	const std::string& getLocation() const { return location_; }
	const std::string& getText() const { return header_text_; }
private:
	size_t code_;
	size_t content_length_;
	bool connection_closed_;

	std::string location_;

	std::string header_text_;
};

class IHttpProvider
{
public:
	static boost::shared_ptr<IHttpProvider> create();

	virtual ~IHttpProvider() = 0 {};

	virtual HttpReplyHeader parseHeader(const char* buffer, size_t length, size_t& headerBorder) = 0;
	virtual std::string buildRequest(const net::Url& url, bool close_connection = false) = 0;
};

}