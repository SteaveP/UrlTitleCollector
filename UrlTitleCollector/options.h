#include <iostream>
#include <boost/program_options.hpp>

namespace nc
{

namespace po = boost::program_options;

class Options
{
	friend std::ostream& operator << (std::ostream& os, const Options& options);

public:
	Options();
		
	void parse(int argc, char* argv[]);
	void dispatch_options();
	void print_usage();

	size_t count(const std::string& name) const { return variables.count(name); }
	auto operator [] (const std::string& name) const { return variables[name]; }

private:
	po::options_description description;
	po::variables_map variables;
};

// TODO support positional arguments
Options::Options()
	: description("Allowed options")
{
}

void Options::parse(int argc, char* argv[])
{
	description.add_options()
		("help,h", "produce help message")
		("input-file,i", po::value<std::string>()->required(), "input file")
		("output-file,o", po::value<std::string>()->required(), "output file")
		("threads-count,j", po::value<int>()->default_value(10), "worker threads count")
	;

	po::positional_options_description p;
	p.add("input-file", 1);
	p.add("output-file", 1);

	po::store(po::command_line_parser(argc, argv).options(description).positional(p).run(), variables);
	po::notify(variables);
}

void Options::dispatch_options()
{
	if (count("help"))
		throw std::exception("help:", 1);

	if (variables["threads-count"].as<int>() <= 0)
		throw std::exception("worker threads count must be greater than 0\n", 1);
}

void Options::print_usage()
{
	std::cout << "Usage: <executable> [options] [input-file] [output-file]\n" << *this << '\n';
}

std::ostream& operator << (std::ostream& os, const Options& options)
{
	return os << options.description;
}

}