#pragma once

#include <memory>
#include <boost/shared_ptr.hpp>

#include "../data/iconnection.h"

namespace nc
{

class IConnection;

namespace net
{

class Url;

class AsioContext : public IConnectionFactory
{
public:	
	AsioContext();
	~AsioContext();
	
	void run_loop();
	virtual boost::shared_ptr<IConnection> create_connection(const Url& url) override;

	class impl;
	impl* getImpl() const { return pimpl_.get(); }

private:
	std::unique_ptr<impl> pimpl_;
};

}
}