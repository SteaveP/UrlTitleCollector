#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "../../UrlTitleCollector/data/htmlparser.h"

namespace // anonymous
{
	struct test_input
	{
		std::string data;
		std::string title;
	};

	std::ostream& operator << (std::ostream& os, const test_input& data)
	{
		return os << data.data << " " << data.title;
	}

	test_input full_valid_dataset[]{
		{ "<title>text</title>", "text" },
		{ " <title>text</title> ", "text" },
		{ " <title>first</title> <title>second</title> ", "first" },
		{ "<title> text </title>", " text " },
		{ "<title></title>", "" },
	};

	const char* partial_valid_dataset[]{
		"<title>partial text",
		"<title>",
	};

	const char* invalid_dataset[]{
		"",
		"no title",
		"< title > wrong spaces </ title >",
	};

	struct HtmlParserFixture
	{
		HtmlParserFixture() {}
		~HtmlParserFixture() {}

		nc::HtmlParser htmlParser;
	};
}

BOOST_FIXTURE_TEST_SUITE(HtmlParserTest, HtmlParserFixture);

BOOST_DATA_TEST_CASE(testFullValid, full_valid_dataset, array_element)
{
	std::string resulTitle;
	auto&& data = array_element.data;

	resulTitle = htmlParser.parseTitle(data.c_str(), data.length());
	BOOST_TEST(resulTitle == array_element.title, array_element);
}

BOOST_DATA_TEST_CASE(testPartialValid, partial_valid_dataset, text)
{
	BOOST_CHECK_THROW(htmlParser.parseTitle(text, std::strlen(text)),
		nc::HtmlParserPartialTagException);
}

BOOST_DATA_TEST_CASE(testInvalid, invalid_dataset, text)
{
	BOOST_CHECK_THROW(htmlParser.parseTitle(text, std::strlen(text)),
		nc::HtmlParserNotFoundException);
}

BOOST_AUTO_TEST_SUITE_END();
