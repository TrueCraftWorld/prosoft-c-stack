#include "cstack.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

	

#define UNUSED(VAR) (void)(VAR)

/*
So it's linked lists of stacks(which are just not fully functional linked lists themselves)
in theory only memory and max uint64_t limits amount of stack and their deep
*/

typedef struct stackElem {    //1 stack frame 
    struct stackElem * prevElem;    // this element was pushed before this one 
    void * data;                    // pointer to user data
    uint64_t size;                  // user data size in bytes 
} stackElem;

typedef struct myStack {    //1 stack 
    stackElem * top;        //highest element pointer
    uint64_t stackSize;     //this is amount of elements in this stack
} myStack;

typedef struct stackListElem {      //stack container
    hstack_t id;                    //id of contained stack
    myStack *thisStack;             //pointer to contained stack top
    struct stackListElem *nextElem; //pointer to next stack container
} stackListElem;

typedef struct stackList {   //linked list of created stacks (id's are sorted highest being in the top)
    uint64_t numStacks;      //number of created stacks
    stackListElem *lastStack;//last created stack (with highest id)
} stackList;

static stackList stackControl = {0, NULL}; //global entry point



/*
@brief - function to get stack pointe
@param - hstack_t hstack, id of wanted stack
@retval - stackListElem * pointer to ctack container, returns NUL if no such stack
@details - also can be used to check stack id vor validity
*/
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
    if (stackControl.numStacks == 0) nextId = 0; //this is for the test to pass witout it id's would not repeat themselves in 1 program run
    myStack *newStack = (myStack *)malloc(sizeof(myStack)); //memory for stack
    if (newStack != NULL) {
        newStack->stackSize = 0;
        newStack->top = NULL;

        stackListElem *newStackBox = (stackListElem *)malloc(sizeof(stackListElem)); //memory for container
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
    if (stack_valid_handler(hstack)) return; //nothing to be freed
    
    stackListElem *tmp = stackControl.lastStack;
    stackListElem *doomed = NULL;
    if (tmp == NULL) return;

    //finding and bypassing doomed stack
    if (tmp->id == hstack) { //special case - removing nearest stcak
        stackControl.lastStack = stackControl.lastStack->nextElem;
        doomed = tmp;
    } else { //other cases
        if (tmp->nextElem == NULL) return;
        for (unsigned int i = 0; i < stackControl.numStacks-1; ++i) {
            if (tmp->nextElem->id == hstack) {
                
                doomed = tmp->nextElem;
                if (tmp->nextElem) tmp->nextElem = tmp->nextElem->nextElem;
                break;
            }
            if (tmp->nextElem) tmp = tmp->nextElem;
        }
    }

    if (doomed) { //freeing all memory used in doomed stack
        stackElem *tmpElem = NULL;
        for (unsigned int i=0; i < doomed->thisStack->stackSize; ++i) { //every frame with user data
            if (doomed->thisStack->top->prevElem) 
                tmpElem = doomed->thisStack->top->prevElem;
            free(doomed->thisStack->top->data); 
            free(doomed->thisStack->top); 
            doomed->thisStack->top = tmpElem;
        }
        free(doomed->thisStack); //then stack
        free(doomed);               //and container
        stackControl.numStacks--; //if all good we now have less stacks
    }
}

int stack_valid_handler(const hstack_t hstack)
{
    if (getStackP(hstack) != NULL) {  //if stack exists return 0 
        return 0;
    } else {
        return 1;
    }
}

unsigned int stack_size(const hstack_t hstack)
{
    stackListElem *tmp = getStackP(hstack);
    if (tmp == NULL) return 0; //all invalid stacks have 0 size
    else return tmp->thisStack->stackSize; //if stack is valid it's safe to return actual size (can be still 0)
}

void stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if ((data_in == NULL) || (size == 0)) return;  //in case user did something not expected
    stackListElem *tmp = getStackP(hstack);         
    if (tmp == NULL) return;                        //one cannot push to non-existing stack;

                      
    stackElem *new_element = (stackElem *)malloc(sizeof(stackElem)); //memory for stack frame
    new_element->data = malloc(size);                                   //memory for data                                 
    memcpy(new_element->data, data_in, size);                          //filling stack frame with data
    new_element->size = size;
    new_element->prevElem = tmp->thisStack->top;
    tmp->thisStack->top = new_element;
    tmp->thisStack->stackSize++; //stack is bigger now
}

unsigned int stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    unsigned int res = 0;
    if (data_out == NULL) return 0;
    if (data_out == NULL) return 0;

    stackListElem *tmp = getStackP(hstack);
    if ((tmp == NULL) || 
        (tmp->thisStack->stackSize == 0) || 
        (tmp->thisStack->top->size != size)) return res; //in case of incorrect params - quit before we mess up memory

    stackElem *doomed = tmp->thisStack->top;
    memcpy(data_out, tmp->thisStack->top->data,  tmp->thisStack->top->size); //return stored dta
    
    tmp->thisStack->stackSize--;
    tmp->thisStack->top = tmp->thisStack->top->prevElem;
    res = doomed->size;
    
    free(doomed->data);
    free(doomed);

    //in the end we have less stack and more memory
    return res;
}


