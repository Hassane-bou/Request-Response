#include "httpRequest.hpp"


int  ParseRequestline( std::string &req, HttpRequestLine &line ,size_t &index)
{

    size_t pos = req.find("\r\n");
    if(pos == std::string::npos)
        return 0;

    std::string Requestline = req.substr(0,pos);

    size_t first_space = Requestline.find(' ');
    if(first_space == std::string::npos)
        return -1;
    size_t second_space = Requestline.find(' ',first_space + 1);
    if(second_space == std::string::npos)
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

std::string TrimOWS( const std::string &value)
{
    std::string newVlaue;
    size_t pos = value.find_first_not_of(" \t");
    if(pos == std::string::npos)
        return "";

    size_t endpos = value.find_last_not_of(" \t");
    
    newVlaue = value.substr(pos,endpos - pos  + 1);
    return newVlaue;

}
int IsValidHeaderName(std::string name)
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


int ParseRequestHeader( std::string &req,HttpHeader &header,size_t &pos)
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

int ParseHeaders(std::string &req, HttpRequest &request, size_t &pos)
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

int checkHeaderNameEquals( std::string &name1, const std::string &name2)
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
 HttpHeader  *FindHeader(HttpRequest &request, const std::string &searchName)
{
    for(size_t i = 0; i  < request.headers.size();i++)
    {

        if(checkHeaderNameEquals(request.headers[i].name,searchName))
            return &request.headers[i];
    }

    return NULL;
}
int countHeader( HttpRequest &request,  const std::string &name)
{
    int count  = 0 ;
    for(size_t i = 0; i < request.headers.size();i++)
    {
        if(checkHeaderNameEquals(request.headers[i].name,name))
            count++;
    }
    return count;
}



int ValidateHeaders(HttpRequest &request,unsigned long &Length)
{
    HttpHeader *host = FindHeader(request,"Host");
    if(host == NULL || host->value.empty())
        return 400;
    int nbHost = countHeader(request,"Host");
    if(nbHost != 1)
        return 400;
    int nbCL = countHeader(request,"Content-Length");
    if(nbCL > 1)
        return 400;

    HttpHeader *transferEncoding = FindHeader(request,"Transfer-Encoding");
    if(transferEncoding != NULL)
    {
        if(nbCL == 1)
            return 400;
        if(transferEncoding->value != "chunked")
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
        unsigned long max_body = 999999999;
        if(Length > max_body)
            return 413;
    }
    if(request.requestLine.Method =="POST" && nbCL == 0 && !transferEncoding)
        return 411;
    return 0;
}

int main()
{
  std::string req = "GET /index.html HTTP/1.1\r\n"
                  "Host: localhost:8080\r\n"
                  "Content-Type: application/x-www-form-urlencoded\r\n"
                  "Content-Length: 888888888\r\n"
                  "Connection: keep-alive\r\n"
                  "\r\n"
                  "username=test";

    HttpRequest request;
    size_t pos;
    if(ParseRequestline(req,request.requestLine,pos) != 1)
    {
        std::cout << "Erreur: Incomplete\n";
        return 1;
    }

    int status = ParseHeaders(req,request,pos);

    if(status != 1)
    {
        std::cout << "Headers Error or Incomplete" << std::endl;
        return 1;
    }
    unsigned long Length;
    int err = ValidateHeaders(request,Length);
    if(err != 0)
    {
        std::cout << "Erreur HTTP: " << err << std::endl;
        return 1;
    }


    return 0;
}