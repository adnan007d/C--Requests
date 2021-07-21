# C++ Requests

## Methods
- get
- More will be added later
  
## Known bugs
> If the server doesn't response the program will run forever :( Need to fix this. <br />
> Need to use url encoding for path currently it will not work if you pass path which is not encoded. <br />
> Parsing the header to be a map (for easy access) and not a string. <br />
> Allowing custom headers. <br />

## Documentation
> I don't know maybe later

## Example

```C++

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "requests.hpp"

int main()
{

    Requests r = Requests();
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

## Compile and run
You can use g++ compiler with this command to run
```bash
g++ requests.hpp requests.cpp main.cpp -o main
```
Or <br />
you can use the Makefile (You would want to customize it according to your file name) by running
```bash
make # To Compile
./main # To run
```



