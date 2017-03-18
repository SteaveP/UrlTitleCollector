#pragma once

#include <memory>
#include <boost/shared_ptr.hpp>

namespace nc
{

class IConnection;

namespace net
{

class Url;

//////////////////////////////////////////////////////////////////////////

class AsioContext
{
public:	
	AsioContext();
	~AsioContext();

	class impl;
	impl* getImpl() const { return pimpl_.get(); }

	boost::shared_ptr<IConnection> create_connection(const Url& url);

	void run_loop();

private:
	std::unique_ptr<impl> pimpl_;
};

}
}