#include "cstack.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
	

#define UNUSED(VAR) (void)(VAR)


typedef struct stackElem {
    struct stackElem * prevElem;   // this element was pushed before this one 
    void * data;            // pointer to user data
    uint64_t size;          // user data size in bytes
} stackElem;

typedef struct myStack {
    stackElem * top;        //highest element pointer
    uint64_t stackSize;     //this is amount of elements in this stack
} myStack;

typedef struct stackListElem {
    hstack_t id;            //id of contained stack
    myStack *thisStack;      //pointer to contained stack
    struct stackListElem *nextElem;//pointer to other stack container
} stackListElem;

typedef struct stackList {
    uint64_t numStacks;     //number of created stacks
    stackListElem *lastStack;//last created stack (with highest id)
} stackList;

static stackList stackControl = {0, NULL}; 

hstack_t stack_new(void)
{
    /*
    creation steps
    1. check if this the first stack
        1.1 if so 
    0. allocate memory
        0.1 check for NULL
    3. return result
    */
    static hstack_t nextId = 0;
    myStack *newStack = (myStack *)malloc(sizeof(myStack));
    if (newStack != NULL) {
        newStack->stackSize = 0;
        newStack->top = NULL;

        stackListElem *newStackBox = (stackListElem *)malloc(sizeof(stackListElem));
        if (newStackBox) {
            newStackBox->id = nextId;
            newStackBox->thisStack = newStack;
            newStackBox->nextElem = stackControl.lastStack;

            stackControl.lastStack = newStackBox;
            stackControl.numStacks++;

            return nextId++;  //all good
        } else { //container memory allocation fail
            free(newStack);
        }
    } //stack memory allocation fail
    return -1;
}

void stack_free(const hstack_t hstack)
{
    /*
    search in stack list than free found stack structure
    */
    // if (stack_valid_handler(hstack)) {
    //     myStack * doomedStack = 
    // }
    if (hstack < 0) return;
    stackElem *tmpElem;
    stackListElem *tmp = stackControl.lastStack;
    stackListElem *doomed = NULL;
    if (tmp->id == hstack) { //finding and bypassing doomed stack
        stackControl.lastStack = stackControl.lastStack->nextElem;
        stackControl.numStacks--;
        doomed = tmp;
    } else {
        for (unsigned int i = 0; i < stackControl.numStacks; ++i) {
            if (tmp->nextElem->id == hstack) {
                doomed = tmp;
                break;
            }
            if (tmp->nextElem) tmp = tmp->nextElem;
        }
    }

    if (doomed) {
        for (unsigned int i=0; i < doomed->thisStack->stackSize; ++i) {
            if (doomed->thisStack->top->prevElem) 
                tmpElem = doomed->thisStack->top->prevElem;
            free(doomed->thisStack->top->data);
            free(doomed->thisStack->top);
            doomed->thisStack->top = tmpElem;
        }
        free(doomed->thisStack);
        free(doomed);
    }

    // UNUSED(hstack);
}

int stack_valid_handler(const hstack_t hstack)
{
    /*
    search in stack list if found - return 1 else -1
    */
   if (hstack < 0) return -1;
    stackListElem *tmp = stackControl.lastStack;
    for (unsigned int i = 0; i < stackControl.numStacks; ++i) { //makes sense to check only amount of existing stacks
        if (tmp->id == hstack) return 1;
        if (tmp->nextElem) tmp = tmp->nextElem; //just in case smth went wrong
    }
    // UNUSED(hstack);
    return -1;
}

unsigned int stack_size(const hstack_t hstack)
{
    /*
    search in stack list 
    */
    stackListElem *tmp = stackControl.lastStack;
    for (unsigned int i = 0; i < stackControl.numStacks; ++i) { //makes sense to check only amount of existing stacks
        if (tmp->id == hstack) return tmp->thisStack->stackSize;
        if (tmp->nextElem != NULL) tmp = tmp->nextElem; //just in case smth went wrong
    }
//    UNUSED(hstack);
    return 0; //is it ok to say that non-existing stack has 0 size?
}

void stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if ((data_in == NULL) || (size == 0)) return;
    if (stack_valid_handler(hstack) == 1) {
        void *tmp_data = malloc(size);
        stackListElem *tmp = stackControl.lastStack;
        for (unsigned int i = 0; i < stackControl.numStacks; ++i) { //makes sense to check only amount of existing stacks
            if (tmp->id == hstack) break;
            if (tmp->nextElem != NULL) tmp = tmp->nextElem; //just in case smth went wrong
        }
        stackElem *new_element = (stackElem *)malloc(sizeof(stackElem));
        new_element->data = tmp_data;
        memcpy(new_element->data, data_in, size);
        new_element->size = size;
        new_element->prevElem = tmp->thisStack->top;
        tmp->thisStack->top = new_element;
        
    }
    // UNUSED(hstack);
    // UNUSED(data_in);
    // UNUSED(size);
}

unsigned int stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    unsigned int res = 0;
    if (data_out == NULL) return 0;
    if (stack_valid_handler(hstack) == 1) {
        stackListElem *tmp = stackControl.lastStack;
        for (unsigned int i = 0; i < stackControl.numStacks; ++i) { //makes sense to check only amount of existing stacks
            if (tmp->id == hstack) break;
            if (tmp->nextElem) tmp = tmp->nextElem; //just in case smth went wrong
        }
        memcpy(tmp->thisStack->top->data, data_out, tmp->thisStack->top->size);
        stackElem *doomed = tmp->thisStack->top;
        tmp->thisStack->stackSize--;
        tmp->thisStack->top = tmp->thisStack->top->prevElem;
        res = doomed->size;
        free(doomed->data);
        free(doomed);
        return res;
    }
    
    // UNUSED(hstack);
    // UNUSED(data_out);
    UNUSED(size);
    return res;
}


