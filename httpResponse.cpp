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

std::string GetFilePath(std::string &root,std::string &location,HttpRequestLine &requestpath)
{
    std::string physicalPath = "";

    if(requestpath.path.compare(0,location.size(), location) != 0)
        return "";

    if(location != "/" && requestpath.path.size() > location.size())
    {
        if(requestpath.path[location.size()] != '/')
            return "";
    }

    std::string partExtra = requestpath.path.substr(location.size());
    if(location == "/")
        physicalPath = root + "/" + partExtra;
    else
        physicalPath = root + partExtra;

    return physicalPath;
}

bool NormalizePath(const std::string &path, std::string &normalized)
{
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;
    bool isAbsolute = (!path.empty() && path[0] == '/');


    while(std::getline(ss, part, '/'))
    {
        if(part.empty() || part == ".")
            continue;

        if(part == "..")
        {
            if(parts.empty())
                return false;
            parts.pop_back();
        }
        else{
            parts.push_back(part);
        }
    }

    normalized = "";

    if(isAbsolute)
        normalized ="/";

    for(size_t i = 0; i < parts.size();i++)
    {
        if(i != 0)
            normalized += "/";

        normalized +=parts[i];
    }
    return true;
}


bool SecureFilePath(const std::string &root,const std::string &physicalPath)
{
    std::string normalizedRoot;
    std::string normalizedPhysical;

    if(!NormalizePath(root,normalizedRoot))
        return false;
    
    if(!NormalizePath(physicalPath, normalizedPhysical))
        return false;

    if(normalizedPhysical.compare(0,normalizedRoot.size(),normalizedRoot)!=0)
        return false;

    if(normalizedPhysical.size() > normalizedRoot.size())
    {
        if(normalizedPhysical[normalizedRoot.size()] != '/')
            return false;
    }
    return true;
}

std::string GetContentType(const std::string &physicalPath)
{
    size_t pos = physicalPath.find_last_of('.');
    if(pos == std::string::npos)
        return "application/octet-stream";
    std::string extension = physicalPath.substr(pos);

    if(extension == ".html")
        return "text/html";
    else if(extension == ".css")
        return "text/css";
    else if(extension == ".js")
        return "application/javascript";
    else if(extension == ".jpg")
        return "image/jpeg";
    else if(extension == ".png")
        return "image/png";
    else
        return "application/octet-stream";
}

int CheckFile(std::string &physicalPath)
{
    struct  stat fileinfo;

    if(stat(physicalPath.c_str(),&fileinfo) != 0)
    {
        if(errno == ENOENT)
            return 404;
        if(errno == EACCES)
            return 403;
        if(errno == ENOTDIR)
            return 404;
        return 500;
    }
    if(!S_ISREG(fileinfo.st_mode))
        return 403;
    if(access(physicalPath.c_str(), R_OK) != 0)
        return 403;

    return 200;
}

bool ReadFile(const std::string &physicalPath ,std ::string &body)
{
    
    std::ifstream file(physicalPath.c_str(), std::ios::binary);
        
    if(!file.is_open())
        return false;

    std::ostringstream buffer;
    buffer << file.rdbuf();
    if(file.bad())
    {
        file.close();
        return false;
    }
    body = buffer.str();

    file.close();

    return true;

}

Httpresponse BuildGetResponse(std::string &root, std::string &location,HttpRequestLine & request)
{
    Httpresponse GetResponse;
    std::string body;

    std::string physicalPath = GetFilePath(root,location,request);

    std::cout << "Physical path: [" <<  physicalPath << "]" << std::endl;

    if(physicalPath.empty())
        return BuildErrorResponse(403);
    bool secure = SecureFilePath(root, physicalPath);
    std::cout << "SecureFilePath: " << secure << std::endl;
    if(!secure)
        return BuildErrorResponse(403);
    int status = CheckFile(physicalPath);
    if( status != 200)
        return BuildErrorResponse(status);

    if(!ReadFile(physicalPath,body))
        return BuildErrorResponse(500);

    std::string ContentType = GetContentType(physicalPath);

    GetResponse.version = "HTTP/1.1";
    GetResponse.statusCode = 200;
    GetResponse.reasonPhrase = reasonPhrase(200);

    SetBody(GetResponse, body);
    AddHeader(GetResponse,"Content-Type", ContentType);
    SetContentLength(GetResponse);
    
    

    return GetResponse;

}

int main()
{
    std::string root = "./www";
    std::string location = "/";

    HttpRequestLine request;
    request.path = "/docs/../index.html";

    Httpresponse response = BuildGetResponse(root, location, request);

    std::cout << BuildResponse(response);
    return 0;
}

