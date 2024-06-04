#include <iostream>
#include <fstream>
#include <cstring>
#include <chrono>
#include <thread>
#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

#define MAX_LINE_LENGTH 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8001

int InitClientServerConnection(SOCKET& sockfd, struct sockaddr_in& server_addr)
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Server address setup
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
#ifdef _WIN32
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address or address not supported" << std::endl;
        return 1;
    }
#else
    if (inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr)) != 1)
    {
        std::cerr << "Invalid address or address not supported" << std::endl;
        close(sockfd);
        return 1;
    }
#endif
    server_addr.sin_port = htons(SERVER_PORT);

    return 0;
}

int SendFileToServer(const char *fileName, SOCKET sockfd, struct sockaddr_in& server_addr)
{
    // Open file
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }

    char line[MAX_LINE_LENGTH];

    unsigned __int64 prevTimeStamp = 0;
    unsigned __int64 lineCounter = 0;

    // Read file line by line
    while (file.getline(line, MAX_LINE_LENGTH))
    {
        size_t lineLen = strlen(line);
        // strangely short line
        if (lineLen < 10)
        {
            continue;
        }

        unsigned __int64 pktTimeStamp = strtoull(line, NULL, 10);;

        lineCounter++;
        std::cout << "Sending : " << lineCounter << " " << line << std::endl;

        // Send line as UDP packet
        int bytes_sent = sendto(sockfd, line, (int)lineLen, 0,
            (struct sockaddr*)&server_addr, (int)sizeof(server_addr));
        if (bytes_sent < 0)
        {
            std::cerr << "Error sending packet" << std::endl;
            break;
        }
        else if (bytes_sent == 0)
        {
            std::cout << "Sent 0 bytes" << std::endl;
        }
        else if (lineLen != bytes_sent)
        {
            std::cout << "Sent " << bytes_sent << "Expected " << lineLen << std::endl;
        }

        // Wait for a short time before sending the next packet
        if (prevTimeStamp != 0 && pktTimeStamp > prevTimeStamp)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(pktTimeStamp - prevTimeStamp));
//            std::this_thread::sleep_for(std::chrono::milliseconds(5)); // for debug
//            if (lineCounter == 100)break;
        }
        prevTimeStamp = pktTimeStamp;
    }

    // Close file and socket
    file.close();

    std::cout << "Done sending file content" << std::endl;

    return 0;
}

int main()
{
    
    SOCKET sockfd;
    struct sockaddr_in server_addr;
    if (InitClientServerConnection(sockfd, server_addr) != 0)
    {
#ifdef _WIN32
        if (sockfd > 0)
        {
            closesocket(sockfd);
        }
        WSACleanup();
#else
        close(sockfd);
#endif
        return 1;
    }

    std::cout << "Sending file content for every ENTER until 'x' command" << std::endl;
    while (1)
    {
        std::string input;
        std::getline(std::cin, input);
        if (input.length() == 1 && input.c_str()[0] == 'x')
        {
            break;
        }
        if (SendFileToServer("LocationModuleMessages.txt", sockfd, server_addr) != 0)
        {
            break;
        }
    }

#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif

    return 0;
}
