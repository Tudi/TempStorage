#!/bin/bash

# create directories where we will import score files
mkdir -p ./data/companies

# run the server
../ss_server_app/bin/ss_server_app --log_level debug --company_dir ./data/companies --industry_dir ./data/industries --title_dir ./data/titles --profile_dir ./data/profiles --port 3006 --request_arrival_timeout 2000 --connection_timeout 2000 --num_connections 2 &
pid1=$!
echo "pid of the server : " && echo $pid1

#wait a bit for the server to start up
sleep 2

# test an unknown type
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3006 --request "save_score" --id 1 --similarity_score_file 10003213.score --similarity_type 5
# test bad file content
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3006 --request "save_score" --id 2 --similarity_score_file resp_expected1.bin --similarity_type 1
# additional data after the real score file content
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3003 --request "save_score" --id 3 --similarity_score_file 1_badcontent.score --similarity_type 1

# allow the server to close the connections properly
sleep 1

# tell the server to terminate
kill $pid1 >/dev/null 2>&1

# allow server to shut down
timeout 2 tail --pid=$pid1 -f /dev/null
kill -9 $pid1 >/dev/null 2>&1

#check if the saved file is the same as the ones that got saved
TEST_RESULT=0
find "./data/companies" -maxdepth 0 -empty
CMP_RESULT1=$?
if [ $CMP_RESULT1 -ne 0 ]
then
	echo -e "\033[0;31m Saved file mismatch \033[0m"
	TEST_RESULT=1
fi    
	
# cleanup
rm -r ./data
	
echo "Test done"

exit $TEST_RESULT