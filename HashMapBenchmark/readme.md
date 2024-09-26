example run output on my laptop :

Bytes allocated while running RunHashTest : 31999936<br>
Bytes allocated while running RunUnorderedMapTest : 32388471<br>
Bytes allocated while running RunLookupTableTest : 12500000<br>
Bytes allocated while running RunLookupTableTest1Indirection : 14000028<br>
Bytes allocated while running RunTreeLookupTable32Test : 29492032<br>
Bytes allocated while running RunArrayStorageTest : 11009880<br>
Running speed tests for Init + Set<br>
Time spent in                     BenchmarkHash : 169839.000000. Junk 0<br>
Time spent in             BenchmarkArrayStorage : 26625.000000. Junk 0<br>
Time spent in            BenchmarkUnorderedHash : 133068.000000. Junk 0<br>
Time spent in        BenchmarkTreeLookuptable32 : 41004.000000. Junk 0<br>
Time spent in  BenchmarkLookuptable1Indirection : 6842.000000. Junk 0<br>
Time spent in              BenchmarkLookuptable : 8888.000000. Junk 0<br>
Test                     BenchmarkHash single execution time 179724 ns, total 1797246<br>
Test             BenchmarkArrayStorage single execution time 29674 ns, total 296747<br>
Test            BenchmarkUnorderedHash single execution time 130457 ns, total 1304571<br>
Test        BenchmarkTreeLookuptable32 single execution time 41159 ns, total 411598<br>
Test  BenchmarkLookuptable1Indirection single execution time 7158 ns, total 71588<br>
Test              BenchmarkLookuptable single execution time 9815 ns, total 98156<br>
Running speed tests for Get<br>
Time spent in                     BenchmarkHash : 179721.000000. Junk 500000<br>
Time spent in             BenchmarkArrayStorage : 40042.000000. Junk 23811<br>
Time spent in            BenchmarkUnorderedHash : 30700.000000. Junk 500000<br>
Time spent in        BenchmarkTreeLookuptable32 : 14760.000000. Junk 500000<br>
Time spent in  BenchmarkLookuptable1Indirection : 7436.000000. Junk 500000<br>
Time spent in              BenchmarkLookuptable : 4422.000000. Junk 500000<br>
Test                     BenchmarkHash single execution time 183446 ns, total 1834465<br>
Test             BenchmarkArrayStorage single execution time 41006 ns, total 410065<br>
Test            BenchmarkUnorderedHash single execution time 32148 ns, total 321485<br>
Test        BenchmarkTreeLookuptable32 single execution time 16176 ns, total 161764<br>
Test  BenchmarkLookuptable1Indirection single execution time 7414 ns, total 74149<br>
Test              BenchmarkLookuptable single execution time 4557 ns, total 45571<br>
Press any key to exit<br>
<br>
using custom hash for unordered map:<br>
<br>
Bytes allocated while running RunHashTest : 31999936<br>
Bytes allocated while running RunUnorderedMapTest : 32388471<br>
Bytes allocated while running RunLookupTableTest : 12500000<br>
Bytes allocated while running RunLookupTableTest1Indirection : 14000028<br>
Bytes allocated while running RunTreeLookupTable32Test : 29492032<br>
Bytes allocated while running RunArrayStorageTest : 11009880<br>
Running speed tests for Init + Set<br>
Time spent in                     BenchmarkHash : 157678.000000. Junk 0<br>
Time spent in             BenchmarkArrayStorage : 29755.000000. Junk 0<br>
Time spent in        BenchmarkTreeLookuptable32 : 41525.000000. Junk 0<br>
Time spent in            BenchmarkUnorderedHash : 76908.000000. Junk 0<br>
Time spent in  BenchmarkLookuptable1Indirection : 8025.000000. Junk 0<br>
Time spent in              BenchmarkLookuptable : 10324.000000. Junk 0<br>
Test                     BenchmarkHash single execution time 175779 ns, total 1757796<br>
Test             BenchmarkArrayStorage single execution time 29358 ns, total 293580<br>
Test        BenchmarkTreeLookuptable32 single execution time 43002 ns, total 430020<br>
Test            BenchmarkUnorderedHash single execution time 81083 ns, total 810832<br>
Test  BenchmarkLookuptable1Indirection single execution time 6694 ns, total 66949<br>
Test              BenchmarkLookuptable single execution time 12140 ns, total 121401<br>
Running speed tests for Get<br>
Time spent in                     BenchmarkHash : 196945.000000. Junk 500000<br>
Time spent in             BenchmarkArrayStorage : 39670.000000. Junk 23811<br>
Time spent in        BenchmarkTreeLookuptable32 : 24056.000000. Junk 500000<br>
Time spent in            BenchmarkUnorderedHash : 13729.000000. Junk 500000<br>
Time spent in  BenchmarkLookuptable1Indirection : 9522.000000. Junk 500000<br>
Time spent in              BenchmarkLookuptable : 5839.000000. Junk 500000<br>
Test                     BenchmarkHash single execution time 188435 ns, total 1884352<br>
Test             BenchmarkArrayStorage single execution time 40066 ns, total 400666<br>
Test        BenchmarkTreeLookuptable32 single execution time 15977 ns, total 159776<br>
Test            BenchmarkUnorderedHash single execution time 11033 ns, total 110339<br>
Test  BenchmarkLookuptable1Indirection single execution time 7627 ns, total 76277<br>
Test              BenchmarkLookuptable single execution time 4543 ns, total 45430<br>
Press any key to exit<br>
