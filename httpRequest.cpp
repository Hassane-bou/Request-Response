#include "requestparser.hpp"
#include "httpRequest.hpp"


int RequestParser::ParseRequestline( std::string &req, HttpRequestLine &line ,size_t &index)
{

    size_t pos = req.find("\r\n");
    if(pos == std::string::npos)
        return 0;
    std::string Requestline = req.substr(0,pos);

    
    size_t first_space = Requestline.find(' ');
    if (first_space == std::string::npos)
        return -1;
    size_t second_space = Requestline.find(' ',first_space + 1);
    if (second_space == std::string::npos)
        return -1;
    size_t thrid_space = Requestline.find(' ',second_space + 1);
    if(thrid_space != std::string::npos)
        return -1;

    line.Method = Requestline.substr(0,first_space);
    line.Target = Requestline.substr(first_space + 1, second_space - first_space - 1);
    line.Version = Requestline.substr(second_space + 1);
    
    if(line.Method.empty() || line.Target.empty() || line.Version.empty())
        return -1;
    if(line.Method != "GET" && line.Method != "POST" && line.Method != "DELETE")
        return -1;
    if(line.Version != "HTTP/1.1")
        return -1;
    if(line.Target[0] != '/')
        return -1;

    size_t pospath = line.Target.find('?');
    if(pospath != std::string::npos)
    {
        line.path = line.Target.substr(0,pospath);
        line.Query = line.Target.substr(pospath + 1);    
    }
    else
    {
        line.path = line.Target;
        line.Query.clear();
    }
    index = pos + 2;
    return 1;
}

std::string RequestParser::TrimOWS( const std::string &value)
{
    std::string newVlaue;
    size_t pos = value.find_first_not_of(" \t");
    if(pos == std::string::npos)
        return "";

    size_t endpos = value.find_last_not_of(" \t");
    
    newVlaue = value.substr(pos,endpos - pos  + 1);
    return newVlaue;

}
int RequestParser::IsValidHeaderName(std::string name)
{
    std::string specaileCar = "!#$%&'*+-.^_`|~";
    if(name.empty())
        return 0;

    for(size_t i = 0; i < name.size();i++)
    {
        if(std::isalnum(static_cast<unsigned char>(name[i])))
            continue;
        if (specaileCar.find(name[i]) != std::string::npos)
            continue;
        else
            return 0;
    }
    return 1;
}


int RequestParser::ParseRequestHeader( std::string &req,HttpHeader &header,size_t &pos)
{
        size_t secondpos = req.find("\r\n",pos);
        if(secondpos == std::string::npos)
            return 0;
        if(secondpos == pos)
        {
            pos += 2;
            return 2;
        }
        std::string line =  req.substr(pos , secondpos - pos);
        size_t colon = line.find(':');
        if(colon == std::string::npos)
            return -1;

        header.name = line.substr(0, colon);
        if(IsValidHeaderName(header.name) != 1)
            return -1;
        header.value = TrimOWS(line.substr(colon + 1));


        pos = secondpos + 2;

    return 1;
}

int RequestParser::ParseHeaders(std::string &req, HttpRequest &request, size_t &pos)
{
    while(true)
    {
        HttpHeader header;
        int status = ParseRequestHeader(req,header,pos);
        if(status == -1)
            return -1;

        if(status == 0)
            return  0;

        if(status ==  2)
            return 1;

        request.headers.push_back(header);
    }
}

int RequestParser::checkHeaderNameEquals( std::string &name1, const std::string &name2)
{
    if(name1.size() != name2.size())
        return 0;
    size_t len = name1.size();

    for(size_t i = 0; i < len;i++)
    {
        unsigned char c1 = static_cast <unsigned char >(name1[i]);
        unsigned char c2 = static_cast <unsigned char >(name2[i]);


        if(std::tolower(c1) != std::tolower(c2))
            return 0;
    }

    return 1;
}
HttpHeader  *RequestParser::FindHeader(HttpRequest &request, const std::string &searchName)
{
    for(size_t i = 0; i  < request.headers.size();i++)
    {

        if(checkHeaderNameEquals(request.headers[i].name,searchName))
            return &request.headers[i];
    }

    return NULL;
}
int RequestParser::countHeader( HttpRequest &request,  const std::string &name)
{
    int count  = 0 ;
    for(size_t i = 0; i < request.headers.size();i++)
    {
        if(checkHeaderNameEquals(request.headers[i].name,name))
            count++;
    }
    return count;
}



