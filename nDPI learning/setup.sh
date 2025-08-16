#!/bin/bash

set -e

echo "Updating package list..."
sudo apt update

echo "Installing required packages..."
sudo apt install -y \
  build-essential \
  cmake \
  libpcap-dev \
  git \
  libssl-dev \
  autoconf \
  automake \
  libtool \
  pkg-config \
  gettext \
  flex \
  bison \
  libjson-c-dev \
  libnuma-dev \
  libpcre2-dev \
  libmaxminddb-dev \
  librrd-dev \
  gdb \
  valgrind \
  libcmocka-dev \
  libmysqlclient-dev

echo "Cloning nDPI repository..."
mkdir -p dependencies
cd dependencies

if [ ! -d "nDPI" ]; then
  git clone -b 4.14-stable https://github.com/ntop/nDPI.git
else
  echo "nDPI directory already exists. Skipping clone."
fi

cd nDPI
git reset --hard
git clean -fdx
git checkout 4.14-stable

echo "Disabling unused dissectors..."
#chmod +x whitelist_ndpi.sh
#../../whitelist_ndpi.sh

echo "Building nDPI..."
./autogen.sh
./configure
make -j"$(nproc)"
sudo make install
sudo ldconfig

echo "nDPI setup complete."
