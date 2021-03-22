#include <aws/core/Aws.h>
#include <aws/sqs/SQSClient.h>
#include <aws/sqs/model/SendMessageRequest.h>
#include <aws/sqs/model/SendMessageResult.h>
#include <iostream>
#include <stdio.h>
#include <string.h>	
#include <stdlib.h>	
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>
#include <pthread.h>
#include <uuid/uuid.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <openssl/md5.h>

#define MAX_WAIT_QUEUE_SIZE 		3
#define READ_SINK_SIZE				128*1024
#define SQS_MESSAGE_BUFFER_SIZE		(READ_SINK_SIZE + 1024)

//the thread function
void *connection_handler(void *);
char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length);

// Some sort of signal to let background thread they should exit. Would be great if not using blockign sockets
int AppIsRunning = 1;
// Could use atomic struct here, but we have only 1 thread accepting connections
int BackgroundThreadsActive = 0;
// Every instance of a server should have a unique ID
const char *ComputerUID = NULL;
// Messages include the port number also
int ListenPort = 0;
// Message might include local IP address also
char ServerIP[INET6_ADDRSTRLEN];
// Server Id is just a hash of the server IP
char *ServerID = NULL;

void FillLocalIP()
{
	struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;      

	// Make sure the returned result is always valid in some form
	ServerIP[0] = 0;
	
    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
	{
        if (ifa ->ifa_addr->sa_family==AF_INET) 
		{ // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			if(((char*)tmpAddrPtr)[0]==127||((char*)tmpAddrPtr)[0]==0)
				continue;
//            char ServerIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, ServerIP, INET_ADDRSTRLEN);
//            printf("'%s': %s\n", ifa->ifa_name, ServerIP); 
			break;
         } else if (ifa->ifa_addr->sa_family==AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
//            char ServerIP[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, ServerIP, INET6_ADDRSTRLEN);
//            printf("'%s': %s\n", ifa->ifa_name, ServerIP); 
			break;
        } 
    }
    if (ifAddrStruct!=NULL) 
        freeifaddrs(ifAddrStruct);//remember to free ifAddrStruct
}

struct ConnectionThreadParam
{
	int client_sock;
	struct sockaddr_in client;
};

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , *new_sock;
	struct sockaddr_in server , client;
	
	if( argc != 2 )
	{
		printf("Syntax : %s [PortNr]\n", argv[0]);
		return -1;
	}

    Aws::SDKOptions options;
    Aws::InitAPI(options);
	
	ListenPort = atoi( argv[1] );
	printf("Will listen on port : %d\n", ListenPort);
	ComputerUID = argv[2];
	
	// We will use over and over this value. Can calculate it once.
	FillLocalIP();
	
	// From server IP, we generate a server ID
	unsigned char serverID[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)ServerIP, strlen(ServerIP), serverID);
	size_t ServerIdLen = 0;
	ServerID = base64_encode((unsigned char*)serverID, MD5_DIGEST_LENGTH, &ServerIdLen);
	if( ServerID == NULL )
	{
		printf("Failed to create MD5\n");
		return -1;
	}

	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket\n");
		return -1;
	}
	printf("Socket created\n");
	
	int on = 1;
	int setOptionRet = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
    if ( setOptionRet != 0)
    {
		printf("Could not set socket option reuse. Error %d\n", setOptionRet);
    }
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( ListenPort );
	
	//Bind
	int BindRes = bind(socket_desc,(struct sockaddr *)&server , sizeof(server));
	if( BindRes < 0)
	{
		//print the error message
		printf("bind failed. Error %d\n", BindRes);
		return -1;
	}
	printf("bind done\n");
	
	//Listen
	listen(socket_desc , MAX_WAIT_QUEUE_SIZE);
	
	//Accept and incoming connection
	c = sizeof(struct sockaddr_in);
	
	//Accept and incoming connection
	printf("Waiting for incoming connections...\n");
	c = sizeof(struct sockaddr_in);
	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		printf("Connection accepted\n");
		
		pthread_t sniffer_thread;
		ConnectionThreadParam *tParams = (ConnectionThreadParam*)malloc(sizeof(ConnectionThreadParam));
		tParams->client_sock = client_sock;
		tParams->client = client;
		
		if( pthread_create( &sniffer_thread, NULL, connection_handler, (void*)tParams) < 0)
		{
			perror("could not create thread\n");
			return 1;
		}
		
		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( sniffer_thread , NULL);
		printf("Handler assigned\n");
	}
	
	if (client_sock < 0)
	{
		perror("accept failed\n");
		return 1;
	}
	
	// Signal threads that we intend to shut down
	AppIsRunning = 0;
	
	// Should have a socket manager that keeps track of active connections. At this point we tell it to close all sockets
	// Wait for threads to exit
	// If this message triggers after the shutdown procedure, means we have an issue
	if(BackgroundThreadsActive>0)
		printf("We have %d background threads still active\n",BackgroundThreadsActive);
	
    Aws::ShutdownAPI(options);
	
	free(ServerID);

	return 0;
}

