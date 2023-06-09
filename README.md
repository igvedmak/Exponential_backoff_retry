# Exponential_backoff_retry
Implementation of async exponential backoff retry in c++17.

Compilation flags:

```
 -pthread
```

Example usage:

```using namespace expbr;

#include "exp_backoff_retry.h"
#include <iostream>

// Define the function to be called
bool myFunction(int arg1, double arg2) {
    std::cout << "working...\n";
    return false;
}

int main()
{
    // Create an instance of the Exponential_backoff_retry class
    Utils::Exponential_backoff_retry retry(5, milliseconds(100), 1.5);

    // Call the function using the retry object
    auto futureResult = retry(true, myFunction, 42, 3.14);

    // Wait for the result of the function call
    bool result = futureResult.get();
    std::cout << result << std::endl;
}
```

Output:

```
working...
working...
working...
working...
working...
0
```
