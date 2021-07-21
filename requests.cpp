#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string>
#include <iostream>
#include <map>
#include <regex>
#include <sys/select.h>

#include "requests.hpp"
void Requests ::clear()
{
    valread = 0;
    timeout = 0;
    status_code = 0;

    memset(buffer, 0, BUFFER);
    memset(ip, 0, 15);
    memset(host, 0, 50);

    response.clear();
    headers.clear();
    path = "/";

    close(sock);
}

void Requests ::setup()
{
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        print_error("Socket Creation Error");

    serv_aaddr.sin_family = AF_INET;
    serv_aaddr.sin_port = htons(PORT);

    if ((inet_pton(AF_INET, ip, &serv_aaddr.sin_addr)) < 0)
        print_error("Invalid Address");

    if (connect(sock, (struct sockaddr *)&serv_aaddr, sizeof(serv_aaddr)) < 0)
        print_error("Connection Failed");
}

void Requests ::get(const char *domain)
{

    clear();
    resolve_host(domain);
    setup();

    // Setting up headers
    std ::string _headers;

    _headers += "GET " + path + " HTTP/1.1\r\n";

    _headers += "HOST: " + std ::string(host) + "\r\n\r\n";

    // printf("%s\n\n", _headers.c_str());
    send(sock, _headers.c_str(), _headers.size(), 0);
    memset(buffer, 0, BUFFER);

    // Timeout for 5 seconds if server doesn't respond it will shutdown
    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    while (1)
    {

        int sret = select(8, &readfds, NULL, NULL, &timeout);

        // printf("%d", sret);
        if (sret == 0)
            break;

        valread = recv(sock, buffer, BUFFER, 0);

        // printf("%s", buffer);
        // printf("%ld", strlen(buffer));

        response += buffer;
        fflush(stdout);

        if (is_end(buffer, valread))
            break;

        memset(buffer, 0, BUFFER);
    }
    close(sock);

    // Trimming the header therefore ltrim()
    ltrim(response);

    // Splitting header and response
    resplit(response, "\r\n\r\n");

    //extract status code

    extract_status_code(headers);

    // Trimming the right of header as left was trimmed before
    rtrim(headers);

    // Trimming everything before < (start) and after > (end)
    html_trim(response);
}

std::string Requests ::get_response()
{
    // Should I do this ?
    // I am not sure but I think it will help to save space
    response.shrink_to_fit();
    // std ::cout << response.size() << std ::endl;
    return response;
}

std ::string Requests ::get_headers()
{
    headers.shrink_to_fit();
    return headers;
}

int Requests ::get_status_code()
{
    return status_code;
}

void Requests ::resolve_host(const char *hostname)
{

    // Removing the protocal string (http, https, etc)
    std ::regex e("^(.*://)");
    std ::string __host = std ::regex_replace(hostname, e, "");

    // std ::cout << hostname << std ::endl;
    // std ::cout << __host << std ::endl;

    // Now getting the path i.e anything after / like www.google.com/search
    // so we remove search and store it into a path variable
    std ::regex f("/(.*)");
    std ::smatch sm;
    if (std ::regex_search(__host, sm, f) && sm.size() > 1)
    {
        // This is the path
        path += sm.str(1);

        // replacing the path from the host
        __host = std ::regex_replace(__host.c_str(), f, "");
    }

    // Copying the std :: string host to c string host
    strncpy(host, __host.c_str(), __host.size());

    struct hostent *he;
    struct in_addr **ip_addr;

    // If this returns NULL means No Internet or invalid domain
    if ((he = gethostbyname(host)) == NULL)
    {
        print_error("Couldn't resolve host");
    }

    // Extracting the array of ip address from hostent struct
    ip_addr = (struct in_addr **)he->h_addr_list;

    // Just a sanity check
    if (ip_addr)
    {
        // Copying the ip address into ip char array
        strcpy(ip, inet_ntoa(*ip_addr[0]));
    }
}

void Requests::print_error(const char *error)
{
    fprintf(stderr, "%s\n", error);
    exit(EXIT_FAILURE);
}

int Requests::is_end(const char *buff, const int size)
{
    // Checking if end of the request which is \r\n\r\n
    if (buff[size - 1] == '\n' && buff[size - 2] == '\r' && buff[size - 3] == '\n' && buff[size - 4] == '\r')
        return 1;

    return 0;
}

void Requests ::substr(const char *str, char *s, int start, int length)
{
    int str_len = strlen(str);
    if (!length)
        length = str_len;
    printf("%d\n", length);

    for (int i = start, x = 0; i < length && i < str_len; ++i, ++x)
    {
        s[x] = str[i];
    }
}

// Works like a fokin charm
void Requests ::resplit(const std ::string &s, std::string reg_str)
{
    std ::regex re(reg_str);

    std ::sregex_token_iterator iter(s.begin(), s.end(), re, -1);
    std ::sregex_token_iterator end;

    headers = *iter;
    ++iter;
    response = *iter;
}

void Requests ::extract_status_code(std ::string &headers)
{

    // This is a cheap solution
    std ::string ::iterator i = headers.begin();
    std ::string temp_code_string = "";

    while (*i != ' ')
    {
        headers.erase(i);
        ++i;
    }

    // trimming the space
    headers.erase(i);

    // extracting the status code in string format
    while (*i != ' ')
    {
        temp_code_string += *i;
        ++i;
    }

    // std ::cout << temp_code_string << std ::endl;
    status_code = atoi(temp_code_string.c_str());

    // removing the whole line
    while (*i != '\n')
    {
        headers.erase(i);
        ++i;
    }
    headers.erase(i);
}

void Requests ::trim(std ::string &s)
{
    ltrim(s);
    rtrim(s);
}

void Requests ::ltrim(std ::string &s)
{
    std ::string ::iterator i = s.begin();

    while (check_trim(*i))
    {
        s.erase(i);
        ++i;
    }
}

void Requests ::rtrim(std ::string &s)
{
    std ::string ::iterator i = s.end();

    --i; // end() points to one forward so making it point at the end
    while (check_trim(*i))
    {
        s.erase(i);
        --i;
    }
}

bool Requests ::check_trim(char s)
{
    if (s == '\0' || s == '\r' || s == '\n' || s == ' ')
        return true;
    return false;
}

void Requests ::html_trim(std ::string &s)
{
    html_ltrim(s);
    html_rtrim(s);
}

// Recursion Beetch xD
void Requests ::html_ltrim(std ::string &s)
{
    if (*s.begin() != '<')
    {
        s.erase(s.begin());
        html_ltrim(s);
    }
}

void Requests ::html_rtrim(std ::string &s)
{
    if (*--s.end() != '>')
    {
        s.erase(--s.end());
        html_rtrim(s);
    }
}
