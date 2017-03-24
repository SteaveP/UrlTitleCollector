#pragma once

#include "../UrlTitleCollector/data/urltitleextractor.h"
#include "../UrlTitleCollector/data/iconnection.h"
#include "../UrlTitleCollector/net/context.h"

// Mock for UrlTitleCollector
class UrlTitleCollectorMock : public nc::IUrlTitleCollector, boost::noncopyable
{
public:
	explicit UrlTitleCollectorMock(const nc::net::AsioContext& context) {}
	virtual ~UrlTitleCollectorMock() {}

	virtual void addUrl(std::size_t index, const nc::net::Url& url, const std::string& title) override 
	{ title_ = title; }

	const std::string& getTitle() const { return title_; }

private:
	std::string title_;
};

// boost.test fixture
struct CollectorFixture
{
	CollectorFixture() 
		: urlTitleCollectorMock(context)
		, httpProvider(nc::IHttpProvider::create())
		, htmlParser(nc::IHtmlParser::create())
	{};
	~CollectorFixture() {};

	nc::net::AsioContext context;

	nc::net::UrlParser urlParser;
	UrlTitleCollectorMock urlTitleCollectorMock;
	boost::shared_ptr<nc::IHttpProvider> httpProvider;
	boost::shared_ptr<nc::IHtmlParser> htmlParser;
};

struct test_input {
	std::string url;
	std::string titlePart;
};

std::ostream& operator << (std::ostream& os, const test_input& url)
{
	return os << url.url << " " << url.titlePart;
}
