#include "urltitleextractor.h"

#include <iostream>
#include <sstream>
#include <boost/bind.hpp>

namespace nc
{

UrlTitleExtractor::UrlTitleExtractor(net::Url&& url, int index, 
	IUrlTitleCollector* titleCollector, IConnection* connection)
: url_(std::move(url)), index_(index), urlTitleCollector_(titleCollector)
, connection_(connection), recieved_content_size_()
{}

UrlTitleExtractor::UrlTitleExtractor(const net::Url& url, int index, 
	IUrlTitleCollector* titleCollector, IConnection* connection)
: url_(url), index_(index), urlTitleCollector_(titleCollector)
, connection_(connection), recieved_content_size_()
{}

UrlTitleExtractor::~UrlTitleExtractor()
{}

bool UrlTitleExtractor::on_connected(const boost::system::error_code& error)
{
	if (error)
		return false;

	assert(connection_);
	if (connection_ == nullptr)
		return true;

	send_request(make_request_header(url_));

	return true;
}

bool UrlTitleExtractor::on_write(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (error)
	{
		std::cerr << "TitleExtractor::on_write: " << error.message() << std::endl;
		return false;
	}

	assert(connection_);
	if (connection_)
		connection_->async_read(
			boost::bind(&UrlTitleExtractor::on_read_header, this, 
				boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

	return true;
}

bool UrlTitleExtractor::on_read_header(const char* buffer, const boost::system::error_code& error, size_t bytes_transferred)
{
	if (error)
	{
		std::cerr << "TitleExtractor::on_read_header: " << error.message() << std::endl;
		return false;
	}

	assert(buffer);
	
	std::cout << "Header! get " << bytes_transferred << " bytes:\n";
	std::cout.write(buffer, bytes_transferred);
	std::cout << std::endl;

	assert(connection_);
	if (connection_)
		connection_->async_read(
			boost::bind(&UrlTitleExtractor::on_read_body, this, 
				boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

	return true;
}

bool UrlTitleExtractor::on_read_body(const char* buffer, const boost::system::error_code& error, size_t bytes_transferred)
{
	if (error)
	{
		std::cerr << "TitleExtractor::on_read_body: " << error.message() << std::endl;
		return false;
	}

	assert(buffer);

	recieved_content_size_ += bytes_transferred;

	std::cout << "Body! get " << bytes_transferred << " bytes (" << recieved_content_size_ << "):\n";
	std::cout.write(buffer, bytes_transferred);
	std::cout << std::endl;

	assert(connection_);

	size_t content_size_hardcode = 11988;
	assert(recieved_content_size_ <= content_size_hardcode);

	bool titleFounded = false;
	bool continue_reading = !titleFounded && recieved_content_size_ < content_size_hardcode;

	if (connection_ && continue_reading)
		connection_->async_read(
			boost::bind(&UrlTitleExtractor::on_read_body, this, 
				boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
	else if (urlTitleCollector_)
	{
		// TODO extract title from body
		std::string title("title");

		urlTitleCollector_->addUrl(index_, url_, titleFounded ? title : "<not found>");
	}

	return true;
}

void UrlTitleExtractor::send_request(const std::string& request)
{
	std::cout << "\tRequest: " << request << '\n';

	assert(connection_);
	if (connection_)
		connection_->async_write(request.c_str(), request.length(),
			boost::bind(&UrlTitleExtractor::on_write, this, 
				boost::placeholders::_1, boost::placeholders::_2));
}

std::string UrlTitleExtractor::make_request_header(const net::Url& url, bool close_connection /*= false*/)
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