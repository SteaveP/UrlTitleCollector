#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

#include "../../UrlTitleCollector/net/url.h"

namespace // anonymous
{
	const char* full_valid_dataset[]{
		"http://yandex.ru/test",
		" https://yandex.ru/test ",
		"http://www.yandex.ru/path",
		" https://yandex.ru/ ",
		"http://www.yandex.ru/long/path",
		"http://www.ya ndex.ru/lon g/pa th",
		"ftp://www.yandex.ru/path",
		"http://yandex.ru/",
	};

	const char* semi_valid_dataset[]{
		"www.yandex.ru",
		"http://yandex.ru",
		" https://yandex.ru ",
		"www.yandex.ru/test",
		"ftp://www.yandex.ru ",
		"somethingwrong",
	};

	const char* invalid_dataset[]{
		"",
		"://",
		"/"
	};
}

BOOST_AUTO_TEST_SUITE(UrlTest);

BOOST_DATA_TEST_CASE(testFullValid, full_valid_dataset, array_element)
{
	const auto& url = nc::net::UrlParser().Parse(array_element);
	BOOST_TEST(url.isValid(), array_element);
	BOOST_TEST(url.getProtocol().empty() == false, array_element);
	BOOST_TEST(url.getPath().empty() == false, array_element);
}

BOOST_DATA_TEST_CASE(testSemiValid, semi_valid_dataset, array_element)
{
	const auto& url = nc::net::UrlParser().Parse(array_element);
	bool somethingEmpty = url.getProtocol().empty() || url.getPath().empty();
	BOOST_TEST(url.isValid(), array_element);
	BOOST_TEST(somethingEmpty, array_element);
}

BOOST_DATA_TEST_CASE(testInvalid, invalid_dataset, array_element)
{
	const auto& url = nc::net::UrlParser().Parse(array_element);
	BOOST_TEST(url.isValid() == false, array_element);
}

BOOST_AUTO_TEST_CASE(testValidOnlyProtocol)
{
	const auto& url = nc::net::UrlParser().Parse("http://yandex.ru");
	BOOST_TEST(url.getProtocol() == "http");
	BOOST_TEST(url.getHost() == "yandex.ru");
	BOOST_TEST(url.getPath().empty());
}

BOOST_AUTO_TEST_CASE(testValidOnlyPath)
{
	const auto& url = nc::net::UrlParser().Parse("yandex.ru/test");
	BOOST_TEST(url.getProtocol().empty());
	BOOST_TEST(url.getHost() == "yandex.ru");
	BOOST_TEST(url.getPath() == "/test");
}

BOOST_AUTO_TEST_CASE(testValidPathAndProtocol)
{
	const auto& url = nc::net::UrlParser().Parse("http://yandex.ru/test");
	BOOST_TEST(url.getProtocol() == "http");
	BOOST_TEST(url.getHost() == "yandex.ru");
	BOOST_TEST(url.getPath() == "/test");
}

BOOST_AUTO_TEST_SUITE_END();