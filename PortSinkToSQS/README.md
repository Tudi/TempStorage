# Green Sea

TCP sinkhole server

############################################################
# Description
############################################################

Listen to a specific port. Catch all incomming traffic. Add a specific ID to the packets. Pack packets into SQS messages and send it to amazon SQS service

############################################################
# Prerequisits for building it
############################################################

#! Note that to build Amazon SDK you need at least 4 GB of RAM. Free tier servers will freeze when trying to build the SDK
# Below steps are for a naked amazon EC2 instance. Based on your PC, you might not need all these steps

sudo apt-get install libcurl4-openssl-dev libssl-dev uuid-dev zlib1g-dev libpulse-dev
sudo apt-get update
sudo apt-get install cmake
sudo apt install g++
git clone https://github.com/aws/aws-sdk-cpp.git
sudo mkdir sdk_build
cd sdk_build
sudo cmake ../aws-sdk-cpp/ -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTING=OFF -DBUILD_SHARED_LIBS=ON -DSTATIC_LINKING=1 -D CMAKE_C_COMPILER="/usr/bin/gcc" -D CMAKE_CXX_COMPILER="/usr/bin/g++" -D BUILD_ONLY="sqs"
sudo make -j 4
sudo make install

# You will need to set amazon credentials in order for the SQS service to function. Set amazon acces key and secret
sudo apt install awscli
aws configure

############################################################
# Building
############################################################

# Below steps were used using a remote ssh console. I used nano to not type the contents of the files but to "paste" them from clipboard
# You can simply upload the files and build them directly
mkdir sqssink
cd sqssink
nano CMakeLists.txt
> portsinkSQS.cpp
nano portsinkSQS.cpp
cmake -Daws-sdk-cpp_DIR=../sdk_build
make

############################################################
# Usage
############################################################

#Syntax : ./portsinkSQS [PortNumber]
./portsinkSQS 25 