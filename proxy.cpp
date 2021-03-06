#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <signal.h>
#include <vector>
#include <map>
#include <iterator> 

using namespace std;

/* Global Variables */

#define MAX_BACK_LOG (5)
#define MAX_REQUEST_LENGTH (4096)
#define MAX_RESPONSE_LENGTH (524288)
int server_port;
int cache_size;

/* Structs */
struct node {
	char * val;
	int size;
	node * next;
	node * prev;
};

typedef struct LRUCache {
	struct cmp_str
		{
			bool operator()(char const *a, char const *b)
		{
			return std::strcmp(a, b) < 0;
		}
	};

	map<char *, node*, cmp_str> nodeMap;
	vector<node*> freeNodes;
	node * head;
	node * tail;
	node * entries;
} LRUCache_t;

LRUCache_t myCache;

typedef struct thread_params {
  int sock;
  char request[MAX_REQUEST_LENGTH];
  int request_size;
} thread_params_t;

/* Function Prototypes */

void* handle_requests(void* input_params);
void get_host_response(char * addr, uint16_t port, char * request, int request_size, int client_sock, char * request_path);
int check_cache(char * request, int client_sock);
void add_to_cache(char * request, char * response, int response_size);
void removeNode (node *n);
void setHeadNode (node *n);

int main (int argc, char* argv[])
{

  int sock, client_sock;
  struct sockaddr_in server_addr;
  char request[MAX_REQUEST_LENGTH];

  memset(request, 0, MAX_REQUEST_LENGTH);

  // Check to see if correct number of inputs
  if (argc != 3) {
    cerr << "Wrong number of arguments." << endl;
    return 1;
  }

  // Save server port and cache size as global variables
  server_port = atoi(argv[1]);
  cache_size = atoi(argv[2]) * 1000000;

  // Initialize LRU Cache
  myCache.entries = new node[cache_size];
  for(int i = 0; i < cache_size; i++) {
  	myCache.freeNodes.push_back(myCache.entries+i);
  }
  myCache.head = new node;
  myCache.tail = new node;
  myCache.head->prev = NULL;
  myCache.head->next = myCache.tail;
  myCache.tail->next = NULL;
  myCache.tail->prev = myCache.head;

  // Ignore SIGPIPE signals
  signal(SIGPIPE, SIG_IGN);

  // Create the socket connection
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    cerr << "Error creating socket." << endl;
    exit(1);
  }

  // Set the fields in the socket
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);

  // Bind the server port to the socket
  if ((bind(sock, (struct sockaddr*) &server_addr, sizeof(server_addr))) < 0) {
    cerr << "Error binding socket." << endl;
    exit(1);
  }

  // Listen on the socket
  listen(sock, MAX_BACK_LOG);

  // Wait for connections
  while (true) {

    // Accept the connection
    socklen_t len = sizeof(server_addr);
    if ((client_sock = accept(sock, (struct sockaddr*) &server_addr, &len)) < 0) {
      cerr << "Error accepting socket." << endl;
      exit(1);
    }

    if ((len = recv(client_sock, request, MAX_REQUEST_LENGTH, 0)) > 0) {
      thread_params_t params;

      memset(&params, 0, sizeof(params));
      params.sock = client_sock;
      params.request_size = len;
      memset(params.request, 0, MAX_REQUEST_LENGTH);
      memcpy(params.request, request, MAX_REQUEST_LENGTH);

      // Use pthread_create to make new thread to and call requestThread
      pthread_t client_thread;

      if (pthread_create(&client_thread, NULL, handle_requests, (void*) &params) != 0) {
        cerr << "Error creating child process thread." << endl;
        // close(client_sock);
        pthread_exit(NULL);
      }
    }
  }
  return 0;
}

void* handle_requests(void* input_params) {

  // cout << "Handling request." << endl;

  int client_sock, sock;
  socklen_t request_size;
  char request[MAX_REQUEST_LENGTH], parsed_request[MAX_REQUEST_LENGTH], response[MAX_RESPONSE_LENGTH];

  memset(parsed_request, 0, MAX_REQUEST_LENGTH);
  memset(request, 0, MAX_REQUEST_LENGTH);
  memset(response, 0, MAX_RESPONSE_LENGTH);

  // Copy data from input params
  thread_params_t * params = (thread_params_t *) input_params;
  client_sock = params->sock;
  request_size = params->request_size;
  memcpy(request, params->request, MAX_REQUEST_LENGTH);
  memcpy(parsed_request, params->request, MAX_REQUEST_LENGTH);

  // cout << "======== REQUEST ========" << endl;
  // cout << request << endl;
  // cout << "======== REQUEST END ========" << endl;

  // Check if command is GET request
  char * request_type = strtok(parsed_request, " ");
  if (strcmp(request_type, "GET") != 0) {
    cout << "Not GET command." << endl;
    return NULL;
  }

  // Parse request to get PATH and HOST
  char * request_path = strtok(NULL, " ");
  strtok(NULL, "\n");
  strtok(NULL, " ");
  char * request_host = strtok(NULL, "\r");

  struct addrinfo hints, *my_addrinfo;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int status;
  if ((status = getaddrinfo(request_host, "http", &hints, &my_addrinfo) != 0)) {
    cerr << "Error resolving hostname for port." << endl;
    // cerr << gai_strerror(status) << endl;
    return NULL;
  }

  // Get destination IP address from HTTP header
  struct sockaddr_in * my_sockaddr = (struct sockaddr_in *) my_addrinfo->ai_addr;
  char dest_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(my_sockaddr->sin_addr), dest_ip, INET_ADDRSTRLEN);
  sock = my_sockaddr->sin_port;

  freeaddrinfo(my_addrinfo);


  // Check if cache contains response to send
  if (check_cache(request_path, client_sock) == -1) {
    // If not, then connect with host to get response and send
    get_host_response(dest_ip, sock, request, request_size, client_sock, request_path);
    // add_to_cache(request, response, response_size);
  }

  // if (strcmp(response, "\r\n") == 0) {
  //   char close[MAX_RESPONSE_LENGTH];
  //   sprintf(close, "Connection: close\r\n");
  //   send(client_sock, close, sizeof(close), 0);
  //   send(sock, close, sizeof(close), 0);
  // }

  cout << "Closing sockets." << endl;
  close(client_sock);
  close(sock);
  pthread_exit(NULL);

}

