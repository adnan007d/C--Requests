#include <iostream>
#include "requests.hpp"
#include <fstream>

void print_response(requests::Requests &r);

void post_example()
{
    std::string url = "https://www.httpbin.org/post";
    std::map<std::string, std::string> request_headers;

    request_headers["User-Agent"] = "C++";
    request_headers["Content-Type"] = "application/x-www-form-urlencoded";

    requests::Requests r = requests::Requests();

    std::map<std::string, std::string> data = {
        {"Animal", "Cat"},
    };

    r.post(url, data, request_headers);
    print_response(r);
}

void get_example()
{
    std::string url = "https://www.google.com";
    std::map<std::string, std::string> request_headers;

    request_headers["User-Agent"] = "C++";

    requests::Requests r = requests::Requests();

    r.get(url, request_headers);
    print_response(r);
}

void redirect_example()
{
    std::string url = "https://bit.ly/3D9nj1K";
    std::map<std::string, std::string> request_headers;

    request_headers["User-Agent"] = "C++";

    requests::Requests r = requests::Requests();

    r.get(url, request_headers);
    print_response(r);
}

// MAIN FUNCTION
int main()
{
    // get_example();
    // post_example();
    redirect_example();
    return 0;
}

void print_response(requests::Requests &r)
{

    std::string response = r.get_response();
    std::string raw_response = r.get_raw_response();
    std::string response_type = r.get_response_type();
    std::map<std::string, std::string> response_headers = r.get_headers();
    int status_code = r.get_status_code();

    std::cout << response << "\n\n";
    std::cout << "response_type = " << response_type << "\n\n";
    std::cout << "Status Code = " << status_code << "\n\n";

    for (const auto &header : response_headers)
    {
        std::cout << header.first << " -> " << header.second << '\n';
    }
}