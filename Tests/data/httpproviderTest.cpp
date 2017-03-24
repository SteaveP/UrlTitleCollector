#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "../../UrlTitleCollector/data/httpprovider.h"
#include "../../UrlTitleCollector/net/url.h"

namespace // anonymous
{
	// buildRequest method test data
	struct test_input_for_build_request
	{
		std::string input;

		std::vector<std::string> expected;
		bool close_connection;
	};

	std::ostream& operator << (std::ostream& os, const test_input_for_build_request& data)
	{
		return os << data.input << " " << data.close_connection;
	}

	test_input_for_build_request valid_dataset_for_build_request[] {
		{ "boost.org", {"Host: boost.org", "GET / HTTP/", "Connection: close", "\r\n\r\n" }, true },
		{ "https://boost.org/",{ "Host: boost.org", "GET / HTTP/", "Connection: keep-alive", "\r\n\r\n" }, false },
		{ "boost.org/path/to/content", { "Host: boost.org", "GET /path/to/content HTTP/", "\r\n\r\n" }, false },
	};

	// parseHeader method test data
	struct test_input_for_parse_header
	{
		std::string input;
		nc::HttpReplyHeader expected;

		size_t delimiterPos;
	};

	std::ostream& operator << (std::ostream& os, const test_input_for_parse_header& data)
	{
		return os << data.input << " " << data.delimiterPos;
	}

	bool compare(const nc::HttpReplyHeader& lhs, const nc::HttpReplyHeader& rhs)
	{
		// NOTE without getText()
		return lhs.getCode() == rhs.getCode()
			&& lhs.getContentLength() == rhs.getContentLength()
			&& lhs.getLocation() == rhs.getLocation()
			&& lhs.isConnectionClosed() == rhs.isConnectionClosed();
	}

	test_input_for_parse_header valid_dataset_for_parse_header[] {
		{	"HTTP/1.0 200 OK\r\n"
				"Date: Fri, 24 Mar 2017 16:45:36 GMT\r\n"
				"Server: Apache/2.2.15 (CentOS)\r\n"
				"Accept-Ranges: bytes\r\n"
				"Content-length: 1234\r\n"
				"Connection: keep-alive\r\n"
				"Transfer-Encoding: chunked\r\n"
				"Content-Type: text/html\r\n\r\n",
			nc::HttpReplyHeader(200, 1234, false, "", ""), 209 
		},
		{	"HTTP/1.1 200 OK\r\n"
				"Date: Fri, 24 Mar 2017 16:45:36 GMT\r\n"
				"Server: Apache/2.2.15 (CentOS)\r\n"
				"Accept-Ranges: bytes\r\n"
				"Content-length: 1234\r\n"
				"Connection: close\r\n"
				"Transfer-Encoding: chunked\r\n"
				"Location: new location\r\n"
				"Content-Type: text/html\r\n\r\n",
			nc::HttpReplyHeader(200, 1234, true, "new location", ""), 228 
		},
		{	"HTTP/1.1 200\r\n\r\n",
			nc::HttpReplyHeader(200, -1, false, "", ""), 16
		},
	};

	const char* invalid_dataset_for_parse_header[]{
		"",
		"HTP/1.1 200 OK",
		"HTTP/1.1\r\n",
		"HTTP/1.1 200 OK\r\n",

		"HTTP/1.1 200 OK\r\n"
		"Date: Fri, 24 Mar 2017 16:45:36 GMT\r\n"
		"Server: Apache/2.2.15 (CentOS)\r\n"
		"Accept-Ranges: bytes\r\n"
		"Content-length: 1234\r\n"
		"Connection: keep-alive\r\n"
		"Transfer-Encoding: chunked\r\n"
		"Content-Type: text/html\r\n", // without \r\n\r\n
	};

	// fixture
	struct HttpProviderFixture
	{
		HttpProviderFixture() {}
		~HttpProviderFixture() {}

		nc::HttpProvider httpProvider;
		nc::net::UrlParser urlParser;
	};
}

BOOST_FIXTURE_TEST_SUITE(HttpProviderTest, HttpProviderFixture);

BOOST_DATA_TEST_CASE(testBuildRequest, valid_dataset_for_build_request, array_element)
{
	auto url = urlParser.Parse(array_element.input);
	BOOST_TEST_REQUIRE(url.isValid());

	auto text = httpProvider.buildRequest(url, array_element.close_connection);

	for (const auto& s : array_element.expected)
	{
		BOOST_TEST(text.find(s) != std::string::npos, array_element);
	}
}

BOOST_DATA_TEST_CASE(testValidParseHeader, valid_dataset_for_parse_header, array_element)
{
	size_t headerBorder;	
	const auto& header = httpProvider.parseHeader(array_element.input.c_str(), array_element.input.length(), headerBorder);

	BOOST_TEST_REQUIRE(array_element.delimiterPos == headerBorder, array_element);
	BOOST_TEST(compare(array_element.expected, header), array_element);
}

BOOST_DATA_TEST_CASE(testInvalidParseHeader, invalid_dataset_for_parse_header, array_element)
{
	size_t headerBorder;
	const auto& header = httpProvider.parseHeader(array_element, std::strlen(array_element), headerBorder);
	
	BOOST_TEST(headerBorder == -1, array_element);
}

BOOST_AUTO_TEST_SUITE_END();
