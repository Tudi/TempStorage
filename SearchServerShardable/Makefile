.PHONE: all clean

all:
	$(MAKE) -C ./logger all
	$(MAKE) -C ./assert_mt all
	$(MAKE) -C ./utils all
	$(MAKE) -C ./test_utils all
	$(MAKE) -C ./tools all
	$(MAKE) -C ./models all
	$(MAKE) -C ./error_reporting all # No Dependencies
	$(MAKE) -C ./item_functions all
	$(MAKE) -C ./search all
	$(MAKE) -C ./daos all
	$(MAKE) -C ./cache_engine all
	$(MAKE) -C ./profiling
	$(MAKE) -C ./filters all
	$(MAKE) -C ./scoring all
	$(MAKE) -C ./profile_client all
	$(MAKE) -C ./profile_client_app all
	$(MAKE) -C ./profile_server all
	$(MAKE) -C ./profile_server_app all
	$(MAKE) -C ./stress_tests all

clean:
	$(MAKE) -C ./logger clean
	$(MAKE) -C ./assert_mt clean
	$(MAKE) -C ./utils clean_all
	$(MAKE) -C ./test_utils clean
	$(MAKE) -C ./tools clean
	$(MAKE) -C ./models clean_all
	$(MAKE) -C ./error_reporting clean
	$(MAKE) -C ./item_functions clean
	$(MAKE) -C ./search clean_all
	$(MAKE) -C ./daos clean_all
	$(MAKE) -C ./cache_engine clean_all
	$(MAKE) -C ./profiling clean
	$(MAKE) -C ./filters clean_all
	$(MAKE) -C ./scoring clean_all
	$(MAKE) -C ./profile_client clean_all
	$(MAKE) -C ./profile_client_app clean
	$(MAKE) -C ./profile_server clean_all
	$(MAKE) -C ./profile_server_app clean
	$(MAKE) -C ./stress_tests clean
