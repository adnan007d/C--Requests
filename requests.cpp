#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string>
#include <iostream>
#include <chrono>
#include <regex>
#include <sys/select.h>
#include <sstream>
#include <iomanip>
#include "requests.hpp"

int ssl_error_callback(const char *, size_t, void *);

void requests::Requests::clear()
{
    this->valread = 0;
    this->status_code = 0;
    this->timeout = 0;
    this->response_headers_length = -1;
    this->content_length = -1;

    memset(this->buffer, 0, BUFFER);
    memset(this->ip, 0, 15);
    memset(this->host, 0, 50);

    this->content_type.clear();
    this->protocol.clear();
    this->raw_response.clear();
    this->response.clear();
    this->headers.clear();
    this->response_type.clear();
    this->path = "/";
}

void requests::Requests::setup(int port)
{
    if ((this->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw requests::connection_error("Socket Creation Error");

    this->serv_aaddr.sin_family = AF_INET;
    this->serv_aaddr.sin_port = htons(port);

    if ((inet_pton(AF_INET, ip, &this->serv_aaddr.sin_addr)) < 0)
        throw requests::connection_error("Invalid Address");

    if (connect(this->sock, (struct sockaddr *)&this->serv_aaddr, sizeof(this->serv_aaddr)) < 0)
        throw requests::connection_error("Connection Failed");
}

void requests::Requests::get(std::string domain, std::map<std::string, std::string> request_headers, int timeout)
{

    make_request(domain, "GET", request_headers);
    cook_responses();
}

void requests::Requests::post(std::string domain, std::map<std::string, std::string> data, std::map<std::string, std::string> request_headers, int timeout)
{

    std::string string_data = format_post_data(data);

    request_headers["Content-Length"] = std::to_string(string_data.size());

    make_request(domain, "POST", request_headers, string_data);
    cook_responses();
}

void requests::Requests::post(std::string domain, std::string data, std::map<std::string, std::string> request_headers, int timeout)
{

    data = url_encode(data);
    request_headers["Content-Length"] = std::to_string(data.size());
    make_request(domain, "POST", request_headers, data);
    cook_responses();
}

void requests::Requests::make_request(std::string domain, std::string method, std::map<std ::string, std ::string> request_headers, std ::string data)
{

    this->protocol = get_protocol(domain);

    if (this->protocol == "http")
    {
        connect_without_ssl(domain, method, request_headers, data);
    }
    else if (this->protocol == "https")
    {
        connect_with_ssl(domain, method, request_headers, data);
    }
    else
    {
        throw requests::requests_exception("Unknown Protocol (valid protocols are http, https)");
    }
}

std::string requests::Requests::format_post_data(std::map<std::string, std::string> data)
{
    std::string string_data = "";
    int i = 0;
    for (auto &x : data)
    {
        if (i != 0)
            string_data += "&";

        string_data += x.first;
        string_data += "=";
        string_data += url_encode(x.second);
        ++i;
    }
    return string_data;
}
void requests::Requests::connect_without_ssl(std::string domain, std::string method, std::map<std::string, std::string> request_headers, std::string data)
{
    clear();
    set_content_type();
    this->timeout = timeout;

    resolve_host(domain.data());

    // Setting up headers
    std::string _headers = format_request_headers(method, request_headers, data);

    setup(80); // Sets up the sockets

    send(this->sock, _headers.c_str(), _headers.size(), 0);
    memset(this->buffer, 0, BUFFER);

    // To check if we are recieving something
    fd_set fds;

    struct timeval timeout_struct;

    // This is additional feature to check if the response is still empty after a given timeout
    // This is from stackoverflow https://stackoverflow.com/questions/728068/how-to-calculate-a-time-difference-in-c
    std::chrono::time_point<clock_> begin_time_ = clock_::now();

    while (1)
    {

        FD_ZERO(&fds);
        FD_SET(this->sock, &fds);

        timeout_struct.tv_sec = this->timeout;
        timeout_struct.tv_usec = 0;

        if ((int)this->response_headers_length != -1 && (int)this->content_length != -1 && (raw_response.size() - this->response_headers_length) >= this->content_length)
            break;
        if (this->timeout != 0)
        {

            int pret = select(8, &fds, NULL, NULL, &timeout_struct);

            double diff;

            if (pret == 0)
            {
                if (this->raw_response.empty())
                {
                    close(this->sock);
                    throw requests::timeout_error("Timeout"); // Will implement a Exception Class
                }
                break;
            }
            else if (pret == 1)
            {
                diff = std::chrono::duration_cast<second_>(clock_::now() - begin_time_).count();
                if (this->timeout != 0 && diff > this->timeout)
                {
                    if (this->raw_response.empty())
                    {
                        close(this->sock);
                        throw requests::timeout_error("Tiemout");
                    }
                    else
                    {
                        close(this->sock);
                        throw requests::timeout_error("Timeout occured after fetching " + std::to_string(this->raw_response.size()) + " Bytes");
                    }
                }
            }
        }
        this->valread = recv(this->sock, this->buffer, BUFFER, 0);

        append_raw_response(this->raw_response, this->buffer, this->valread);

        if (this->valread <= 0)
            break;

        // URL Redirection and extracting content length and headers length
        if (this->raw_response.size() > 0)
        {

            set_headers_and_content_length();
            std::string location = check_redirect();

            if (!location.empty())
            {
                make_request(location, method, request_headers, data);
                return;
            }
        }

        // if (is_end(this->buffer, this->valread))
        // break;
        memset(this->buffer, 0, BUFFER);
    }
    close(this->sock);
}

void requests::Requests::connect_with_ssl(std::string domain, std::string method, std::map<std::string, std::string> request_headers, std::string data)
{
    clear();
    set_content_type();
    this->timeout = timeout;

    resolve_host(domain.data());
    // Setting up headers
    std::string _headers = format_request_headers(method, request_headers, data);

    setup(443); // Sets up the sockets

    this->ctx = init_ctx();
    this->ssl = SSL_new(this->ctx);
    SSL_set_fd(this->ssl, this->sock);

    // Setting up hostname to connect
    SSL_set_tlsext_host_name(this->ssl, this->host);

    if (SSL_connect(this->ssl) <= 0)
    {
        ERR_print_errors_cb(&ssl_error_callback, NULL);
    }

    SSL_write(this->ssl, _headers.c_str(), _headers.size());

    memset(this->buffer, 0, BUFFER);

    fd_set sockfds;

    struct timeval timeout_struct;

    // This is additional feature to check if the response is still empty after a given timeout
    // This is from stackoverflow https://stackoverflow.com/questions/728068/how-to-calculate-a-time-difference-in-c
    std::chrono::time_point<clock_> begin_time_ = clock_::now();

    while (1)
    {
        FD_ZERO(&sockfds);
        FD_SET(sock, &sockfds);

        timeout_struct.tv_sec = this->timeout;
        timeout_struct.tv_usec = 0;

        if ((int)this->response_headers_length != -1 && (int)this->content_length != -1 && (raw_response.size() - this->response_headers_length) >= this->content_length)
            break;

        if (this->timeout != 0)
        {

            int sret = select(8, &sockfds, NULL, NULL, &timeout_struct);

            double diff;

            if (sret == 0)
            {
                if (this->raw_response.empty())
                {
                    close(this->sock);
                    SSL_CTX_free(this->ctx);
                    throw requests::timeout_error("Timeout"); // Will implement a Exception Class
                }
                break;
            }
            else if (sret == 1)
            {
                diff = std::chrono::duration_cast<second_>(clock_::now() - begin_time_).count();
                if (diff > this->timeout)
                {
                    if (this->raw_response.empty())
                    {
                        close(this->sock);
                        SSL_CTX_free(this->ctx);
                        throw requests::timeout_error("Tiemout");
                    }
                    else
                    {
                        close(this->sock);
                        SSL_CTX_free(this->ctx);
                        throw requests::timeout_error("Timeout occured after fetching " + std::to_string(this->raw_response.size()) + " Bytes");
                    }
                }
            }
        }

        this->valread = SSL_read(this->ssl, this->buffer, BUFFER);

        append_raw_response(this->raw_response, this->buffer, this->valread);
        if (this->valread <= 0)
            break;

        // URL Redirection and extracting content length and headers length
        if (this->raw_response.size() > 0)
        {

            set_headers_and_content_length();

            std::string location = check_redirect();
            if (!location.empty())
            {
                close(this->sock);
                SSL_CTX_free(this->ctx);
                make_request(location, method, request_headers, data);
                return;
            }
        }

        // if (is_end(this->buffer, this->valread) && this->raw_response.size() != this->response_headers_length)
        // break;

        memset(this->buffer, 0, BUFFER);
    }
    close(this->sock);
    SSL_CTX_free(this->ctx);
}

SSL_CTX *requests::Requests::init_ctx()
{
    SSL_library_init();

    const SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL)
    {
        // requests::Requests::Error = "CTX Creation Error";
        ERR_print_errors_cb(&ssl_error_callback, NULL);
    }

    return ctx;
}

std::string requests::Requests::url_encode(std::string_view s)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (auto i = s.begin(); i != s.end(); ++i)
    {
        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char)c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}

