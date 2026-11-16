#include "httpResponse.hpp"


std::string reasonPhrase(int statusCode)
{
    switch(statusCode)
    {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 204:
            return "No Content";
        case 301:
            return "Moved Permanently";
        case 400:
            return "Bad Request";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 413:
            return "Content Too Large";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        default:
            return "Unknown Status";
    }
}

void AddHeader(Httpresponse &response, const std::string &name, const std::string &value)
{
    HttpHeader head;
    
    head.name = name;
    head.value = value;

    response.headers.push_back(head);
}

void SetBody(Httpresponse &response, const std::string &body)
{
    response.body = body;
}


void SetContentLength(Httpresponse &response)
{
    size_t size = response.body.size();
    std::ostringstream oss;

    oss << size;

    std::string sizeConvert = oss.str();
    AddHeader(response,"Content-Length",sizeConvert);

}


std::string BuildResponse(Httpresponse &response)
{
    std::string result = "";
    std::ostringstream oss;

    result.append(response.version);
    result.append(" ");
    oss << response.statusCode;
    result.append(oss.str());
    result.append(" ");
    result.append(response.reasonPhrase);
    result.append("\r\n");


    for(size_t i = 0; i < response.headers.size();i++)
    {
        result.append(response.headers[i].name);
        result.append(": ");
        result.append(response.headers[i].value);
        result.append("\r\n");
    }


    result.append("\r\n");

    result.append(response.body);

    return result;
}

std::string GetErrorResponse(int Status)
{
    switch(Status)
    {
        case 400:
            return "Bad Request";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 413:
            return "Content Too Large";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        default:
            return "Error";

    }
}
Httpresponse BuildErrorResponse(int statusCode)
{
    Httpresponse response;
    std::ostringstream oss;
    response.version = "HTTP/1.1";
    response.statusCode = statusCode;
    oss << response.statusCode;
    response.reasonPhrase =reasonPhrase(statusCode);
    

    response.body ="<html>\n"
                    "<body>\n"
                    "<h1> "+ oss.str() + " " + GetErrorResponse(statusCode) +" </h1>\n"
                    "<p> Error.</p>\n"
                    "</body>\n"
                    "</html>\n";

    AddHeader(response,"Content-Type", "text/html");
    SetContentLength(response);
    

    return response;
}

std::string GetFilePath(std::string &root,HttpRequestLine &requestpath)
{
    std::string  physicalPath  = root + requestpath.path;

    return physicalPath;
}

bool CheckFile(std::string &physicalPath)
{
    struct  stat fileinfo;

    if(stat(physicalPath.c_str(),&fileinfo) != 0)
        return false;
    if(!S_ISREG(fileinfo.st_mode))
        return false;
    if(access(physicalPath.c_str(), R_OK) != 0)
        return false;

    return true;
}


int main()
{
    // Httpresponse response;
    // response.version = "HTTP/1.1";
    // response.statusCode = 200;
    // response.reasonPhrase = reasonPhrase(response.statusCode);

    // AddHeader(response, "Content-Type", "text/plain");
    // response.body = "hello world\n";


    // SetContentLength(response);

    // std::string result = BuildResponse(response);

    // Httpresponse Error = BuildErrorResponse(500);

    // std::cout << result;

    // std::cout << BuildResponse(Error);


    std::string root = "./www";
    std::string path = "/index.html";

    std::string fullPath = root + path;

    std::cout << fullPath << std::endl;

    return 0;
}