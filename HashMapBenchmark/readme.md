Purpuse of these tests is compare unordered_map with direct lookup map. = To know when it's worth switching to/from a direct lookup map <br>
 <br>
example run output on my laptop : <br>
<br>
KBytes allocated while running RunHashTest : 125000 <br>
KBytes allocated while running RunUnorderedMapTest : 126518 <br>
KBytes allocated while running RunLookupTableTest : 62500 <br>
KBytes allocated while running RunLookupTableTest1Indirection : 54687 <br>
KBytes allocated while running RunTreeLookupTable32Test : 64128 <br>
KBytes allocated while running RunArrayStorageTest : 47615 <br>
Running speed tests for Init + Set <br>
Time spent in                      std::HashMap : 607814.000000. Junk 0 <br>
Time spent in                    BinSearchArray : 589378.000000. Junk 0 <br>
Time spent in                              Tree : 318443.000000. Junk 0 <br>
Time spent in                std::unordered_map : 347679.000000. Junk 0 <br>
Time spent in                   1LayerLookupMap : 33718.000000. Junk 0 <br>
Time spent in                   DirectLookupMap : 65186.000000. Junk 0 <br>
Test                      std::HashMap single execution time 639665 ns, total 38379956 <br>
Test                    BinSearchArray single execution time 593762 ns, total 35625762 <br>
Test                              Tree single execution time 316779 ns, total 19006771 <br>
Test                std::unordered_map single execution time 367512 ns, total 22050763 <br>
Test                   1LayerLookupMap single execution time 35256 ns, total 2115365 <br>
Test                   DirectLookupMap single execution time 54214 ns, total 3252865 <br>
Running speed tests for Get <br>
Time spent in                      std::HashMap : 749182.000000. Junk 2000000047619 <br>
Time spent in                    BinSearchArray : 73614.000000. Junk 72029200 <br>
Time spent in                              Tree : 54446.000000. Junk 2000000047619 <br>
Time spent in                std::unordered_map : 47070.000000. Junk 2000000047619 <br>
Time spent in                   1LayerLookupMap : 40228.000000. Junk 2000000047619 <br>
Time spent in                   DirectLookupMap : 28154.000000. Junk 2000000047619 <br>
Test                      std::HashMap single execution time 759085 ns, total 45545120 <br>
Test                    BinSearchArray single execution time 70423 ns, total 4225407 <br>
Test                              Tree single execution time 55139 ns, total 3308362 <br>
Test                std::unordered_map single execution time 46680 ns, total 2800811 <br>
Test                   1LayerLookupMap single execution time 41190 ns, total 2471445 <br>
Test                   DirectLookupMap single execution time 27696 ns, total 1661789 <br>
 <br>
