#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "requests.hpp"

// MAIN FUNCTION
int main()
{

    //NOTE: defaut timeout is 5 seconds
    Requests r = Requests();
    try
    {
        std ::map<std ::string, std ::string> request_headers;

        request_headers["User-Agent"] = "C++";
        r.get("https://www.google.com", request_headers, 10);
    }
    catch (std ::logic_error &e) // Will implement a custom exception class later :)
    {
        std ::cout << e.what() << std ::endl;
        r.clear(); // Cleanup duty
        exit(EXIT_FAILURE);
    }

    std ::string response = r.get_response();
    std ::map<std ::string, std ::string> headers = r.get_headers();
    int status_code = r.get_status_code();

    // Use either one of them doesn't matter or it does ?
    std ::cout << response << std ::endl;
    // printf("%s", response.c_str());

    for (auto header : headers)
        std ::cout << header.first << ": " << header.second << std ::endl;

    // std ::cout << headers["Content-Type"] << std ::endl;

    std ::cout << status_code << std ::endl;

    return 0;
}