#include <iostream>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>

#include <fstream>

#include "data/urltitleextractor.h"
#include "data/urltitlecollector.h"

#include "data/iconnection.h"
#include "net/context.h"

// TODO explicit declare move ctors and operators to types with explicitly defined dtor
// TODO use https://boost-experimental.github.io/di/

void try_to_start_worker_threads(nc::net::AsioContext& context, boost::thread_group& threads)
{
	static bool started = false;

	if (started == false)
	{
		for (int i = 0; i < 10; ++i)
			threads.create_thread(boost::bind(&nc::net::AsioContext::run_loop, &context));

		started = true;
	}
}

int main(int argc, char* argv[])
{
	setlocale(0, "");
	
	if (argc != 3)
	{
		std::cerr << "Usage: client <in_file> <out_file>\n";
		return 1;
	}

	try
	{
		nc::net::AsioContext context;
		boost::thread_group threads;

		nc::net::UrlParser urlParser;
		nc::UrlTitleCollector urlTitleCollector(context, argv[2]);
		auto httpProvider = nc::IHttpProvider::create();
		auto htmlParser = nc::IHtmlParser::create();
		
		std::ifstream input_file(argv[1], std::ios_base::in);
		std::string line; std::size_t index = 0;
		while (std::getline(input_file, line))
		{
			if (!line.empty() && *line.crbegin() != '/')
				line.push_back('/');

			// TODO line may have "number: url" format

			if (auto url = urlParser.Parse(line))
			{
				++index;
				
				if (auto connectionPtr = context.create_connection(url))
				{
					auto extractorPtr = boost::make_shared<nc::UrlTitleExtractor>(
						std::move(url), index, &urlTitleCollector, httpProvider.get(), 
						htmlParser.get(), connectionPtr.get()
					);

					connectionPtr->connect(
						boost::bind(&nc::UrlTitleExtractor::on_connected, extractorPtr, boost::placeholders::_1),
						boost::make_shared<boost::any>(extractorPtr)
					);

					try_to_start_worker_threads(context, threads);
				}
				else
				{
					std::cerr << "Connection creation for \"" << url.getUrl() << "\" failed" << std::endl;
					urlTitleCollector.addUrl(index, nc::net::Url::create(line), "<connection creation failed>");
				}
			}
			else
			{
				std::cerr << "Url parse for \"" << line << "\" failed" << std::endl;
				urlTitleCollector.addUrl(index, nc::net::Url::create(line), "<url parse failed>");
			}
		}

		threads.join_all();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	system("pause");

	return 0;
}