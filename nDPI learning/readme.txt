This is an unfinished project. Note to self for next time : work on ndpiSimpleIntegration.c 

Description :
	mode1 : Tool to process prerecorded PCAP files and extract all used JA3,JA4,Certificates, DNS related values. These values will be added to a DB so we can recognize certain flows
	mode2 : Tool to "guess" net traffic based on previously prerecorded fingerprints
	
Requirements:
	chmod +x setup.sh
	./setup.sh
	
Build:
	make
	
Usage:
	mode1 :
		ex : ./bin/db_gen -i ./pcaps/american_airlines_browse.pcapng -J 1
	mode2 :
		ex : ./bin/flow_inspector -i ./pcaps/american_airlines_browse.pcapng
	
Testing:
	- build tests :
		make tests
	- run tests :
		./bin/test_runner
		
Debugging:
	make clean
	make debug=1 debug_stat_allocs=1
	valgrind --leak-check=full --show-leak-kinds=all ./bin/db_gen -i ./pcaps/tls12.pcapng -J 1
	valgrind --leak-check=full --show-leak-kinds=all ./bin/flow_inspector -i ./pcaps/american_airlines_browse.pcapng > out.txt