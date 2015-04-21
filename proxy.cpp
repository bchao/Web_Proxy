#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include <pthread.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <iterator> 

using namespace std;

/* Structs */
struct node {
	char *val;
	int size;
	node *next;
	node *prev;
};

struct LRUCache {
	map<char *, node*> nodeMap;
	vector<node*> freeNodes;
	node * head;
	node * tail;
	node * entries;
};

/* Global Variables */
const int MAX_BACK_LOG = 5;
int server_port;
int cache_size;
LRUCache myCache;

/* Function Prototypes */

void* request_thread(void* client_sock_value);
void handle_browser_requests(int client_sock);
char * get_host_response(char * request);
char * get_message(int sock, bool host);
void send_message(int sock, char * message);
char * check_cache(char * request);
void add_to_cache(char * request, char * response);

int main (int argc, char* argv[])
{

  int sock, client_sock;
  struct sockaddr_in server_addr, client_addr;

  // Check to see if correct number of inputs
  if (argc != 2) {
    cerr << "Wrong number of arguments." << endl;
    return -1;
  }

  // Save server port and cache size as global variables
  server_port = atoi(argv[1]);
  cache_size = atoi(argv[2]);

  // Create the socket connection
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    cerr << "Error creating socket." << endl;
    exit(-1);
  }

  // Set the fields in the socket
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);

  // Bind the server port to the socket
  if (bind(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
    cerr << "Error binding socket." << endl;
    exit(-1);
  }

  // Listen on the socket
  listen(sock, MAX_BACK_LOG);

  // Wait for connections
  while (true) {

    // Accept the connection
    socklen_t len = sizeof(client_addr);
    if ((client_sock = accept(sock, (struct sockaddr*) &client_addr, &len)) < 0) {
      cerr << "Error accepting socket." << endl;
      continue;
    }

    // Use pthread_create to make new thread to and call requestThread
    pthread_t client_thread;

    if (pthread_create(&client_thread, NULL, request_thread, (void*) &client_sock) != 0) {
      cerr << "Error creating child process thread." << endl;
      close(client_sock);
      pthread_exit(NULL);
    }

  }

  return 0;

}

void* request_thread(void* client_sock_value) {

  int client_sock = *((int*) client_sock_value);

  // Do I need to detach the thread here???

  handle_browser_requests(client_sock);

  close(client_sock);

  pthread_exit(NULL);

}

void handle_browser_requests(int client_sock) {

  char * request, response;

  // Get request from browser
  request = get_message(client_sock, false);
  cout << "===== REQUEST =====" << endl;
  cout << request << endl;

  // Check if cache contains request, if so grab corresponding response
  response = check_cache(request);

  // If not, then connect with host to get response
  if (response.empty()) {
    response = get_host_response(request);
  }

  cout << "===== RESPONSE =====" << endl;
  cout << request << endl;

  // Send the message response back to the client
  send_message(client_sock, response);

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

char * get_message(int sock, bool host) {

  return "";

}

void send_message(int sock, char * message) {

}

char * check_cache(char * request) {

  return NULL;

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
			newNode->val = val;
			myCache.nodeMap[request] = newNode;
			setHeadNode(newNode);
		} else {
			newNode = myCache.freeNodes.back();
			myCache.freeNodes.pop_back();
			newNode->val = val;
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

char * get(char * k) {
	node * n = myCache.nodeMap[k];

	if(n) {
		removeNode(n);
		setHeadNode(n);
		return n->val;
	} else {
		return NULL;
	}
}