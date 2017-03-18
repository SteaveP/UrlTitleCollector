#include "ihttpprovider.h"

#include "httpprovider.h"

#include <boost/make_shared.hpp>

namespace nc
{

boost::shared_ptr<IHttpProvider> IHttpProvider::create()
{
	return boost::make_shared<HttpProvider>();
}

}