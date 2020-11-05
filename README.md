### Description:

This Lua module implements an userdata type for arbitrary big integers as BCD numbers.
It defines the arithmetic binary operators '+','-','/','*','%' and the unary '-' operator.

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
The BCD numbers implementation originates from the project https://github.com/Wolframe/Wolframe.
I implemented the module in 2013 for this project that is dead since summer 2014.
The code is reissued under a new licence (MIT) with the concent of the co-authors of the projects.