void get_host_response(char * addr, uint16_t port, char * request, int request_size, int client_sock, char * request_path) {

  cout << "Getting host response." << endl;

  int sock;
  struct sockaddr_in server_addr;

  // Create socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    cerr << "Error creating socket." << endl;
    exit(1);
  }

  // Set fields for socket
  server_addr.sin_addr.s_addr = inet_addr(addr);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;

  // Connect to host
  if (connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
    cerr << "Error connecting to host." << endl;
    exit(1);
  }

  // Forward the request message to host
  cout << "Request size: " << request_size << endl;
  if (send(sock, request, request_size, 0) < 0) {
    cerr << "Error sending to host." << endl;
    exit(1);
  }
  // Get the response from host and copy into response
  int bytesRecv = 0;
  int total_response_size = 0;
  char temp_response[MAX_RESPONSE_LENGTH];

  char * buf;
  int bufSize = 0;

  while (true) {
    memset(temp_response, 0, MAX_RESPONSE_LENGTH);
    bytesRecv = recv(sock, temp_response, MAX_RESPONSE_LENGTH, 0);
    cout << "bytesRecv: " << bytesRecv << endl;
    if (bytesRecv < 0) {
      cerr << "Error receiving from host" << endl;
      exit(1);
    }
	else if (bytesRecv == 0) {
		//add to cache here
		add_to_cache(request_path, buf, total_response_size);

    cout << "===== RESPONSE BEING ADDED of size " << total_response_size << endl;
    cout << buf << endl;

		free(buf);
		break;
	}
    else {
      total_response_size += bytesRecv;
      send(client_sock, temp_response, bytesRecv, 0);

      if(buf == NULL) {
      	// cout << "null";
      	bufSize += sizeof(temp_response);
      	buf = (char *) malloc (bufSize + 1);
      	strcpy(buf, temp_response);
      } else {
      	// cout << "not null";

      	bufSize += sizeof(temp_response);

      	char * bufTemp = (char *) malloc (bufSize + 1);
      	strcpy(bufTemp, buf);
      	strcpy(bufTemp + sizeof(temp_response), temp_response);
      	free(buf);
      	buf = bufTemp;
      }
      
      // Do some cache stuff here
    }
  }

  // close(sock);
}

int check_cache(char * request, int client_sock) {

  cout << "Checking cache." << endl;
  // node * n  = myCache.nodeMap.find(request)->second;
  node * n = myCache.nodeMap[request];
	if (n) {
		cout << "FOUND" << endl;

		removeNode(n);
		setHeadNode(n);
		// Send response to client
		int response_size = n->size;

    cout << "RESPONSE SIZE" << endl;
    cout << response_size << endl;

		char response[response_size];

		int sent = 0;
		memcpy(response, n->val, response_size);

    cout << "RESPONSE FROM CACHE" << endl;
    cout << response << endl;

		while (sent != response_size) {
			sent = send(client_sock, (void *) response, response_size, 0);
		}

		return 1;
	}
	else {
		cout << "NOT FOUND" << endl;
		return -1;
	}
}

void add_to_cache(char * request, char * response, int response_size) {

  // cout << "Adding to cache." << endl;

  node * n = myCache.nodeMap[request];

	// if node with key 'request' is found
	if(n) {
    // cout << "n: " << n->val << "\n";
		removeNode(n);
		n->val = response;
		n->size = response_size;
		setHeadNode(n);
	} else {
		if(myCache.freeNodes.empty()) {
			n = myCache.tail->prev;
			removeNode(n);
			myCache.nodeMap.erase(request);
			n->val = response;
			n->size = response_size;
			myCache.nodeMap[request] = n;
			setHeadNode(n);
		} else {
			n = myCache.freeNodes.back();
			myCache.freeNodes.pop_back();
			n->val = response;
			n->size = response_size;
			myCache.nodeMap[request] = n;
			setHeadNode(n);
		}
	}
}

void removeNode (node *n) {
	n->prev->next = n->next;
	n->next->prev = n->prev;
}

void setHeadNode (node *n) {
	n->next = myCache.head->next;
	n->prev = myCache.head;
	myCache.head->next = n;
	n->next->prev = n;
}
