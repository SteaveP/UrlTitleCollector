#include "url.h"

#include <regex>

namespace nc
{
namespace net
{

Url UrlParser::Parse(const std::string& url)
{
	std::string pattern("^(?:(http[s]?)://)?([^/]*)(/.*)?$");

	std::regex r(pattern);
	std::smatch results;

	bool result = std::regex_search(url, results, r);

	if (result)
	{
		std::string protocol = results.str(1);
		std::string host = results.str(2);
		std::string path = results.str(3);

		return std::move(Url(std::move(url), std::move(protocol), std::move(host), std::move(path)));
	}
	else
		return std::move(Url());
}

Url::Url()
{ // empty 
}

Url::Url(std::string url, std::string protocol, std::string host, std::string path)
	: url_(url), protocol_(protocol), host_(host), path_(path)
{}

std::ostream& operator<<(std::ostream & os, const Url& url)
{
	return os << url.getUrl();
}

}
}