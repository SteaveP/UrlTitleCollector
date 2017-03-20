#include "urltitlecollector.h"

#include <boost/bind.hpp>
#include <boost/asio/strand.hpp>

#include <fstream>

#include "../net/url.h"
#include "../net/contextimpl.h"

namespace nc
{

namespace asio = boost::asio;

class UrlTitleCollector::impl
{
	friend class UrlTitleCollector;

public:
	impl(const net::AsioContext& context);

private:
	asio::strand strand_;
	const net::AsioContext& context_;
};

UrlTitleCollector::impl::impl(const net::AsioContext& context)
	: strand_(context.getImpl()->get_io_service()), context_(context)
{}

UrlTitleCollector::UrlTitleCollector(const net::AsioContext& context, const std::string& filename)
	: outStream_(std::make_unique<std::ofstream>(filename, 
		std::ios_base::out | std::ios_base::ate | std::ios_base::app))

	, something_written_(false)
	, pimpl_(std::make_unique<impl>(context))
{}

UrlTitleCollector::~UrlTitleCollector()
{}

void UrlTitleCollector::addUrl(std::size_t index, const net::Url& url, const std::string& title)
{
	asio::io_service& io_service = pimpl_->context_.getImpl()->get_io_service();

	// use asio::strand to avoid concurent calls
	io_service.post(pimpl_->strand_.wrap(
		boost::bind(&UrlTitleCollector::addUrlImpl, this, index, url, title)
	));
}

void UrlTitleCollector::addUrlImpl(std::size_t index, const net::Url& url, const std::string& title)
{
	if (something_written_ == false)
	{
		(*outStream_) << "\n=========================\n\n";
		something_written_ = true;
	}

	(*outStream_) << index << " : " << url << " : " << title << '\n';
}

}