# About
Since there are now numerous outdated osu!mania performance calculators out there, I decided to put another one into the mix!<br>
#### BEHOLD: @mpp - A very light C++ osu!mania performance calculator
## Usage:
```cpp
#include <stdio.h>
#include "mpp.h"

int main() {
    //                    diff, score, od, notes, mods, debug
    printf(mpp::calculatePP(6.14, 891396, 9, 1928, 0, false));
    return 0;
}
```

## Example Output:
```
> ./test

351.827003
```
