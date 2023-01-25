FROM debian:buster as builder

WORKDIR /scoring-client

# Install system dependencies
RUN apt-get update && apt-get install -y make cmake build-essential valgrind curl libcurl4-openssl-dev lcov

# Copy all the source code into the container
COPY . .

# Build and install 3rd party dependencies
RUN cd 3rdparty && ./clean.sh && ./install.sh

# Build application binaries
RUN make clean

# Environment configuration
ENV TZ=UTC \
    LD_LIBRARY_PATH=/scoring-client/3rdparty/json-c/lib:/scoring-client/3rdparty/cmocka/lib:/scoring-client/3rdparty/sentry-native/lib \
    FERRARI_C_TEST_LIB_PATH=/scoring-client/3rdparty/json-c/lib:/scoring-client/3rdparty/cmocka/lib \
    ERROR_DIR=/scoring-client/data/.sentry-database \
    CODACY_PROJECT_TOKEN=ff7c7d6a81e645ff9d589d264da64fec

ARG TARGET

RUN if [ "$TARGET" = "test" ] ; then  \
    make DEBUG=1 COVERAGE=1 all && \
    ./run_all_tests.sh && \
    ./generate_coverage_report.sh generate && \
    curl -Ls https://coverage.codacy.com/get.sh -o codacy.sh && \
    chmod u+x codacy.sh && \
    ./codacy.sh report  -l C --force-language -r coverage.info; \
    else make all && \
    mkdir -p /scoring-client/data/profiles && mkdir -p /scoring-client/data/companies && mkdir -p /scoring-client/data/.sentry-database; \
    fi

CMD ["./profile_server_app/bin/profile_server_app", "--server_id", "1", "--num_servers", "1", "--profile_dir", "./data/profiles", "--num_profile_cache_entries", "5000000", "--num_profiles_per_file", "128", "--company_dir", "./data/companies", "--num_company_cache_entries", "3000000", "--num_companies_per_file", "128", "--port", "3002", "--timeout", "200", "--profile_cache_update", "20", "--company_cache_update", "20", "--num_connections", "4", "--num_search_threads", "4","--search_results_expiration", "6","--log_level", "info"]
