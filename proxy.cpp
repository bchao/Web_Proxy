#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include <pthread.h>
#include <unordered_map>
#include "LRUCache.h"

using namespace std;

/* Structs */


/* Global Variables */
int serverPort;
int cacheSize;

/* Function Prototypes */

void* requestThread();
void handleBrowserRequests(int client_sock);



int main (int argc, char* argv[])
{
  // Check to see if correct number of inputs
  if (argc != 2) {
    cerr << "Wrong number of arguments." << endl;
    return -1;
  }

  // Save server port and cache size as global variables
  serverPort = atoi(argv[1]);
  cacheSize = atoi(argv[2]);

  // Create the socket connection
  int sock;
  if (sock = socket(AF_INET, SOCK_STREAM, 0) < 0) {
    cerr << "Error creating socket." << endl;
    exit(-1);
  }

  // Set the fields in the socket
    // TODO: Look in Snowcast server

  // Bind the server port to the socket
    // TODO: Look in Snowcast server 

  // Listen on socket
    // TODO: Look in Snowcast server

  // Wait for connections
  while (true) {

    // Accept the connection

    // Use pthread_create to make new thread to and call requestThread
      // Remember to pass in the client_sock into requestThread

  }

  return 0;
}

void checkCache(string reqest) {

}

void addToCache() {

}

void* requestThread() {

  int client_sock;

  handleBrowserRequests(client_sock);

  close(client_sock);

  pthread_exit(NULL);
}

void handleBrowserRequests(int client_sock) {


  // Get request from browser

  // Check if cache contains request, if so grab corresponding response

  // If not, then connect with host to get response

  // Send the message response back to the client

}

string connectWithHost(string requestMessage) {

  // Extract host IP address from message

  // Create socket

  // Connect to host

  // Forward the request message to host

  // Get the response from host and return

}
