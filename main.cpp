#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "requests.hpp"

// MAIN FUNCTION
int main()
{
    std ::string url;
    url = "https://www.httpbin.org/headers";
    url = "https://www.google.com/search?q=Hello";

    std ::map<std ::string, std ::string> request_headers;
    request_headers["User-Agent"] = "C++";

    //NOTE: defaut timeout is 5 seconds
    requests ::Requests r = requests ::Requests();

    try
    {
        r.get(url, request_headers, 3);
    }
    catch (requests ::timeout_error &e) // Will implement a custom exception class later :)
    {
        std ::cout << e.what() << std ::endl;
        r.clear(); // Cleanup duty
        exit(EXIT_FAILURE);
    }
    catch (std ::exception &e) // catching all exceptions
    {
        std ::cout << e.what() << std ::endl;
        r.clear();
        exit(EXIT_FAILURE);
    }

    std ::string raw_response = r.get_raw_response();
    std ::string response = r.get_response();
    std ::map<std ::string, std ::string> headers = r.get_headers();
    int status_code = r.get_status_code();
    std ::string response_type = r.get_response_type();

    std ::cout << response << std ::endl;

    for (auto header : headers)
        std ::cout << header.first << ": " << header.second << std ::endl;

    // std ::cout << headers["Content-Type"] << std ::endl;

    std ::cout << status_code << std ::endl;

    std ::cout << response_type << std ::endl;

    return 0;
}