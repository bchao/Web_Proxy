#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include <pthread.h>
#include "LRUCache.h"

using namespace std;

/* Structs */


/* Global Variables */
const int MAX_BACK_LOG = 5;
int server_port;
int cache_size;

/* Function Prototypes */

void* request_thread();
void handle_browser_requests(int client_sock);
string get_host_response(string request);
string get_message(int sock, bool host);
void send_message(int sock, string message);


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

    if ((pthread_create(&client_thread, NULL, request_thread, (void*) &client_sock) != 0)) {
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

  handleBrowserRequests(client_sock);

  close(client_sock);

  pthread_exit(NULL);

}

void handle_browser_requests(int client_sock) {

  string request, response;

  // Get request from browser
  request = get_message(client_sock, false);

  // Check if cache contains request, if so grab corresponding response
  response = check_cache(request);

  // If not, then connect with host to get response
  if (response == "") {
    response = get_host_response(request);
  }

  // Send the message response back to the client
  send_message(client_sock, response);

}

string get_host_response(string request) {

  // Extract host IP address from message

  // Create socket

  // Connect to host

  // Forward the request message to host

  // Get the response from host and return

}

string get_message(int sock, bool host) {

}

void send_message(int sock, string message) {

}