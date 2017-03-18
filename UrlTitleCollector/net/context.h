#pragma once

#include <memory>
#include <boost/shared_ptr.hpp>

namespace nc
{

class IConnection;

namespace net
{

class Url;

class AsioContext
{
public:	
	AsioContext();
	~AsioContext();

	void run_loop();
	boost::shared_ptr<IConnection> create_connection(const Url& url);

	class impl;
	impl* getImpl() const { return pimpl_.get(); }

private:
	std::unique_ptr<impl> pimpl_;
};

}
}