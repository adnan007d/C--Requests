#include <iostream>
#include "requests.hpp"
#include <regex>
#include <ctime>
#include <fstream>
#include <filesystem>

std::string geturl(const std::string &s);

int main()
{
    std::string url = "https://api.reddit.com/r/meme?limit=25";
    std::map<std::string, std::string> headers;
    headers["User-Agent"] = "C++";

    requests::Requests r = requests::Requests();

    try
    {
        r.get(url, headers);
    }
    catch (const requests::requests_exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    std::string response = r.get_response();

    url = geturl(response);
    std::cout << "Fetched URL: " << url << '\n';
    std::filesystem::path p = url;

    if (p.extension() == "")
        std::cerr << "Bad Luck, its not a image file";
    else
    {
        r.get(url);
        response = r.get_response();
        std ::ofstream image;
        std::string filename = "image" + std::string(p.extension().c_str());
        image.open(filename, std ::ios::out);
        image << response;
        image.close();

        std::cout << filename << " Written to " << std::filesystem::current_path() << '\n';
    }

    r.clear();
}

std::string geturl(const std::string &s)
{
    std::srand(std::time(NULL));
    std::regex re(R"(stickied": (true|false), "url": "([^"]+))");
    std::regex_iterator start = std::sregex_iterator(s.begin(), s.end(), re);
    std::regex_iterator end = std::sregex_iterator();
    int size = std::distance(start, end);

    int i = 0;
    int random = std::rand() % size;

    for (auto m = start; m != end; ++m, ++i)
    {
        if (i == random)
            return (*m).str(2);
    }

    return (*start).str(2);
}
