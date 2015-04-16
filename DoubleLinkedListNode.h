class DoubleLinkedListNode {
public:
	DoubleLinkedListNode(int k, int v) {
		val = v;
		key = k;
	}

public:
	int val;
	int key;
	DoubleLinkedListNode *pre;
	DoubleLinkedListNode *next;
};