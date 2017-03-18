#pragma once

#include <boost/shared_ptr.hpp>
#include <exception>

namespace nc
{

class HtmlParserException : public std::exception
{
public:
	HtmlParserException() {}
	HtmlParserException(const char* message) : std::exception(message) {}
};

class HtmlParserNotFoundException : public HtmlParserException
{
public:
	HtmlParserNotFoundException() {}
	HtmlParserNotFoundException(const char* message) : HtmlParserException(message) {}
};

class HtmlParserPartialTagException : public HtmlParserException
{
public:
	HtmlParserPartialTagException() {}
	HtmlParserPartialTagException(const char* message) : HtmlParserException(message) {}
};

class IHtmlParser
{
public:
	static boost::shared_ptr<IHtmlParser> create();

	virtual ~IHtmlParser() = 0 {};
	virtual std::string parseTitle(const char* buffer, size_t length) = 0;
};

}
