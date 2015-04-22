Proxy

Time Spent
Peter Yom (pky3) - 20 hours
Brandon Chao (bc105) - 20 hours

Implementation
We decided to attempt this project with C++ instead of C in order to take advantage of various libraries and functions.

When the browser uses the proxy for the first time and spins up its first website, it queries the cache for the appropriate response. If the request is met, the cache will return the responses straight back to the browser. If the request is not met, the proxy will request a response from the internet. The response will come back to the proxy, be added to the cache, and will return to the browser. Should the same request be made by the browser, the proxy should be able to find the request in the cache and not need to access the internet.

Our LRU cache is comprised of a hashmap of char * to nodes, a list of free nodes, and a head and tail node. These nodes hold the response value, the size of the response, and pointers to the next and previous nodes. Each time a new request/response is added to the cache, we check if the node is already included in the cache. If so, we reset the node to the head. Otherwise, we check to see if there is space in the cache to add a new node. If there is not, we remove the least recently used node and add the new node to the head. When checking the cache, we query the cache hashmap for the request. If the node is found, we reset the node to the head and return the response.