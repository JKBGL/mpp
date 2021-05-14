# About
Since there are now numerous outdated osu!mania performance calculators out there, I decided to put another one into the mix!<br>
#### BEHOLD: @mpp - A very light C++ osu!mania performance calculator
## Usage:
```cpp
#include <stdio.h>
#include "mpp.h"

int main() {
 // diff, score, od, notes, mods, debug
 printf(mpp::calculatePP(5.11, 746246, 9, 1462, 256, true));
 return 0;
}
```

## Example Output:
```
> ./mpp 5.11 446246 9 1462 256

DIFF 5.110000*, SCORE: 446246, OD: 9.000000, NOTES: 1462, MODS: 256
NF: 0 EZ: 0 HT: 1
score_rate: 0.500000
real_score: 892492
hit_window300: 37.000000
strain: 223.011342
pp_multiplier: 0.800000
acc: 0.000000

Result: 178.409074pp
```

## WARNING: THIS LIBRARY IS CURRENTLY BROKEN, WAIT FOR A FIX BEFORE USING.
