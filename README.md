# C++ Requests

## Dependencies
> - ![openssl](https://github.com/openssl/openssl) (For ssl (https) connection)

## Methods
- get
- More will be added later
  
## Known bugs
> - Need to use url encoding for path currently it will not work if you pass path which is not encoded. <br />

## TODO
> - URL encoding for path of passed url

## New
> - OpenSSL support for https requests

## Documentation
### GET
>
>```C++
>requests :: Requests :: get(const char *domain, std :: map<std :: string, std :: string> >request_headers = {}, int timeout = 0)
>```
> throws an exception requests :: requests_exception see [exception](#exception) for more derived exceptions

### Methods after successful requests
> 
> ```C++
> std::string get_response();
> std ::map<std ::string, std ::string> get_headers();
> int get_status_code();
> std ::string get_raw_response();
> std ::string get_response_type(); 
> ```

### Exception
> 
> ```C++
> requests :: requests_exception
> requests :: timeout_error
> requests :: connection_error
> ```
### Some utility functions (If you want to use it)
>
> ```C++
> void trim(std::string &);
> void ltrim(std ::string &);
> void rtrim(std::string &);
> void html_trim(std ::string &);
> void html_ltrim(std ::string &);
> void html_rtrim(std ::string &);
> std ::string join(std ::vector<std ::string>, std ::string); // joins a vector of strings with the 2 parameter as the joining string
> bool check_trim(char); // util for trim, ltrim, rtrim
> ```

## Example

### Normal GET Request
```C++

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "requests.hpp"

int main()
{

    requests :: Requests r = requests :: Requests();
    r.get("https://www.google.com");

    // Currently these are the methods supported after a succesful request
    std ::string response = r.get_response();
    std ::string headers = r.get_headers();
    int status_code = r.get_status_code();

    std ::cout << response << std ::endl;

    std ::cout << headers << std ::endl;

    std ::cout << status_code << std ::endl;

    return 0;
}
```
### Custom Headers GET Request
```C++

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "requests.hpp"

int main()
{

    requests :: Requests r = requests :: Requests();

    std :: map<std :: string, std :: string> request_headers;

    request_headers["User-Agent"] = "C++";

    r.get("https://www.google.com", request_headers);
    // Currently these are the methods supported after a succesful request
    std ::string response = r.get_response();
    std ::string headers = r.get_headers();
    int status_code = r.get_status_code();

    std ::cout << response << std ::endl;

    std ::cout << headers << std ::endl;

    std ::cout << status_code << std ::endl;

    return 0;
}
```
### Timeout GET Request
```C++

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "requests.hpp"

int main()
{

    requests :: Requests r = requests :: Requests();

    try
    {
        // You can also pass custom headers in it too if you want
        r.get("https://www.google.com/Hello World", {}, 2);
    }
    catch(requests :: timeout_error &e) // Will implement a custom exception class later
    {
        std :: cout << e.what() << std :: endl;
        r.clear() // Clean Up 
       exit(EXIT_FAILURE);
    }
    // Currently these are the methods supported after a succesful request
    std ::string response = r.get_response();
    std ::string headers = r.get_headers();
    int status_code = r.get_status_code();

    std ::cout << response << std ::endl;

    std ::cout << headers << std ::endl;

    std ::cout << status_code << std ::endl;

    return 0;
}
```

## Compile and run
You can use g++ compiler with this command to run
```bash
g++ -std=c++17 requests.hpp requests.cpp main.cpp -o main -I/path/to/your/ssl/incude -L/path/to/your/ssl/linker/directory -lcrypto -lssl -Wall
```
Or <br />
you can use the Makefile (You would want to customize it according to your file name) by running
```bash
make # To Compile
./main # To run
```