void requests::Requests::set_headers_and_content_length()
{
    // Parsing headers
    std::smatch sm;
    std::regex head("\r\n\r\n");

    // Only if there is a \r\n\r\n (seperator between header and response)
    // And if we have no headers before
    if (std::regex_search(this->raw_response, sm, head) && (int)this->response_headers_length == -1)
    {

        std::vector<std::string> parts = resplit(this->raw_response, "\r\n\r\n");
        if (parts.size() > 0)
        {
            std::string _h = parts.at(0);
            std::regex length("Content-Length: (.*)\r\n", std::regex_constants::icase);
            this->response_headers_length = _h.size() + 4; // Adding 4 as we strip \r\n\r\n

            if (std::regex_search(_h, sm, length) && sm.size() > 1)
                this->content_length = stoi(sm.str(1));
        }
    }
}

std::string requests::Requests::check_redirect()
{
    int s_code = extract_status_code(this->raw_response);
    std::string location = "";

    // If status code is redirect (300-399 theoratically)
    if (s_code >= 300 && s_code <= 399)
    {
        std::regex l("Location: (.*)\r\n", std::regex_constants::icase);
        std::smatch sm;

        if (std::regex_search(this->raw_response, sm, l) && sm.size() > 1)
        {
            location = sm.str(1);
        }
        return location;
    }
    return location;
}

