#include <map>
#include <cstdlib>
#include <cmath>
using namespace std;

struct node
{
	int val;
    int key;
    node *pre;
    node *next;
};

class LRUCache {
public:
	LRUCache(int cap) {
		capacity = cap;
		len = 0;
	}

	int get(int key) {
		if(map.count(key) > 0) {
			node latest = map.find(key)->second;
			removeNode(latest);
			setHead(latest);
			return latest.val;
		} else {
			return -1;
		}
	}

	void removeNode(node n) {

	}

	void setHead(node n) {

	}

	void set(int key, int value) {
		if(map.count(key) > 0) {

		} else {

		}
	}

private:
	std::map <int, node> map;
	node head;
	node end;
	int capacity;
	int len;
};