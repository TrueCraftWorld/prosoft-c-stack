#include "cstack.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

	

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

stackListElem *getStackP (const hstack_t hstack) {
    stackListElem *tmp = NULL; 
    if ((hstack >= 0) && (stackControl.numStacks > 0)) {
        tmp = stackControl.lastStack;
        for (unsigned int i = 0; i < stackControl.numStacks; ++i) { //makes sense to check only amount of existing stacks
            if (tmp->id == hstack) {
                return tmp;
            }
            if (tmp->nextElem) tmp = tmp->nextElem; //just in case smth went wrong
        }
    }
    return NULL; //if we return here - no such stack
}

hstack_t stack_new(void)
{
    static hstack_t nextId = 0;
    if (stackControl.numStacks == 0) nextId = 0;
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
            printf("stack made - id=%d \r\n", newStackBox->id);
            return nextId++;  //all good
        } else { //container memory allocation fail
            free(newStack);
        }
    } //stack memory allocation fail
    return -1;
}

void stack_free(const hstack_t hstack)
{
   if (stack_valid_handler(hstack)) return;
    stackElem *tmpElem = NULL;
    stackListElem *tmp = stackControl.lastStack;
    stackListElem *doomed = NULL;
    if (tmp == NULL) return;
    
    if (tmp->id == hstack) { //finding and bypassing doomed stack
        stackControl.lastStack = stackControl.lastStack->nextElem;
        doomed = tmp;
    } else {
        if (tmp->nextElem == NULL) return;
        for (unsigned int i = 0; i < stackControl.numStacks-1; ++i) {
            if (tmp->nextElem->id == hstack) {
                
                doomed = tmp->nextElem;
                if (tmp->nextElem) tmp->nextElem = tmp->nextElem->nextElem;
                // stackControl.numStacks--;
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
        printf("stack doomed - id=%d \r\n", doomed->id);
        free(doomed->thisStack);
        free(doomed);
        stackControl.numStacks--;
    }
}

int stack_valid_handler(const hstack_t hstack)
{
    if (getStackP(hstack) != NULL) {
        return 0;
    } else {
        return 1;
    }
}

unsigned int stack_size(const hstack_t hstack)
{
    if (hstack < 0) return 0;
    stackListElem *tmp = getStackP(hstack);
    if (tmp == NULL) return 0;
    else return tmp->thisStack->stackSize;
}

void stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if ((data_in == NULL) || (size == 0)) return;
    stackListElem *tmp = getStackP(hstack);
    if (tmp == NULL) return;

    void *tmp_data = malloc(size);
    stackElem *new_element = (stackElem *)malloc(sizeof(stackElem));
    new_element->data = tmp_data;
    memcpy(new_element->data, data_in, size);
    new_element->size = size;
    new_element->prevElem = tmp->thisStack->top;
    tmp->thisStack->top = new_element;
    tmp->thisStack->stackSize++;
}

unsigned int stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    unsigned int res = 0;
    if (data_out == NULL) return 0;
    if (data_out == NULL) return 0;

    stackListElem *tmp = getStackP(hstack);
    if ((tmp == NULL) || 
        (tmp->thisStack->stackSize == 0) || 
        (tmp->thisStack->top->size != size))return res;

    stackElem *doomed = tmp->thisStack->top;
    memcpy(data_out, tmp->thisStack->top->data,  tmp->thisStack->top->size);
    
    tmp->thisStack->stackSize--;
    tmp->thisStack->top = tmp->thisStack->top->prevElem;
    res = doomed->size;
    
    free(doomed->data);
    free(doomed);
    return res;
}


