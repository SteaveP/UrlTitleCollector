#include "urltitleextractor.h"

#include <iostream>
#include <sstream>
#include <boost/bind.hpp>

namespace nc
{

UrlTitleExtractor::UrlTitleExtractor(net::Url&& url, int index, 
	IUrlTitleCollector* titleCollector, IHttpProvider* httpProvider, IHtmlParser* htmlParser, IConnection* connection)
: url_(std::move(url)), index_(index), urlTitleCollector_(titleCollector), htmlParser_(htmlParser)
, httpProvider_(httpProvider), connection_(connection), recieved_content_size_(0)
{}

UrlTitleExtractor::UrlTitleExtractor(const net::Url& url, int index, 
	IUrlTitleCollector* titleCollector, IHttpProvider* httpProvider, IHtmlParser* htmlParser, IConnection* connection)
: url_(url), index_(index), urlTitleCollector_(titleCollector), htmlParser_(htmlParser)
, httpProvider_(httpProvider), connection_(connection), recieved_content_size_(0)
{}

UrlTitleExtractor::~UrlTitleExtractor()
{}

bool UrlTitleExtractor::on_connected(const boost::system::error_code& error)
{
	if (error)
	{
		if (urlTitleCollector_)
			urlTitleCollector_->addUrl(index_, url_, "<connection failed>");

		return false;
	}

	assert(connection_);
	if (connection_ == nullptr || httpProvider_ == nullptr)
		return false;

	send_request(httpProvider_->buildRequest(url_));

	return true;
}

bool UrlTitleExtractor::on_write(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (error)
	{
		std::cerr << "TitleExtractor::on_write: " << error.message() << std::endl;

		if (urlTitleCollector_)
			urlTitleCollector_->addUrl(index_, url_, "<request failed>");

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
	// TODO handle eof error
	if (error)
	{
		std::cerr << "TitleExtractor::on_read_header: " << error.message() << std::endl;

		if (urlTitleCollector_)
			urlTitleCollector_->addUrl(index_, url_, "<read header failed>");

		return false;
	}

	// NOTE first read might not receive whole headers

	assert(httpProvider_);
	assert(connection_);
	assert(buffer);

	if (httpProvider_ == nullptr)
		return false;

	buffer_.insert(buffer_.end(), buffer, buffer + bytes_transferred);

	size_t header_border = -1;
	reply_header_ = httpProvider_->parseHeader(buffer_.c_str(), buffer_.length(), header_border);

	if (header_border == -1)
	{
		// read until whole header is received
		if (connection_)
			connection_->async_read(boost::bind(&UrlTitleExtractor::on_read_header, this,
				boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

		return true;
	}

	// remove header from buffer
	buffer_.erase(begin(buffer_), begin(buffer_) + header_border);
	recieved_content_size_ += buffer_.length();
	
	if (reply_header_.getCode() == 200)
	{
		if (connection_)
			connection_->async_read(boost::bind(&UrlTitleExtractor::on_read_body, this,
				boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
	}
//	else if (reply_header_.getCode() >= 300 && reply_header_.getCode() < 400)
// 	{
// 		// TODO redirect
// 	}
	else
	{
		if (urlTitleCollector_)
			urlTitleCollector_->addUrl(index_, url_, "<code " + std::to_string(reply_header_.getCode()) + ">");

		return false;
	}

	return true;
}

bool UrlTitleExtractor::on_read_body(const char* buffer, const boost::system::error_code& error, size_t bytes_transferred)
{
	// TODO handle eof error
	if (error)
	{
		std::cerr << "TitleExtractor::on_read_body: " << error.message() << std::endl;

		if (urlTitleCollector_)
			urlTitleCollector_->addUrl(index_, url_, "<read body failed>");

		return false;
	}

	assert(buffer);

	recieved_content_size_ += bytes_transferred;
	buffer_.insert(buffer_.end(), buffer, buffer + bytes_transferred);
	
	assert(connection_);
	assert(htmlParser_);
	assert(recieved_content_size_ <= reply_header_.getContentLength());

	bool titleFounded = false;
	if (htmlParser_ == nullptr)
		return false;

	bool clearBuffer = true;
	std::string title;
	try
	{
		title = htmlParser_->parseTitle(buffer_.c_str(), buffer_.length());
		titleFounded = !title.empty();
	}
	catch (const HtmlParserNotFoundException&)
	{}
	catch (const HtmlParserPartialTagException&)
	{
		clearBuffer = false;
	}

	if (clearBuffer)
		buffer_.clear();

	bool continue_reading = !titleFounded && recieved_content_size_ < reply_header_.getContentLength();
	
	if (connection_ && continue_reading)
		connection_->async_read(boost::bind(&UrlTitleExtractor::on_read_body, this, 
			boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
	else if (urlTitleCollector_)
	{
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

}