// Const conversion table for base64 encoding
static const char encoding_table[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/' };
// Compiler will probably optimize this one to static code								
static const int mod_table[] = { 0, 2, 1 };

// Convert a binary string to a base64 encoded string to avoid 0 getting interpreted as a null terminated string
char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length) 
{
    *output_length = 4 * ((input_length + 2) / 3);

    char* encoded_data = (char*)malloc(*output_length + 1);
    if (encoded_data == NULL) 
        return NULL;

    for (int i = 0, j = 0; i < input_length;) 
    {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    encoded_data[*output_length] = 0;

    return encoded_data;
}

/*
 * This will handle connection for each client
 * This is a sink client. Will eat up all data that arrives to it
 * SQS message example 
 {'weather': 'sunny-77742', 
	'msg': {'session': '6ab30584-85a9-11eb-b2e0-274f00f140ac', 'cmd': 'capa'}, 
	'port': 110, 
	'server_id': 'b629366f92bf412bfbf461a4b2a98c7e58cda91c', 
	'cid': 'e1fdca1d-2896-5f4e-8188-e0e00f152f8a', 
	'sensor': 'hector', 
	'proto': 'tcp', 
	'ip':'1.1.1.1'}
 */
 
void *connection_handler(void *p)
{
	BackgroundThreadsActive++;
	
	if( p == NULL )
	{
		perror("Bad thread parameters\n");
		BackgroundThreadsActive--;
		return NULL;
	}
	
	uuid_t ConnectionUID;
	uuid_generate(ConnectionUID);
	char uuid_str[37];
	uuid_unparse_lower(ConnectionUID, uuid_str);
	
	//Get the socket descriptor
	ConnectionThreadParam *params=(ConnectionThreadParam*)p;
	int sock = params->client_sock;
	int read_size;
	char *message = (char*)malloc(READ_SINK_SIZE);
	char *messageFormatted = (char*)malloc(SQS_MESSAGE_BUFFER_SIZE);
	char *clientIP = inet_ntoa(params->client.sin_addr);
	
	if(message == NULL || messageFormatted==NULL)
	{
		perror("Memory allocation error\n");
		free(p);
		BackgroundThreadsActive--;
		return NULL;
	}

	Aws::SQS::SQSClient sqs;
	Aws::SQS::Model::SendMessageRequest sm_req;
    const Aws::String queue_url = "https://sqs.us-east-2.amazonaws.com/507620807254/DaveHudson";
    sm_req.SetQueueUrl(queue_url);
	
	//Receive a message from client
	int totalBytesSinked = 0;
	while( (read_size = recv(sock, message, READ_SINK_SIZE - 1, 0)) > 0 )
	{
		// Not required, just for the sake of statistics ( checking if works )
		totalBytesSinked += read_size;
		
		//Send the message back to client
		//write(sock , client_message , strlen(client_message));
		size_t EncodedSize = 0;
		char *EncodedMSG = base64_encode((const unsigned char*)message, read_size, &EncodedSize);
		if( EncodedMSG == NULL )
		{
			continue;
		}
		if( EncodedSize == 0 )
		{
			free(EncodedMSG);
			continue;
		}
		
		// Format message to required format
		snprintf( messageFormatted, SQS_MESSAGE_BUFFER_SIZE, 
			"{'weather':'','msg':{'session':'%s','cmd':'%s'},'port':%d,'server_ip':'%s','server_id':'%s','cid':'','sensor':'','proto': 'tcp','ip':'%s'}",
			uuid_str,EncodedMSG,ListenPort,ServerIP,ServerID,clientIP);
		
		// Convert the message to amazon string
        Aws::String msg_body = messageFormatted;
		
		// Do the internal magic so the string can be sent
        sm_req.SetMessageBody(msg_body);

		// The actual sending
        auto sm_out = sqs.SendMessage(sm_req);
        if (sm_out.IsSuccess())
        {
            std::cout << "Successfully sent message to " << queue_url << std::endl;
        }
        else
        {
            std::cout << "Error sending message to " << queue_url << ": " << sm_out.GetError().GetMessage() << std::endl;
			std::cout << "msg :" << msg_body << std::endl;
        }
		
		// We no longer need the encoded message
		free( EncodedMSG );
	}
	
	// No bytes read at all ? Dissapointing but not an issue
	if(read_size == 0)
	{
		printf("Client disconnected. Bytes swallowed %d\n", totalBytesSinked);
	}
	else if(read_size == -1)
	{
		perror("recv failed\n");
		printf("Client disconnected. Bytes swallowed %d\n", totalBytesSinked);
	}
		
	// Free the socket pointer
	free(p);
	free(message);
	free(messageFormatted);
	
	// Mark this as a dead thread. In theory we will only have 1 worker thread 
	BackgroundThreadsActive--;
	
	return 0;
}
