FROM debian:buster

RUN apt-get update \
	&& apt-get -y install make=\* cmake=\* build-essential=\*  curl=\* libcurl4-openssl-dev=\* ca-certificates=\* --no-install-recommends \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/*

COPY . /data-pipeline-fileserver

WORKDIR /data-pipeline-fileserver/3rdparty

# Build and install 3rd party dependencies
RUN ./clean.sh && ./install.sh

WORKDIR /data-pipeline-fileserver

RUN make clean
RUN make

ENV TZ=UTC \
    LD_LIBRARY_PATH=/data-pipeline-fileserver/3rdparty/sentry-native/lib \
    ERROR_DIR=/data-pipeline-fileserver/data/.sentry-database

# Create data directories
RUN mkdir -p ./data && mkdir -p ./data/.sentry-database  

CMD ["./file_server.sh"]
