
#ifndef STACK_H
#define STACK_H

#include <iostream>
using namespace std;

// this is the node class used to build up the LIFO stack
template <class Data>
class Node {
    
private:
    
    Data holdMe;
    Node *next;
    
public:
    
    Node(Data x){
        holdMe = x;
        next = NULL;
    }
    
    Node(Data x, Node* nextNode){
        holdMe = x;
        next = nextNode;
    }

    Node* getNode(){
        return next;
    }
    
    Data getData(){
        return holdMe;
    }
    /*****************************************/
    /** WHATEVER CODE YOU NEED TO ADD HERE!! */
    /*****************************************/
    
};

// a simple LIFO stack
template <class Data>
class Stack {
    
    Node <Data> *head;
    
public:
    
    // destroys the stack
    ~Stack () { /* your code here */
        if(head != NULL){
            while(head){
                Node<Data>* nextNode = head->getNode();
                delete(head);
                head = nextNode;
            }
        }
    }
    
    // creates an empty stack
    Stack () { /* your code here */
        head = NULL;
    }
    
    // adds pushMe to the top of the stack
    void push (Data x) { /* your code here */
        Node<Data>* newHead = new Node<Data>(x, head);
        head = newHead;
    }
    
    // return true if there are not any items in the stack
    bool isEmpty () { /* replace with your code */
        return head == NULL;
    }
    
    // pops the item on the top of the stack off, returning it...
    // if the stack is empty, the behavior is undefined
    Data pop () { /* replace with your code */
        if(head == NULL){
            return Data();
        }
        
        Node<Data>* newHead = head->getNode();
        Data ret = head->getData();
        delete(head);
        head = newHead;
        return ret;
    }
};

#endif

