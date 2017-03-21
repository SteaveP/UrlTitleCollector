# UrlTitleCollector
Console utility for fetching url titles.
It maps url list to list of pairs \<url : title\> and save result into output file.

## Description
The project that at the same time solves practical and research goals.
The practical goal was creation of handy utility for fetching page titles for some urls set (large set).

Why C++? After all, there are more handy languages like Python will be good choise for such task. In that case development wi be easer and fatser.
Answer is that there was research goal - try the asynchronous approach to development using Boost.Asio in C++.

## Features
* Asynchronous fetches urls titles
* Support HTTP and HTTPS protocols (througth SSL)
* Support HTTP redirections (3XX response codes)

## Dependencies
* C++14
* Boost.Program_options
* Boost.Asio
* OpenSSL
* MSVC++

## License
MIT

## Usage
```
$<executable-name> <input-file-name> <output-file-name> [threads-count=<count>]
```

## TODO
* Logging support
* Boost.DI (Dependency Injection) for objects creation
