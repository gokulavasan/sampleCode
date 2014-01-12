#include <iostream>
#include <string>
#include <sstream>
using namespace std;

struct Node {
	int value;
	Node * next;
};

Node * initNode (int val) {
	if (val <= 0) {
		return nullptr;
	}
	Node * head = nullptr;
	Node * curr = nullptr;
	for (int i = 0; i < val; i++) {
		Node * nNode = new Node();
		nNode->value = i;
		nNode->next = nullptr;
		if (curr)
			curr->next = nNode;
		else
			head = nNode;

		curr = nNode;
	}
	return head;
}

void printList(Node * n) {
	while (n != nullptr) {
		cout << n->value << " ";
		n = n->next;
	}
	cout << endl;
}

void evenOddList (Node * head) {
	if (head == nullptr || head->next == nullptr)
		return;

	Node * even = head;
	Node * odd = head->next;
	Node * oddHead = odd;
	bool eB = false, oB = false;
	while (1) {
		eB = true;
		oB = true;
		if (even->next != nullptr && even->next->next != nullptr) {
			eB = false;
			even->next = even->next->next;
			even = even->next;
		}

		if (odd->next != nullptr && odd->next->next != nullptr) {
			oB = false;
			odd->next = odd->next->next;
			odd = odd->next;
		}

		if (eB && oB)
			break;
	}
	even->next = oddHead;
	odd->next = nullptr;
}

void freeList(Node * head) {
	if (head == nullptr)
		return;
	while (head != nullptr) {
		Node * del = head->next;
		delete head;
		head = del;
	}
}

int main() {
    int numNodes;
    scanf("%d", &numNodes);
    Node * head = nullptr;
    head = initNode(numNodes);
    printList(head);
    evenOddList(head);
    printList(head);
    freeList(head);
	return 0;
}

