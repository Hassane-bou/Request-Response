#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <cstdlib>
#include <cerrno>
#include <sstream>


#define MAX_BODY 999999999UL

struct HttpRequestLine
{
    std::string Method;
    std::string Target;
    std::string path;
    std::string Query;
    std::string Version;
};

struct HttpHeader
{
    std::string name;
    std::string value;
};

struct HttpRequest
{
    HttpRequestLine requestLine;
    std::vector<HttpHeader> headers;
    bool chunked;
    std::string body;
};

#endif