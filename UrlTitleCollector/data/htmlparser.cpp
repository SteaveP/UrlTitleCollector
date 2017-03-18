#include "htmlparser.h"

#include <boost/make_shared.hpp>
#include <regex>

namespace nc
{

boost::shared_ptr<IHtmlParser> IHtmlParser::create()
{
	return boost::make_shared<HtmlParser>();
}

class HtmlParser::impl
{
	friend class HtmlParser;

public:
	impl();

private:
	std::string buffer_;
	std::regex regex_;
};

// TODO ignore space between tags?
HtmlParser::impl::impl()
	: regex_("<title>(((?!</title>).)*)(</title>)?", std::regex_constants::icase)
{

}

HtmlParser::HtmlParser()
	: pimpl_(std::make_unique<impl>())
{}

HtmlParser::~HtmlParser()
{}

std::string HtmlParser::parseTitle(const char* buffer, size_t length)
{
	std::cmatch results;

	if (std::regex_search(buffer, buffer + length, results, pimpl_->regex_))
	{
		std::string closingTag = results.str(3);

		if (closingTag.empty())
			throw HtmlParserPartialTagException();

		return results.str(1);
	}

	throw HtmlParserNotFoundException();
}


}