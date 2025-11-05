How many colors do you need in order to color all the numbers from 1 to N, while maintining the rule of non monotonic coloring : color(x)!=color(y)!=color(x+y) where x+y=N
if N = 1 you need 1 color		c1
if N = 2 you need 2 colors		c1,c2
if N = 3 you need 2 colors		[c1,c2,c2]	or 	[c1,c2,c1]
if N = 4 you need 2 colors		[c1,c2,c2,c1]
if N = 5 you need 3 colors		[c1,c2,c2,c1,c3]
if N = 6 you need 3 colors		[c1,c2,c2,c1,c3,c1]

This program has a dummy approach, it keeps expanding the number of coloring options and never reduce it.
Todo : Eliminate candidates that are no longer valid for next N+1
Todo : Add multi threading
Todo : Make a better save/load