#pragma once

#include <boost/system/error_code.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>

namespace nc
{

namespace net
{
class Url;
}

class IConnection
{
public:
	typedef boost::function< bool(
		const boost::system::error_code& error)
	> TConnectionCallback;

	typedef boost::function< bool(const char* buffer,
		const boost::system::error_code& error,
		size_t bytes_transferred)
	> TReadCallback;

	typedef boost::function< bool(
		const boost::system::error_code& error,
		size_t bytes_transferred)
	> TWriteCallback;

	typedef boost::shared_ptr<boost::any> TAnyPtr;

public:
	virtual ~IConnection() = 0 {};

	virtual void connect(TConnectionCallback connectionCallback, TAnyPtr data = TAnyPtr()) = 0;
	virtual void close() = 0;

	virtual void setData(TAnyPtr data) = 0;
	virtual TAnyPtr getData() const = 0;

	virtual void async_read(TReadCallback callback) = 0;
	virtual void async_write(const char* message, size_t message_size, TWriteCallback callback) = 0;
};

class IConnectionFactory
{
public:
	virtual boost::shared_ptr<IConnection> create_connection(const net::Url& url) = 0;
};

}