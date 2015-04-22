Proxy

Time Spent
Peter Yom (pky3) - 30 hours
Brandon Chao (bc105) - 30 hours

Implementation
We decided to implement this project with C++ instead of C in order to take advantage of various libraries and functions.

PROXY

When the browser uses the proxy for the first time and spins up its first website, it queries the cache for the appropriate response. If the request is met, the cache will return the responses straight back to the browser. If the request is not met, the proxy will request a response from the internet. The response will come back to the proxy, be added to the cache, and will return to the browser. Should the same request be made by the browser, the proxy should be able to find the request in the cache and not need to access the internet.

To send and receive responses, we found that we had to send them in a while loop and check that all of the message is sent for messages that are very large and much greater than our MAX_RESPONSE_SIZE.

One issue that we ran into that other groups on Piazza mentioned as well is that the browser page doesn't load until our Proxy program ends. However, when it does end, the web page renders completely with the full page source which we have compared to the page source without using the proxy. We tried to fix this bug for a long time but could not find a solution. However, it does not affect the rest of our project in that we are still able to use the cache, etc. 

LRU CACHE with STORAGE LIMIT 

Our LRU cache is comprised of a hashmap of char * to nodes, a list of free nodes, and a head and tail node. These nodes hold the response value, the size of the response, and pointers to the next and previous nodes. Each time a new request/response is added to the cache, we check if the node is already included in the cache. If so, we reset the node to the head. Otherwise, we check to see if there is space (to enforce storage based on the input cache size (WHICH IS IN MEGABYTES)) in the cache to add a new node. If there is not, we remove the least recently used node and add the new node to the head. When checking the cache, we query the cache hashmap for the request. If the node is found, we reset the node to the head and return the response. This way we are able to maintain a list for LRU purposes.
