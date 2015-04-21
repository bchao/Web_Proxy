#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
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
#define MAX_REQUEST_LENGTH (1024)
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
	map<char *, node*> nodeMap;
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
char * get_host_response(char * request);
void send_message(int sock, char * message);
char * check_cache(char * request);
void add_to_cache(char * request, char * response);
void removeNode (node *n);
void setHeadNode (node *n);
char * get(char * k);

int main (int argc, char* argv[])
{

  int sock, client_sock;
  struct sockaddr_in server_addr;
  char request[MAX_REQUEST_LENGTH];

  // Check to see if correct number of inputs
  if (argc != 2) {
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

  char * response;
  int client_sock;
  char request[MAX_REQUEST_LENGTH];

  memset(request, 0, MAX_REQUEST_LENGTH);

  // Copy data from input params
  thread_params_t * params = (thread_params_t *) input_params;
  client_sock = params->sock;
  memcpy(request, params->request, MAX_REQUEST_LENGTH);

  cout << "===== REQUEST =====" << endl;
  cout << request << endl;

  // Check if command is GET request
  char * tok = strtok(request, " ");
  if (strcmp(tok, "GET") != 0) {
    cout << "Not GET command." << endl;
    return NULL;
  }

  // Check if cache contains request, if so grab corresponding response
  response = check_cache(request);

  // If not, then connect with host to get response
  if (response == NULL) {
    response = get_host_response(request);
  }

  cout << "===== RESPONSE =====" << endl;
  cout << request << endl;

  // Send the message response back to the client
  send_message(client_sock, response);

  close(client_sock);
  pthread_exit(NULL);

}

char * get_host_response(char * request) {

  char * response;

  // Extract host IP address from message

  // Create socket

  // Connect to host

  // Forward the request message to host

  // Get the response from host and return

  return response;

}

void send_message(int sock, char * message) {

}

char * check_cache(char * request) {
	node * n = myCache.nodeMap[request];

	if(n) {
		removeNode(n);
		setHeadNode(n);
		return n->val;
	} else {
		return NULL;
	}
}

void add_to_cache(char * request, char * response) {
	node * newNode = myCache.nodeMap[request];

	// if node with key 'request' is found
	if(newNode) {
		removeNode(newNode);
		newNode->val = response;
		setHeadNode(newNode);
	} else {
		if(myCache.freeNodes.empty()) {
			newNode = myCache.tail->prev;
			removeNode(newNode);
			myCache.nodeMap.erase(request);
			newNode->val = response;
			myCache.nodeMap[request] = newNode;
			setHeadNode(newNode);
		} else {
			newNode = myCache.freeNodes.back();
			myCache.freeNodes.pop_back();
			newNode->val = response;
			myCache.nodeMap[request] = newNode;
			setHeadNode(newNode);
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