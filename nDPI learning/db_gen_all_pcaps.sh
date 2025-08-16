#!/bin/bash 

./bin/db_gen -i ./pcaps/airbnb_login_browse.pcap -J 1
./bin/db_gen -i ./pcaps/american_airlines_browse.pcapng -J 2
./bin/db_gen -i ./pcaps/american_express_try_login.pcapng -J 3
./bin/db_gen -i ./pcaps/amtrak_make_acct.pcapng -J 4
./bin/db_gen -i ./pcaps/banck_of_scotland_try_register.pcapng -J 5
./bin/db_gen -i ./pcaps/Barclays_failed_login.pcapng -J 6
./bin/db_gen -i ./pcaps/lufthansa_register_check_app.pcapng -J 7
./bin/db_gen -i ./pcaps/swiss_airlines_register_check_app.pcapng -J 8
./bin/db_gen -i ./pcaps/uber_check_app.pcapng -J 9

