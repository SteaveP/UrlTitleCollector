#include "urltitleextractor.h"

#include <iostream>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

namespace nc
{

UrlTitleExtractor::UrlTitleExtractor(net::Url&& url, int index, IUrlTitleCollector* titleCollector, 
	IHttpProvider* httpProvider, IHtmlParser* htmlParser, 
	IConnectionFactory* connectionFactory, IConnection* connection)
: url_(std::move(url)), index_(index), urlTitleCollector_(titleCollector), htmlParser_(htmlParser)
, httpProvider_(httpProvider), connection_(connection), connectionFactory_(connectionFactory), recieved_body_size_(0)
{}

UrlTitleExtractor::UrlTitleExtractor(const net::Url& url, int index, IUrlTitleCollector* titleCollector, 
	IHttpProvider* httpProvider, IHtmlParser* htmlParser, 
	IConnectionFactory* connectionFactory, IConnection* connection)
: url_(url), index_(index), urlTitleCollector_(titleCollector), htmlParser_(htmlParser)
, httpProvider_(httpProvider), connection_(connection), connectionFactory_(connectionFactory), recieved_body_size_(0)
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

	// NOTE first read might not receive whole header

	assert(httpProvider_);
	assert(connection_);
	assert(buffer);

	if (httpProvider_ == nullptr)
		return false;

	// feed internal buffer
	buffer_.insert(buffer_.end(), buffer, buffer + bytes_transferred);

	// try to parse header
	size_t header_border = -1;
	reply_header_ = httpProvider_->parseHeader(buffer_.c_str(), buffer_.length(), header_border);

	if (header_border == -1)
	{
		// header received partially, read until whole header is received
		if (connection_)
			connection_->async_read(boost::bind(&UrlTitleExtractor::on_read_header, this,
				boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

		return true;
	}

	// remove header from buffer, because buffer may contain body
	buffer_.erase(begin(buffer_), begin(buffer_) + header_border);
	// now buffer contains only body (if body is received)
	recieved_body_size_ += buffer_.length();
	
	if (reply_header_.getCode() == 200)
	{
		if (connection_)
			connection_->async_read(boost::bind(&UrlTitleExtractor::on_read_body, this,
				boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
	}
	else if (reply_header_.getCode() >= 300 && reply_header_.getCode() < 400)
	{
		if (!redirect())
			return false;
	}
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
	assert(connection_);
	assert(htmlParser_);
	assert(recieved_body_size_ <= reply_header_.getContentLength());

	// feed internal buffer
	recieved_body_size_ += bytes_transferred;
	buffer_.insert(buffer_.end(), buffer, buffer + bytes_transferred);

	if (htmlParser_ == nullptr)
		return false;

	std::string title;
	bool titleFound = false;
	bool needToClearBuffer = true;

	try
	{
		title = htmlParser_->parseTitle(buffer_.c_str(), buffer_.length());
		titleFound = !title.empty();
	}
	catch (const HtmlParserNotFoundException&)
	{}
	catch (const HtmlParserPartialTagException&)
	{
		// buffer contains only part of title tag 
		// so we must feed another part of the body into internal buffer		
		needToClearBuffer = false;
	}

	if (needToClearBuffer)
		buffer_.clear();

	bool continue_reading = !titleFound && recieved_body_size_ < reply_header_.getContentLength();
	
	if (connection_ && continue_reading)
		connection_->async_read(boost::bind(&UrlTitleExtractor::on_read_body, this, 
			boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
	else if (urlTitleCollector_)
	{
		urlTitleCollector_->addUrl(index_, url_, titleFound ? title : "<not found>");
	}

	return true;
}

bool UrlTitleExtractor::on_skip_body(const char* buffer, const boost::system::error_code& error, size_t bytes_transferred)
{
	if (error)
	{
		std::cerr << "TitleExtractor::on_skip_body: " << error.message() << std::endl;

		if (urlTitleCollector_)
			urlTitleCollector_->addUrl(index_, url_, "<skip body failed>");

		return false;
	}

	// feed internal buffer
	recieved_body_size_ += bytes_transferred;

	bool continue_reading = recieved_body_size_ < reply_header_.getContentLength();

	if (continue_reading)
	{
		if (connection_)
			connection_->async_read(boost::bind(&UrlTitleExtractor::on_skip_body, this,
				boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
	}
	else
	{
		if (!build_and_send_request(reply_header_.getLocation()))
			return false;
	}

	return true;
}

bool UrlTitleExtractor::redirect()
{
	if (reply_header_.getLocation().empty())
	{
		if (urlTitleCollector_)
			urlTitleCollector_->addUrl(index_, url_, "<empty redirection url>");

		return false;
	}
	else if (url_.getUrl() == reply_header_.getLocation())
	{
		if (urlTitleCollector_)
			urlTitleCollector_->addUrl(index_, url_, "<cyclic redirection>");

		return false;
	}
	else if (reply_header_.getContentLength() == -1 || recieved_body_size_ == reply_header_.getContentLength())
	{
		if (!build_and_send_request(reply_header_.getLocation()))
			return false;
	}
	else
	{
		// skip body
		if (connection_)
			connection_->async_read(boost::bind(&UrlTitleExtractor::on_skip_body, this,
				boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
	}

	return true;
}

bool UrlTitleExtractor::build_and_send_request(const std::string& url_text)
{
	if (httpProvider_ == nullptr)
		return true;

	if (auto redirect_url = net::UrlParser().Parse(url_text))
	{
		buffer_.clear();
		recieved_body_size_ = 0;

		if (reply_header_.isConnectionClosed())
		{
			// create new connection and extractor
			if (connectionFactory_)
			{
				// NOTE new extractor have url that differs from source url
				auto new_connection = connectionFactory_->create_connection(redirect_url);
				if (!new_connection)
				{
					if (urlTitleCollector_)
						urlTitleCollector_->addUrl(index_, url_, "<redirection create connection failed>");

					return false;
				}

				auto new_exctractor = boost::make_shared<UrlTitleExtractor>(
					std::move(redirect_url), index_, urlTitleCollector_, httpProvider_,
					htmlParser_, connectionFactory_, new_connection.get()
				);
				
				new_connection->connect(
					boost::bind(&UrlTitleExtractor::on_connected, new_exctractor, boost::placeholders::_1),
					boost::make_shared<boost::any>(new_exctractor)
				);

				return true;
			}
			else if (urlTitleCollector_)
				urlTitleCollector_->addUrl(index_, url_, "<redirection connection closed>");

			return false;
		}
		else
		{
			send_request(httpProvider_->buildRequest(redirect_url));
		}

		return true;
	}
	
	return false;
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