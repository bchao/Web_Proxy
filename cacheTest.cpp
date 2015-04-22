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
#define MAX_REQUEST_LENGTH (1024)
#define MAX_RESPONSE_LENGTH (8192)
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
void get_host_response(char * request, char * response);
void send_message(int sock, char * message);
int check_cache(char * request, char * response);
void send_message(int sock, char * message);
void add_to_cache(char * request, char * response);
void removeNode (node *n);
void setHeadNode (node *n);

int main (int argc, char* argv[])
{
  cache_size = atoi(argv[1]);

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

  char req1[] = "first";
  char res1[] = "hello";
  add_to_cache(req1, res1);

  char req2[] = "second";
  char res2[] = "hola";
  add_to_cache(req2, res2);

  char req3[] = "third";
  char res3[] = "howdy";
  add_to_cache(req3, res3);

  char req4[] = "fourth";
  char res4[] = "hey";
  add_to_cache(req4, res4);

  char request[MAX_REQUEST_LENGTH];

  memcpy(request, argv[2], sizeof argv[2]);

  cout << "request: " << request << "\n";

  char response[MAX_RESPONSE_LENGTH];
  int found = check_cache(request, response);

  cout << "resp:  " << response << "\n";

  return 0;
}

int check_cache(char * request, char * response) {
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