void requests::Requests::append_raw_response(std::string &s, char *buffer, int buffer_len)
{
    for (int i = 0; i < buffer_len; ++i)
    {
        s.push_back(buffer[i]);
    }
}

int ssl_error_callback(const char *str, size_t len, void *u)
{
    throw requests::requests_exception(/*requests::Requests::Error + ":\n"+*/ str);
}

std::string requests::Requests::get_raw_response()
{
    this->raw_response.shrink_to_fit();
    return this->raw_response;
}

std::string requests::Requests::get_response()
{
    // Should I do this ?
    // I am not sure but I think it will help to save space
    this->response.shrink_to_fit();

    return this->response;
}

std::map<std::string, std::string> requests::Requests::get_headers()
{
    return this->headers;
}

int requests::Requests::get_status_code()
{
    return this->status_code;
}

std::string requests::Requests::get_response_type()
{
    this->response_type.shrink_to_fit();
    return this->response_type;
}

std::string requests::Requests::format_request_headers(std::string method, std::map<std::string, std::string> request_headers, std::string data)
{
    std::string _headers;

    _headers += method + " " + this->path + " HTTP/1.1\r\n";

    _headers += "HOST: " + std::string(this->host) + "\r\n";

    _headers += "Connection: Close\r\n";

    for (auto &x : request_headers)
    {
        // Trimming the value part of headers
        // Really some users are dumb smh
        // Or some are very smart to use it and break stuff
        trim(x.second);

        _headers += x.first + ": " + x.second + "\r\n";
    }

    _headers += "\r\n"; // Requests end with \r\n\r\n

    _headers += data;

    return _headers;
}

