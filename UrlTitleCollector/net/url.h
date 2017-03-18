#pragma once

#include <string>

namespace nc
{
namespace net
{

class Url
{
	friend class UrlParser;

	Url(const std::string& url);
	Url(std::string url, std::string protocol, std::string host, std::string path);

public:
	static Url create(const std::string& url) { return Url(url); }

	const std::string& getProtocol() const { return protocol_; }
	const std::string& getHost() const { return host_; }
	const std::string& getPath() const { return path_; }

	const std::string& getUrl() const { return url_; }

	bool isValid() const { return !getUrl().empty() && !host_.empty(); }
	explicit operator bool() const { return isValid(); }

private:
	std::string protocol_;
	std::string host_;
	std::string path_;

	std::string url_;
};

std::ostream& operator << (std::ostream & os, const Url& url);

//////////////////////////////////////////////////////////////////////////

class UrlParser
{
public:

	Url Parse(const std::string& url);
};

}
}