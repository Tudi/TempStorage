#pragma once

//WHY ? Because exporting STL containers in DLLs is a bad habbit that may lead in a very far feature to forced recompiles or undefined behavior
// I read this on the internet. If the info is outdated than great, next time feel free to use STL containers

template <typename T>
class LIBRARY_API LinkedList{
	struct Node {
		T val;
		Node *next;
	};

public:
	LinkedList(){
		head = NULL; // set head to NULL
		itr = NULL;
	}
	~LinkedList()
	{
		clear();
	}
	void clear()
	{
		Node *thead = head;
		head = NULL;
		while (thead != NULL){
			Node *todel = thead;
			thead = thead->next;
			delete todel;
		}
	}
	// This prepends a new value at the beginning of the list
	void push_front(T val){
		Node *n = new Node();   // create new Node
		n->val = val;             // set value
		n->next = head;         // make the node point to the next node.
		//  If the list is empty, this is NULL, so the end of the list --> OK
		head = n;               // last but not least, make the head point at the new node.
	}

	// returns the first element in the list and deletes the Node.
	// caution, no error-checking here!
	int popValue(){
		Node *n = head;
		int ret = n->x;

		head = head->next;
		delete n;
		return ret;
	}

	T	begin(){
		if (head != NULL)
		{
			itr = head->next;
			return head->val;
		}
		itr = NULL;
		return NULL;
	}

	T	end() { return NULL; }	//only valid for pointers !!

	bool IsItrEnd(){ return itr == NULL; }

	T	GetNext(){
		if (itr != NULL)
		{
			T ret = itr->val;
			itr = itr->next;
			return ret;
		}
		return NULL;
	}
private:
	Node *head; // this is the private member variable. It is just a pointer to the first Node
	Node *itr;
};