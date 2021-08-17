#ifndef REQUESTS_HPP
#define REQUESTS_HPP

#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <regex>
#include <vector>
#include <map>

#define PORT 80
#define BUFFER 4096

namespace requests
{

    class Requests
    {
    private:
        int sock;
        int valread;
        struct sockaddr_in serv_aaddr;
        char host[50];
        int status_code;
        char buffer[BUFFER];
        char ip[15]; // MAX LENGTH OF IP ADDRESS

        std ::map<std ::string, std ::string> content_type;

        std ::string raw_response = "";
        std ::string response = "";
        std ::map<std::string, std ::string> headers;
        std ::string path = "/"; // default path will be the root directory
        std ::string response_type;

        std::vector<std ::string> resplit(const std ::string &, std ::string);
        int extract_status_code(std ::string &);
        std::map<std ::string, std ::string> format_headers(std ::string &);
        void set_content_type();
        void cook_responses();
        std ::string check_response_type(std ::map<std ::string, std::string>);

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
        void json_trim(std ::string &);
        void json_ltrim(std ::string &);
        void json_rtrim(std ::string &);

        std ::string join(std ::vector<std ::string>, std ::string);
        bool check_trim(char);
        void clear();

        // response data
        std ::string get_raw_response();
        std::string get_response();
        std ::map<std ::string, std ::string> get_headers();
        int get_status_code();
        std ::string get_response_type();

        // http methods
        void get(std ::string, std ::map<std ::string, std ::string> = {}, int = 5); // default timeout will be 5
    };

    // Exception Classes
    class requests_exception : public std ::exception
    {
    private:
        std ::string s;

    public:
        requests_exception(std ::string d) { s = d; }
        const char *what() const noexcept override { return s.c_str(); }
    };

    class timeout_error : public requests_exception
    {
    private:
        std ::string s;

    public:
        timeout_error(std ::string d = "Connection Timed Out") : requests_exception(d) { s = d; }
        const char *what() const noexcept override { return s.c_str(); }
    };

    class connection_error : public requests_exception
    {
    private:
        std ::string s;

    public:
        connection_error(std ::string d = "Connection Error") : requests_exception(d) { s = d; };
        const char *what() const noexcept override { return s.c_str(); }
    };

} // requests Namespace

#endif // REQUESTS_HPP