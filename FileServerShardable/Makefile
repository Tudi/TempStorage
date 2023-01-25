.PHONE: all clean

all:
	$(MAKE) -C ./logger all
	$(MAKE) -C ./utils all
	$(MAKE) -C ./daos all
	$(MAKE) -C ./profiling
	$(MAKE) -C ./fs_client all
	$(MAKE) -C ./fs_client_app all
	$(MAKE) -C ./fs_server all
	$(MAKE) -C ./fs_server_app all

clean:
	$(MAKE) -C ./logger clean
	$(MAKE) -C ./utils clean_all
	$(MAKE) -C ./daos clean_all
	$(MAKE) -C ./profiling clean
	$(MAKE) -C ./fs_client clean_all
	$(MAKE) -C ./fs_client_app clean
	$(MAKE) -C ./fs_server clean_all
	$(MAKE) -C ./fs_server_app clean
