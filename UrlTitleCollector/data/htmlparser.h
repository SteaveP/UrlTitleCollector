#pragma once

#include "ihtmlparser.h"
#include <memory>

namespace nc
{

class HtmlParser : public IHtmlParser
{
public:
	HtmlParser();
	~HtmlParser();
	
	virtual std::string parseTitle(const char* buffer, size_t length) override;

private:
	class impl;
	std::unique_ptr<impl> pimpl_;
};

}
