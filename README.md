### Description
This Lua module written in C++ implements an userdata type for arbitrary big integers as BCD numbers.
It defines the arithmetic binary operators  **+** **-** **/** ** * ** **%** **^** and the unary **-** operator.

#### Example

```lua
bcd = require "bcd"
local result = bcd.int( "1091274089731205741574315105408501238459018244") + "9837450983259878234932079584098479356329382873490537340570384"
print( result)
```

#### Output
```
9837450983259879326206169315304220930644488281991775799588628
```

#### Origin
The implementation is based on the paper [BCD Arithmetic, a Tutorial](http://homepage.divms.uiowa.edu/~jones/bcd/bcd.html)
from Douglas W. Jones from the University of Iowa.
I implemented it in 2013 for the project https://github.com/Wolframe/Wolframe that is dead since summer 2014.
The code is now reissued under a new licence (MIT) with the consent of the co-authors of the projects.

