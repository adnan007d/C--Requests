#include <iostream>
#include "requests.hpp"
#include <fstream>

// MAIN FUNCTION
int main()
{
    std ::string url;

    url = "https://api.reddit.com/r/tittydrop?limit=25";

    std ::map<std ::string, std ::string> request_headers;
    request_headers["User-Agent"] = "C++";

    requests ::Requests r = requests ::Requests();

    r.get(url, request_headers);

    std ::string response = r.get_response();

    std ::ofstream json_file;
    json_file.open("drop.json", std ::ios::out);
    json_file << response;
    json_file.close();

    return 0;
}