int RequestParser::ValidateHeaders(HttpRequest &request,unsigned long &Length)
{
    request.chunked = false;
    Length = 0;
    HttpHeader *host = FindHeader(request,"Host");
    if(host == NULL || host->value.empty())
        return 400;
    int nbHost = countHeader(request,"Host");
    if(nbHost != 1)
        return 400;
    int nbCL = countHeader(request,"Content-Length");
    if(nbCL > 1)
        return 400;

    int nbTE = countHeader(request, "Transfer-Encoding");
    if(nbTE > 1)
        return 400;

    HttpHeader *transferEncoding = FindHeader(request,"Transfer-Encoding");
    if(transferEncoding != NULL)
    {
        if(nbCL == 1)
            return 400;
        if(!checkHeaderNameEquals(transferEncoding->value,"chunked"))
            return 501;
        else
            request.chunked = true;
    }
    
    HttpHeader *ContentLength = FindHeader(request,"Content-Length");
    if(nbCL == 1)
    {
        if(ContentLength->value.empty())
            return 400;
        for(size_t i = 0;i < ContentLength->value.size();i++)
        {
            if(!std::isdigit(static_cast<unsigned char>(ContentLength->value[i])))
                return 400;
        }
        if(ContentLength->value.size() > 10)
            return 413;
        Length = std::strtoul(ContentLength->value.c_str(),NULL,10);
        
        if(Length > MAX_BODY)
            return 413;
    }
    if(request.requestLine.Method =="POST" && nbCL == 0 && transferEncoding == NULL)
        return 411;
    return 1;
}

int RequestParser::ParseBody(std::string &req,HttpRequest &request,size_t &pos, unsigned long &Length)
{
    if(pos > req.size())
        return -1;
    if(Length == 0)
    {
            request.body ="";
            return 1;
    }
    size_t disp = req.size() - pos;
    if(disp < Length)
        return 0;

    request.body = req.substr(pos,Length);
    pos += Length;
    return 1;
}

int RequestParser::ParseBodyWithChunked(std::string& req,HttpRequest &request, size_t &pos)
{
    std::string tempbody = "";
    size_t cursor = pos;

    while(true)
    {
        size_t pos1 =  req.find("\r\n",cursor);
        if(pos1 == std::string::npos)
            return 0;

        std::string chunk_size = req.substr(cursor,pos1 - cursor);
        size_t semi = chunk_size.find(';');
        if(semi != std::string::npos)
            chunk_size = chunk_size.substr(0,semi);
        if(chunk_size.empty() || chunk_size.size() > 8)
            return -1;
        for(size_t i = 0; i < chunk_size.size() ; i++)
        {
            if(!std::isxdigit(static_cast<unsigned char>(chunk_size[i])))
                return -1;
        }
        unsigned long chunk_size_number = std::strtoul(chunk_size.c_str(),NULL,16);
        cursor = pos1 + 2;
        if(tempbody.size() > MAX_BODY || chunk_size_number > MAX_BODY - tempbody.size())
            return -2;

        if(chunk_size_number == 0)
        {
            while(true)
            {
                size_t end = req.find("\r\n",cursor);
                if(end == std::string::npos)
                    return 0;

                std::string trailer = req.substr(cursor,end - cursor);
                cursor = end + 2;

                if(trailer.empty())
                    break;
            }
            request.body = tempbody;
            pos = cursor;
            return 1;
        }
        if(cursor > req.size())
            return 0;
        size_t available = req.size() - cursor;
        if(chunk_size_number > available)
            return 0;

        if(available - chunk_size_number < 2)
            return 0;
        tempbody.append(req,cursor,chunk_size_number);
        cursor += chunk_size_number;
        if(req.compare(cursor,2,"\r\n") != 0)
            return -1;
        cursor += 2;
    }

    return 1;
}

int  RequestParser::ParseRequest(std::string &req, HttpRequest &request,size_t &pos)
{
    request = HttpRequest();
    pos = 0;

    if(req.find("\r\n") == std::string::npos && req.size() > MAX_REQUEST_LINE)
        return 414;
    if(req.find("\r\n\r\n") == std::string::npos && req.size() > MAX_HEADERS)
        return 431;


    unsigned long Length=0;
    int result = ParseRequestline(req,request.requestLine,pos);
    std::cout << "Requestline: " << result << std::endl;
    if(result == 0)
        return 0;
    if(result == -1)
        return 400;

    result = ParseHeaders(req,request,pos);
    std::cout << "Headers: " << result << std::endl;
    if(result == 0)
        return 0;
    if(result == -1)
        return 400;

    int  ErrType = ValidateHeaders(request,Length);
    std::cout << "ValidateHeaders: " << result << std::endl;
    if( ErrType != 1)
        return ErrType;
   
    if(request.chunked == false)
    {
        int body = ParseBody(req,request,pos,Length);
        std::cout << "ParseBody: " << result << std::endl;
        if(body == 0)
            return 0;
        if(body == -1)
            return 400;
    }
    else if(request.chunked == true)
    {
        int chunkedbody =  ParseBodyWithChunked(req,request,pos);
        if(chunkedbody == 0)
            return 0;
        if(chunkedbody == -1)
            return 400;
        if(chunkedbody == -2)
            return 413;
    }

    return 1;
}





