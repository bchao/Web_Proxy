#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include <netdb.h>
#include <pthread.h>
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
} thread_params_t;

/* Function Prototypes */

void* handle_requests(void* input_params);
void get_host_response(char * addr, uint16_t port, char * request, char * response);
void send_message(int sock, char * message);
int check_cache(char * request, char * response);
void send_message(int sock, char * message);
void add_to_cache(char * request, char * response);
void removeNode (node *n);
void setHeadNode (node *n);

int main (int argc, char* argv[])
{

  int sock, client_sock;
  struct sockaddr_in server_addr;
  char request[MAX_REQUEST_LENGTH];

  // Check to see if correct number of inputs
  if (argc != 3) {
    cerr << "Wrong number of arguments." << endl;
    return 1;
  }

  // Save server port and cache size as global variables
  server_port = atoi(argv[1]);
  cache_size = atoi(argv[2]);

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
      memset(params.request, 0, MAX_REQUEST_LENGTH);
      memcpy(params.request, request, MAX_REQUEST_LENGTH);

      // Use pthread_create to make new thread to and call requestThread
      pthread_t client_thread;

      if (pthread_create(&client_thread, NULL, handle_requests, (void*) &params) != 0) {
        cerr << "Error creating child process thread." << endl;
        close(client_sock);
        pthread_exit(NULL);
      }
    }
  }
  return 0;
}

void* handle_requests(void* input_params) {

  cout << "Handling request." << endl;

  int client_sock;
  char request[MAX_REQUEST_LENGTH], parsed_request[MAX_REQUEST_LENGTH], response[MAX_RESPONSE_LENGTH];

  memset(parsed_request, 0, MAX_REQUEST_LENGTH);
  memset(request, 0, MAX_REQUEST_LENGTH);
  memset(response, 0, MAX_RESPONSE_LENGTH);

  // Copy data from input params
  thread_params_t * params = (thread_params_t *) input_params;
  client_sock = params->sock;
  memcpy(request, params->request, MAX_REQUEST_LENGTH);
  memcpy(parsed_request, params->request, MAX_REQUEST_LENGTH);

  cout << "======== REQUEST ========" << endl;
  cout << request << endl;
  cout << "======== REQUEST END ========" << endl;

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

  freeaddrinfo(my_addrinfo);

  // cout << "========= DEST IP =========" << endl;
  // cout << dest_ip << endl;

  // Check if cache contains request, if so grab corresponding response
  // If not, then connect with host to get response
  // if (check_cache(request_path, response) != 1) {
    get_host_response(dest_ip, my_sockaddr->sin_port, request, response);
    add_to_cache(request, response);
  // }

  cout << "======= RESPONSE =======" << endl;
  cout << response << endl;
  cout << "======= RESPONSE END =======" << endl;

  // Send the message response back to the client
  // send_message(client_sock, response);
  send(client_sock, response, MAX_RESPONSE_LENGTH, 0);

  // TODO: Check if Keep Alive connection while parsing and only close if not???
  close(client_sock);
  pthread_exit(NULL);

}

void get_host_response(char * addr, uint16_t port, char * request, char * response) {

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
  if (send(sock, request, MAX_REQUEST_LENGTH, 0) < 0) {
    cerr << "Error sending to host." << endl;
    exit(1);
  }

  // Get the response from host and copy into response
  int bytesRecv = 0;
  if ((bytesRecv = recv(sock, response, MAX_RESPONSE_LENGTH, 0)) < 0) {
    cerr << "Error receiving from host." << endl;
    exit(1);
  }

  cout << "======= PRE RESPONSE =======" << endl;
  cout << response << endl;
  cout << "======= PRE RESPONSE END =======" << endl;

  close(sock);
}

void send_message(int sock, char * message) {

}

int check_cache(char * request, char * response) {

  cout << "Checking cache." << endl;

  node * n  = myCache.nodeMap.find(request)->second;

	if(n) {
		removeNode(n);
		setHeadNode(n);
		memcpy(response, n->val, sizeof n->val);
		return 1;
	} else {
		return -1;
	}
}

void add_to_cache(char * request, char * response) {

  cout << "Adding to cache." << endl;

  node * n = myCache.nodeMap[request];

	// if node with key 'request' is found
	if(n) {
    // cout << "n: " << n->val << "\n";
		removeNode(n);
		n->val = response;
		setHeadNode(n);
	} else {
		if(myCache.freeNodes.empty()) {
			n = myCache.tail->prev;
			removeNode(n);
			myCache.nodeMap.erase(request);
			n->val = response;
			myCache.nodeMap[request] = n;
			setHeadNode(n);
		} else {
			n = myCache.freeNodes.back();
			myCache.freeNodes.pop_back();
			n->val = response;
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
