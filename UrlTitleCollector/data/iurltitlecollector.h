#pragma once

#include <string>

namespace nc
{

namespace net
{
class Url;
}

class IUrlTitleCollector
{
public:
	virtual ~IUrlTitleCollector() = 0 {};

	virtual void addUrl(std::size_t index, const net::Url& url, const std::string& title) = 0;
};

}