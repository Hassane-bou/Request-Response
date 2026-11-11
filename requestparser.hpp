#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include "httpRequest.hpp"

class RequestParser
{
    private:
        std::string TrimOWS( const std::string &value);
        int checkHeaderNameEquals( std::string &name1, const std::string &name2);
        HttpHeader  *FindHeader(HttpRequest &request, const std::string &searchName);
        int countHeader( HttpRequest &request,  const std::string &name);

    public:
        int ParseRequest(std::string &req, HttpRequest &request,size_t &pos);
        int  ParseRequestline( std::string &req, HttpRequestLine &line ,size_t &index);
        int ParseRequestHeader( std::string &req,HttpHeader &header,size_t &pos);
        int IsValidHeaderName(std::string name);
        int ParseHeaders(std::string &req, HttpRequest &request, size_t &pos);
        int ValidateHeaders(HttpRequest &request,unsigned long &Length);
        int ParseBody(std::string &req,HttpRequest &request,size_t &pos, unsigned long &Length);
        int ParseBodyWithChunked(std::string& req,HttpRequest &request, size_t &pos);
};

#endif