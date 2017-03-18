#include "context.h"
#include "contextimpl.h"

namespace nc
{
namespace net
{
	
AsioContext::AsioContext()
try
	: pimpl_(std::make_unique<impl>())
{
	// empty
}
catch (...)
{
	// TODO
	throw;
}

AsioContext::~AsioContext()
{}

boost::shared_ptr<nc::IConnection> AsioContext::create_connection(const Url& url)
{
	return pimpl_->create_connection(url);
}

void AsioContext::run_loop()
{
	return pimpl_->run_loop();
}

}
}