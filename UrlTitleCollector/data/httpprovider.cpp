#include "httpprovider.h"

#include <sstream>
#include <algorithm>

#include <boost/make_shared.hpp>

#include "../net/url.h"

namespace
{
const std::string border_str{ "\r\n\r\n" };
}

namespace nc
{
	
boost::shared_ptr<IHttpProvider> IHttpProvider::create()
{
	return boost::make_shared<HttpProvider>();
}

HttpReplyHeader::HttpReplyHeader()
	: HttpReplyHeader(0u, 0u, "", "")
{}

HttpReplyHeader::HttpReplyHeader(size_t code, size_t content_length, std::string location, std::string header_text)
	: code_(code), content_length_(content_length), location_(location), header_text_(header_text)
{}

HttpProvider::HttpProvider()
{

}

HttpReplyHeader HttpProvider::parseHeader(const char* buffer, size_t length, size_t& headerBorder)
{
	std::string header_text(buffer, length);

	headerBorder = header_text.find(border_str);

	if (headerBorder == std::string::npos)
		return HttpReplyHeader();

	// check header
	std::istringstream istr(header_text.substr(0, headerBorder));

	size_t code = 0, content_length = -1;
	std::string location;

	std::string line, key, value;
	// read first line
	if (std::getline(istr, line))
	{
		std::istringstream iss_line(line);

		iss_line >> key >> code;

		if (key.find("HTTP/") != 0 || !iss_line)
			return HttpReplyHeader();
	}
	// read other lines
	while (std::getline(istr, line))
	{
		// TODO trim \r from rightside
		std::istringstream iss_line(line);
		iss_line >> key;

		std::transform(begin(key), end(key), begin(key), ::tolower);

		if (key.find("content-length") == 0)
		{
			iss_line >> content_length;
		}
		else if (key.find("location") == 0)
		{
			std::getline(iss_line, location);
		}
	}

	if (istr.fail() && !istr.eof())
		return HttpReplyHeader();
	
	// find real header border
	headerBorder += border_str.length();

	return HttpReplyHeader(
		std::move(code), std::move(content_length), 
		std::move(location), std::move(header_text.substr(0, headerBorder))
	);
}

std::string HttpProvider::buildRequest(const net::Url& url, bool close_connection /*= false*/)
{
	std::ostringstream s;
	s << "GET " << url.getPath() << " HTTP/1.0\r\n";
	s << "Host: " << url.getHost() << "\r\n";
	s << "Accept: */*\r\n";

	if (close_connection)
		s << "Connection: close\r\n";
	else
		s << "Connection: keep-alive\r\n";
	
	s << "\r\n";

	return s.str();
}

}