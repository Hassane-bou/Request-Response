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
#define MAX_REQUEST_LINE 8192UL
#define MAX_HEADERS 16384UL

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

    HttpRequest() : chunked(false) {}
};

// // 0 = incomplet, 1 = OK, >= 400 = code d'erreur HTTP
// int ParseRequest(std::string &req, HttpRequest &request, size_t &pos);

#endif