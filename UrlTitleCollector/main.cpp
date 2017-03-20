#include <iostream>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>
#include <boost/program_options.hpp>

#include <fstream>

#include "data/urltitleextractor.h"
#include "data/urltitlecollector.h"

#include "data/iconnection.h"
#include "net/context.h"
#include "options.h"

// TODO explicit declare move ctors and operators to types with explicitly defined dtor
// TODO use https://boost-experimental.github.io/di/
// TODO use logging
// TODO remove code duplication between HttpsConnection and with HttpConnection
// TODO support input and output to cin and cout streams

void try_to_start_worker_threads(nc::net::AsioContext& context, boost::thread_group& threads, int threads_count)
{
	static bool started = false;

	if (started == false)
	{
		for (int i = 0; i < threads_count; ++i)
			threads.create_thread(boost::bind(&nc::net::AsioContext::run_loop, &context));

		started = true;
	}
}

int main(int argc, char* argv[])
{
	setlocale(0, "");

	nc::Options options;

	// parse options
	try
	{
		options.parse(argc, argv);
		options.dispatch_options();
	}
	catch(const std::exception& error)
	{
		std::cout << "Arguments error: " << error.what() << "\n\n";
		options.print_usage();

		system("pause");
		return -1;
	}
	
	// run async operations
	try
	{
		nc::net::AsioContext context;
		boost::thread_group threads;

		nc::net::UrlParser urlParser;
		nc::UrlTitleCollector urlTitleCollector(context, options["output-file"].as<std::string>());
		auto httpProvider = nc::IHttpProvider::create();
		auto htmlParser = nc::IHtmlParser::create();
		
		std::ifstream input_file(options["input-file"].as<std::string>(), std::ios_base::in);
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
						htmlParser.get(), &context, connectionPtr.get()
					);

					connectionPtr->connect(
						boost::bind(&nc::UrlTitleExtractor::on_connected, extractorPtr, boost::placeholders::_1),
						boost::make_shared<boost::any>(extractorPtr)
					);

					try_to_start_worker_threads(context, threads, options["threads-count"].as<int>());
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
		system("pause");
		return 1;
	}

	system("pause");

	return 0;
}