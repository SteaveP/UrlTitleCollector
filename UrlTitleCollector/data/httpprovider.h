#pragma once

#include "ihttpprovider.h"

namespace nc
{

class HttpProvider : public IHttpProvider
{
public:
	HttpProvider();

	virtual HttpReplyHeader parseHeader(const char* buffer, size_t length, size_t& headerBorder) override;
	virtual std::string buildRequest(const net::Url& url, bool close_connection = false) override;
};

}