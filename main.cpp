#include "requestparser.hpp"



// void testRequestLine()
// {
//      std::cout << "\n===== TEST REQUEST LINE =====\n";

//     std::string tests[] = {
//         "GET /index.html HTTP/1.1\r\n",
//         "POST /login HTTP/1.1\r\n",
//         "DELETE /file.txt HTTP/1.1\r\n",
//         "PUT /index.html HTTP/1.1\r\n",
//         "GET /index.html HTTP/1.0\r\n",
//         "GET index.html HTTP/1.1\r\n",
//         "GET  /index.html HTTP/1.1\r\n",
//         "GET /index.html HTTP/1.1 EXTRA\r\n",
//         "GET /index.html HTTP/1.1"
//     };

//     for (size_t i = 0; i < 9 ;i++)
//     {
//         HttpRequestLine line;

//         size_t pos = 0;

//         int result = ParseRequestline(tests[i],line,pos);

//         std::cout << "Test " << i + 1 << " | Result: " << result << " | Position: " << pos << "\n";
//     }
// }

// void testParseHeaders()
// {
//     std::cout << "\n===== TEST PARSE HEADERS =====\n";

//     std::string req =
//         "Host: localhost:8080\r\n"
//         "Content-Type: application/json\r\n"
//         "Content-Length: 13\r\n"
//         "\r\n";

//     HttpRequest request;
//     size_t pos = 0;

//     int result = ParseHeaders(req, request, pos);

//     std::cout << "Result: " << result << "\n";
//     std::cout << "Headers count: "
//               << request.headers.size() << "\n";
//     std::cout << "Position: " << pos << "\n";
// }

// void testValidateHeaders()
// {
//     std::cout << "\n===== TEST VALIDATE HEADERS =====\n";

//     std::vector<std::string> tests;

//     tests.push_back(
//         "Host: localhost\r\nContent-Length: 13\r\n");

//     tests.push_back(
//         "Content-Length: 13\r\n");

//     tests.push_back(
//         "Host: localhost\r\nContent-Length: 13\r\n"
//         "Content-Length: 20\r\n");

//     tests.push_back(
//         "Host: localhost\r\n"
//         "Content-Length: 13\r\n"
//         "Transfer-Encoding: chunked\r\n");

//     tests.push_back(
//         "Host: localhost\r\n"
//         "Transfer-Encoding: chunked\r\n");

//     tests.push_back(
//         "Host: localhost\r\n"
//         "Content-Length: abc\r\n");

//     tests.push_back(
//         "Host: localhost\r\n"
//         "Content-Length: 999999999999999999999999\r\n");

//     tests.push_back(
//         "Host: localhost\r\n"
//         "Transfer-Encoding: gzip\r\n");
    
//     std::cout << "La liste vector est: \n";     
//     for(size_t i = 0;i < tests.size();i++)
//     {
//         std::cout << tests[i] << std::endl;
//     }


//     for (size_t i = 0; i < tests.size(); i++)
//     {
//         HttpRequest request;
//         size_t pos = 0;
//         unsigned long Length = 0;

//         std::istringstream stream(tests[i]);
//         std::string line;

//         while (std::getline(stream, line))
//         {
//             if (!line.empty() && line[line.size() - 1] == '\r')
//                 line.erase(line.size() - 1);

//             if (!line.empty())
//             {
//                 HttpHeader header;
//                 size_t colon = line.find(':');

//                 header.name = line.substr(0, colon);
//                 header.value = TrimOWS(line.substr(colon + 1));

//                 request.headers.push_back(header);
//             }
//         }

//         int result = ValidateHeaders(request, Length);

//         std::cout << "Test " << i + 1
//                   << " | Result: " << result
//                   << " | Length: " << Length
//                   << " | Chunked: " << request.chunked << "\n";
//     }
// }

// void testParseBody()
// {
//     std::cout << "\n===== TEST PARSE BODY =====\n";

//     std::string req = "username=test";

//     HttpRequest request;
//     size_t pos = 0;
//     unsigned long Length = 13;

//     int result = ParseBody(req, request, pos, Length);

//     std::cout << "Result: " << result << "\n";
//     std::cout << "Body: " << request.body << "\n";
//     std::cout << "Position: " << pos << "\n";
// }

// void testChunked()
// {
//     std::cout << "\n===== TEST CHUNKED =====\n";

//     std::string req =
//         "D\r\n"
//         "username=test\r\n"
//         "5\r\n"
//         "hello\r\n"
//         "4\r\n"
//         "test\r\n"
//         "0\r\n"
//         "\r\n";

//     HttpRequest request;
//     size_t pos = 0;

//     int result = ParseBodyWithChunked(req, request, pos);

//     std::cout << "Result: " << result << "\n";
//     std::cout << "Body: " << request.body << "\n";
//     std::cout << "Position: " << pos << "\n";
// }


int main()
{
  std::string req = "GET /index.html HTTP/1.1\r\n"
                  "Host: localhost:8080\r\n"
                  "\r\n";
    HttpRequest request;
    size_t pos = 0;

    RequestParser parser;

    int result = parser.ParseRequest(req,request,pos);
    std::cout << "Retour: " << result << std::endl;
    return 0;
}