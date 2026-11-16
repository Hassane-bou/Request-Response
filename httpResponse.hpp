#ifndef HTTPRESPONSE_HTTP
#define HTTPRESPONSE_HTTP

#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include "httpRequest.hpp"



struct Httpresponse
{
    std::string version;
    int statusCode;
    std::string reasonPhrase;
    std::vector<HttpHeader> headers;
    std::string body;
};





#endif