void requests::Requests::resolve_host(const char *hostname)
{
    // Removing the protocal string (http, https, etc)

    std::regex e("(^(https?)://)");
    std::cmatch cm;

    std::string __host;
    if (std::regex_search(hostname, cm, e) && cm.size() > 0)
    {
        __host = std::regex_replace(hostname, e, "");
        this->protocol = cm.str(2);
    }
    else
    {
        // TODO: Add a exception class
        throw requests::requests_exception("Invalid url: Must contain http/https protocols");
    }

    // Now getting the path i.e anything after / like www.google.com/search
    // so we remove search and store it into a path variable
    std::regex f("/(.*)");
    std::smatch sm;
    if (std::regex_search(__host, sm, f) && sm.size() > 1)
    {
        // This is the path
        this->path += sm.str(1);

        // replacing the path from the host
        __host = std::regex_replace(__host.c_str(), f, "");
    }

    // Copying the std:: string host to c string host
    strncpy(this->host, __host.c_str(), __host.size());

    struct hostent *he;
    struct in_addr **ip_addr;

    // If this returns NULL means No Internet or invalid domain
    if ((he = gethostbyname(this->host)) == NULL)
    {
        throw requests::connection_error("Couldn't resolve host");
    }

    // Extracting the array of ip address from hostent struct
    ip_addr = (struct in_addr **)he->h_addr_list;

    // Just a sanity check
    if (ip_addr)
    {
        // Copying the ip address into ip char array
        strcpy(this->ip, inet_ntoa(*ip_addr[0]));
    }
}

void requests::Requests::print_error(const char *error)
{
    fprintf(stderr, "%s\n", error);
    exit(EXIT_FAILURE);
}

int requests::Requests::is_end(const char *buff, const int size)
{
    // Checking if end of the request which is \r\n\r\n
    if (buff[size - 1] == '\n' && buff[size - 2] == '\r' && buff[size - 3] == '\n' && buff[size - 4] == '\r')
        return 1;

    return 0;
}

void requests::Requests::substr(const char *str, char *s, int start, int length)
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

void requests::Requests::cook_responses()
{
    // Do nothing when raw response is not filled
    if (this->raw_response.size() <= 0)
        return;

    // Trimming the header therefore ltrim()
    ltrim(this->raw_response);

    // Splitting header and response
    std::vector<std::string> parts = resplit(this->raw_response, "\r\n\r\n");

    std::string response_headers = "";
    if (parts.size() == 1)
    {
        // This mainly occurs during permanent redirect
        response_headers = parts.at(0);
        // this->response = this->raw_response;
    }
    else if (parts.size() == 2) // This is the common result
    {
        response_headers = parts.at(0);
        this->response = parts.at(1);
    }
    else
    {
        // This when someone wants to mess with you
        throw requests::requests_exception("Recieved unexpected response");
    }

    // Trimming the right of header as left was trimmed before
    rtrim(response_headers);

    //extract status code
    this->status_code = extract_status_code(response_headers);

    this->headers = format_headers(response_headers);

    this->response_type = check_response_type(headers);

    if (this->response.size() > 0)
    {
        if (this->response_type == "json")
            json_trim(this->response);
        else
            trim(this->response);
    }
}

void requests::Requests::set_content_type()
{
    this->content_type.insert(std::make_pair("html", "text/html"));
    this->content_type.insert(std::make_pair("json", "application/json"));
    this->content_type.insert(std::make_pair("plain", "text/plain"));
}

std::string requests::Requests::check_response_type(std::map<std::string, std::string> headers)
{
    for (auto header : content_type)
    {
        size_t type_len = header.second.size();
        if (headers["Content-Type"].substr(0, type_len) == header.second)
        {
            return header.first;
        }
    }
    std::string t = "";
    for (auto x : headers["Content-Type"])
    {
        if (t == ";")
            break;
        t += x;
    }
    return t;
}

