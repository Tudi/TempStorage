.PHONE: all clean

all:
	$(MAKE) -C ./logger all
	$(MAKE) -C ./assert_mt all
	$(MAKE) -C ./utils all
	$(MAKE) -C ./profiling
	$(MAKE) -C ./error_reporting all # No Dependencies 
	$(MAKE) -C ./score_manager
	$(MAKE) -C ./ss_client all
	$(MAKE) -C ./ss_client_app all
	$(MAKE) -C ./ss_server all
	$(MAKE) -C ./ss_server_app all

clean:
	$(MAKE) -C ./logger clean
	$(MAKE) -C ./assert_mt clean
	$(MAKE) -C ./utils clean_all
	$(MAKE) -C ./profiling clean
	$(MAKE) -C ./error_reporting clean 
	$(MAKE) -C ./score_manager clean
	$(MAKE) -C ./ss_client clean_all
	$(MAKE) -C ./ss_client_app clean
	$(MAKE) -C ./ss_server clean_all
	$(MAKE) -C ./ss_server_app clean
