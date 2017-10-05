#pragma once

#include <mutex>

//WHY ? Because exporting STL containers in DLLs is a bad habbit that may lead in a very far future to forced recompiles or undefined behavior
// I read this on the internet. If the info is outdated than great, next time feel free to use STL containers
/// @cond DEV

template <typename TemplateType1>
class TSLinkedList{
	struct Node {
		TemplateType1	val;
		Node			*next;
	};

public:
	TSLinkedList(){
		head = NULL; // set head to NULL
		itr = NULL;
	}
	~TSLinkedList()
	{
		clear();
	}
	void clear()
	{
//		ThreadLock.lock(); // might crash on shutdown
		Node *thead = head;
		head = NULL;
		itr = NULL;
		while (thead != NULL){
			Node *todel = thead;
			thead = thead->next;
			delete todel;
		}
//		ThreadLock.unlock();
	}
	// This prepends a new value at the beginning of the list
	void push_front(TemplateType1 val){
		Node *n = new Node();   // create new Node
		n->val = val;             // set value
		ThreadLock.lock();
		n->next = head;         // make the node point to the next node.
		//  If the list is empty, this is NULL, so the end of the list --> OK
		head = n;               // last but not least, make the head point at the new node.
		ThreadLock.unlock();
	}
	bool push_front_unique(TemplateType1 val){
		ThreadLock.lock();
		if (HasValue(val))
			return false;
		Node *n = new Node();   // create new Node
		n->val = val;             // set value
		n->next = head;         // make the node point to the next node.
		//  If the list is empty, this is NULL, so the end of the list --> OK
		head = n;               // last but not least, make the head point at the new node.
		ThreadLock.unlock();
		return true;
	}
	bool HasValue(TemplateType1 val){
		for (Node *i = head; i != NULL; i = i->next)
			if (i->val == val)
				return true;
		return false;
	}
	// returns the first element in the list and deletes the Node.
	// caution, no error-checking here!
	TemplateType1 popFirst(){
		ThreadLock.lock();
		Node *n = head;
		TemplateType1 ret = n->val;

		head = head->next;
		delete n;
		itr = NULL; //make sure we invalidate the iterator
		ThreadLock.unlock();
		return ret;
	}

	TemplateType1 begin(){
		ThreadLock.lock();
		if (head != NULL)
		{
			itr = head->next;
			TemplateType1 ret = head->val;
			ThreadLock.unlock();
			return ret;
		}
		itr = NULL;
		ThreadLock.unlock();
		return NULL;
	}

	TemplateType1 end() { return NULL; }	//only valid for pointers !!

	bool IsItrEnd(){ return itr == NULL; }

	TemplateType1 GetNext(){
		ThreadLock.lock();
		if (itr != NULL)
		{
			TemplateType1 ret = itr->val;
			itr = itr->next;
			return ret;
		}
		ThreadLock.unlock();
		return NULL;
	}
	bool empty() { return head == NULL; }
private:
	Node		*head; // this is the private member variable. It is just a pointer to the first Node
	Node		*itr;
	std::mutex  ThreadLock;	// thread safe push/pop linked list
};
/// @endcond