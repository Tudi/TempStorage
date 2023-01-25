FROM debian:buster

RUN apt-get update \
	&& apt-get -y install make=\* cmake=\* build-essential=\*  curl=\* libcurl4-openssl-dev=\* ca-certificates=\* --no-install-recommends \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/*
	
COPY . /scoring-similarity-service

WORKDIR /scoring-similarity-service/3rdparty

# Build and install 3rd party dependencies
RUN ./clean.sh && ./install.sh

WORKDIR /scoring-similarity-service

# Build application binaries
RUN make clean all 
RUN make

ENV TZ=UTC \
    LD_LIBRARY_PATH=/scoring-similarity-service/3rdparty/sentry-native/lib \
    ERROR_DIR=/scoring-similarity-service/data/.sentry-database

# Create data directories
RUN mkdir -p /scoring-similarity-service/data && mkdir -p /scoring-similarity-service/data/.sentry-database  

CMD ["./SS_server.sh"]
