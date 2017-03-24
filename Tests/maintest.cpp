#define BOOST_TEST_MODULE UrlTitleCollector
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "maintest.h"

// TODO tests with multiple worker threads

test_input urls[] = {
	{ "http://www.boost.org",	"boost" },
	{ "https://www.boost.org",	"boost" },
};

BOOST_FIXTURE_TEST_SUITE(FunctionalTest, CollectorFixture);

BOOST_DATA_TEST_CASE(SingleTaskTest, urls, array_element)
{
	static const int index = 1;

	auto url = urlParser.Parse(array_element.url);
	BOOST_TEST_REQUIRE(url);
	
	auto connectionPtr = context.create_connection(url);
	BOOST_TEST_REQUIRE(connectionPtr);

	auto extractorPtr = boost::make_shared<nc::UrlTitleExtractor>(
		std::move(url), index, &urlTitleCollectorMock, httpProvider.get(),
		htmlParser.get(), &context, connectionPtr.get()
	);

	connectionPtr->connect(
		boost::bind(&nc::UrlTitleExtractor::on_connected, extractorPtr, boost::placeholders::_1),
		boost::make_shared<boost::any>(extractorPtr)
	);

	context.run_loop();

	auto title = boost::to_lower_copy(urlTitleCollectorMock.getTitle());
	BOOST_TEST_REQUIRE(title.find(array_element.titlePart) != std::string::npos,
		"header \"" << urlTitleCollectorMock.getTitle() << "\" does not contains \"" << array_element.titlePart << "\"");
}

BOOST_AUTO_TEST_SUITE_END();
