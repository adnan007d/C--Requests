#ifndef REQUESTS_HPP
#define REQUESTS_HPP

#include <arpa/inet.h>

#include <string>
#include <regex>

#define PORT 80
#define BUFFER 4096

class Requests
{
private:
    int sock;
    int valread;
    struct sockaddr_in serv_aaddr;
    char host[50];
    int timeout;
    int status_code;
    char buffer[BUFFER];
    char ip[15]; // MAX LENGTH OF IP ADDRESS

    std ::string response = "";
    std ::string headers = "";
    std ::string path = "/"; // default path will be the root directory

    void resplit(const std ::string &, std ::string);
    void extract_status_code(std ::string &);

public:
    Requests()
    {
        clear();
    }
    ~Requests()
    {
        clear();
    };

    void print_error(const char *);
    int is_end(const char *, const int);
    void setup();
    void resolve_host(const char *);

    // utils
    void substr(const char *, char *, int, int length = 0);
    void trim(std::string &);
    void ltrim(std ::string &);
    void rtrim(std::string &);
    void html_trim(std ::string &);
    void html_ltrim(std ::string &);
    void html_rtrim(std ::string &);
    bool check_trim(char);
    void clear();

    // response data
    std::string get_response();
    std ::string get_headers();
    int get_status_code();

    // http methods
    void get(const char *domain);
};

#endif // REQUESTS_HPP