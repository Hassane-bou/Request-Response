#ifndef HTTPRESPONSE_HTTP
#define HTTPRESPONSE_HTTP

#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <errno.h>
#include "httpRequest.hpp"


struct Httpresponse
{
    std::string version;
    int statusCode;
    std::string reasonPhrase;
    std::vector<HttpHeader> headers;
    std::string body;
};


Httpresponse BuildGetResponse(std::string &root, std::string &location,HttpRequestLine & request);
std::string BuildResponse(Httpresponse &response);


#endif