// Works like a fokin charm
std::vector<std::string> requests::Requests::resplit(const std::string &s, std::string reg_str)
{
    std::vector<std::string> parts;
    std::regex re(reg_str);

    std::sregex_token_iterator iter(s.begin(), s.end(), re, -1);
    std::sregex_token_iterator end;

    while (iter != end)
    {
        parts.push_back(*iter);
        ++iter;
    }

    return parts;
}

int requests::Requests::extract_status_code(std::string &headers)
{

    // This is a cheap solution
    std::string::iterator i = headers.begin();
    std::string temp_code_string = "";

    while (*i != ' ')
    {
        ++i;
    }

    ++i;

    // extracting the status code in string format
    while (*i != ' ')
    {
        temp_code_string += *i;
        ++i;
    }

    return atoi(temp_code_string.c_str());
}

std::map<std::string, std::string> requests::Requests::format_headers(std::string &headers)
{
    // Removing the first line which contains the protocal and status code
    while (*headers.begin() != '\n')
    {
        headers.erase(headers.begin());
    }
    headers.erase(headers.begin());

    std::vector<std::string> _headers = resplit(headers, "\r\n");

    std::map<std::string, std::string> real_headers;

    // Now extracting single headers and converting it into a map
    for (auto header : _headers)
    {
        std::vector<std::string> _header = resplit(header, ": ");

        std::string key = "";

        // Doing this because there might be a `: ` in value and it will be splitted too so we will join them
        std::string value = "";

        // Setting the first element as key
        key = _header.at(0);

        if (_header.size() > 1)
        {
            // Poping the first element as we would want to join the vector if the size is greater than 1
            _header.erase(_header.begin());
            if (_header.size() > 1)
                value = join(_header, ": ");
            else
                value = _header.at(0);
        }
        else // If for some reason there is no value for key
            value = "0";

        real_headers[key] = value;
    }
    return real_headers;
}

std::string requests::Requests::get_protocol(std::string domain)
{
    std::regex e("(^(https?)://)");
    std::smatch sm;

    std::string _protocol = "";

    if (std::regex_search(domain, sm, e) && sm.size() > 0)
    {
        _protocol = sm.str(2);
    }

    return _protocol;
}

std::string requests::Requests::join(std::vector<std::string> vec, std::string sep)
{
    std::string s = "";
    for (auto v : vec)
    {
        s += v;
        if (!s.empty())
            s += sep;
    }

    return s;
}

void requests::Requests::trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

void requests::Requests::ltrim(std::string &s)
{
    while (check_trim(*s.begin()))
        s.erase(s.begin());
}

void requests::Requests::rtrim(std::string &s)
{

    while (check_trim(*--s.end()))
        s.erase(--s.end());
}

bool requests::Requests::check_trim(char s)
{
    if (s == '\r' || s == '\n' || s == ' ')
        return true;
    return false;
}

void requests::Requests::html_trim(std::string &s)
{
    html_ltrim(s);
    html_rtrim(s);
}

// Recursion Beetch xD
void requests::Requests::html_ltrim(std::string &s)
{
    if (*s.begin() != '<')
    {
        s.erase(s.begin());
        html_ltrim(s);
    }
}

void requests::Requests::html_rtrim(std::string &s)
{
    if (*--s.end() != '>')
    {
        s.erase(--s.end());
        html_rtrim(s);
    }
}

void requests::Requests::json_trim(std::string &s)
{
    json_ltrim(s);
    json_rtrim(s);
}

void requests::Requests::json_ltrim(std::string &s)
{
    if (*s.begin() != '{')
    {
        s.erase(s.begin());
        json_ltrim(s);
    }
}

void requests::Requests::json_rtrim(std::string &s)
{
    if (*--s.end() != '}')
    {
        s.erase(--s.end());
        json_rtrim(s);
    }
}