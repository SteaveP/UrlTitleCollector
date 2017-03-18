#pragma once

#include "iurltitlecollector.h"

#include <memory>
#include <iosfwd>

#include <boost/noncopyable.hpp>

namespace nc
{

namespace net
{
class AsioContext;
}

class UrlTitleCollector : public IUrlTitleCollector, boost::noncopyable
{
public:
	explicit UrlTitleCollector(const net::AsioContext& context, const std::string& filename);
	virtual ~UrlTitleCollector();

	virtual void addUrl(std::size_t index, const net::Url& url, const std::string& title) override;

private:
	void addUrlImpl(std::size_t index, const net::Url& url, const std::string& title);

private:
	bool something_written_;
	std::unique_ptr<std::ofstream> outStream_;

	class impl;
	std::unique_ptr<impl> pimpl_;
};

}