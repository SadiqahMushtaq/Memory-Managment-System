#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <time.h>


#define MEMSIZE 500
#define MAX_COMMAND_LENGTH 1024
#define MAX_NAME 5
#define MAX_FRAMES 5

/*

Create a frame
syntax: CF functionname functionaddressx
This command should create a frame on stack. The functionname is a maximum of 8 characters.
The functionaddress is an assumed address of the function in program code.
If there’s not enough space on stack, it should output an error saying “stack overflow, not enough
memory available for new function”. If all the maximum number of frames have reached, it should
output an error saying “cannot create another frame, maximum number of frames have reached”.
If a function with the given name already exists, it should give an error “function already exists”.In
case of no errors, it should create a frame on stack and create an entry in framestatus_array.
4.2 Delete a Function
syntax: DF
This command deletes the function on top of the stack.
If no function exists on stack, it should output an error message saying “stack is empty”.
4.3 Create integer local variable
syntax: CI integername integervalue
This command creates an integer of size 4 bytes on the current frame. If the frame is full, it should
output an error message saying “the frame is full, cannot create more data on it”.
4.4 Create double local variable
syntax: CD doublename doublevalue
This command creates a double of size of 8 bytes on the current frame. If the frame is full, it should
output an error message saying “the frame is full, cannot create more data on it”.
4.5 Create character local variable
syntax: CC charactername charactervalue
This command creates a character of size of 1 byte on the current frame. If the frame is full, it should
output an error message saying “the frame is full, cannot create more data on it”.
4.6 Create character buffer on heap
syntax: CH buffername size
This command allocates a buffer of bytes size plus 4 bytes on heap. It also creates a local pointer on stack
and stores the starting address of the allocated region. The buffer is filled with random characters. If the
heap is full, it should output an error message saying “the heap is full, cannot create more data”.
4.7 Deallocate a buffer on heapsyntax: DH buffername
This command de-allocates a buffer on stack. A total of buffer size plus 4 bytes are deallocated.
The data in the deallocated region is replaced with zeros. If the buffer was already de-allocated or the
pointer is invalid, output an error message saying “the pointer is NULL or already de-allocated”.
4.8 Show memory image
syntax: SM
This command should output the stack and heap snapshots.
*/

#pragma pack(1)
struct framestatus {        // for stack
    int number;             // frame number
    char name[8];           // function name representing the frame
    int functionaddress;    // address of function in the code section (will be randomly generated in this case)
    int frameaddress;       // starting address of the frame belonging to this header in Stack
    char used;              // a boolean value indicating whether the frame status entry is in use or not
};
#pragma pack()


struct freelist {            // for heap
    int start;               // start address of free region
    int size;                // size of free region
    struct freelist* next;   // pointer to the next free region
};

struct allocated {
    char name[8];
    int startaddress;
    struct allocated* next;
};

/*
Helper functions
*/

void append_allocated(struct allocated** head, char* newName, int newAddress) {
    struct allocated* newallocated = (struct allocated*)malloc(sizeof(struct allocated));
    if (newallocated == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    newallocated->startaddress = newAddress;
    strcpy(newallocated->name, newName);
    newallocated->next = NULL;

    if (*head == NULL) {
        *head = newallocated;
        return;
    }

    struct allocated* lastallocated = *head;
    while (lastallocated->next != NULL) {
        lastallocated = lastallocated->next;
    }

    lastallocated->next = newallocated;
    // free(newallocated);
}

void printAllocated(struct allocated* head_allocated) {
    // Check if the allocated list is empty
    if (head_allocated == NULL) {
        printf("Empty\n");
        return;
    }

    // Print each node in the allocated list
    while (head_allocated != NULL) {
        printf("Start: %d, Name: %s -> ", head_allocated->startaddress, head_allocated->name);
        head_allocated = head_allocated->next;
    }
    printf("NULL\n");
}

void printFreelist(struct freelist* head) {
    // Check if the freelist is empty
    if (head == NULL) {
        printf("Empty\n");
        return;
    }

    // Print each node in the freelist
    while (head != NULL) {
        printf("Size: %d, Start: %d -> ", head->size, head->start);
        head = head->next;
    }
    printf("NULL\n");
}

// fuction to free the allocated list
void free_allocated_list(struct allocated* head_allocated) {
    struct allocated* current = head_allocated;
    struct allocated* next;

    while (current != NULL) {
        next = current->next;
        current = next;
    }
}
/*
Required Functions
*/

// create a new frame on the stack
void create_frame(char *name, int address, struct framestatus *framestatus_array, int *top, char *memory, int *frame_head){                     
    printf("create frame function called\n");

    //=================================================================================================

    // check if the stack is full
    if (*top == MAX_FRAMES - 1) {
        printf("cannot create another frame, maximum number of frames have reached\n");
        return;
    }

    //check if the frame with the same name already exists
    for (int i = 0; i <= *top; i++) {
        if (strcmp(framestatus_array[i].name, name) == 0) {
            printf("frame already exists\n");
            return;
        }
    }

    // check if there is enough memory available for new frame
    if (105 - sizeof(struct framestatus) * (*top + 1) < sizeof(struct framestatus)) {
        printf("stack overflow, not enough memory available for new frame\n");
        return;
    }

    // create a new frame
    struct framestatus fs;
    fs.frameaddress = (long long int)*frame_head;
    fs.used = '1';
    fs.number = *top + 1;
    fs.functionaddress = address;
    strcpy(fs.name, name);
    // fs.size = 0;
    (*top)++;
    printf("top: %d\n", *top);
    framestatus_array[*top] = fs;

    //=================================================================================================
    // update the memory buffer. Remove comment to view with sprintf
    // sprintf(&memory[(*frame_head)], "%d", fs.number);
    // memcpy(&memory[(*frame_head)], &fs.number, sizeof(int));
    // sprintf(&memory[(*frame_head) + 4], "%s", fs.name);
    // memcpy(&memory[(*frame_head) + 4], fs.name, sizeof(char) * 8);
    // sprintf(&memory[(*frame_head) + 12], "%d", fs.functionaddress);
    // memcpy(&memory[(*frame_head) + 12], &fs.functionaddress, sizeof(int));
    // sprintf(&memory[(*frame_head) + 16], "%d", fs.frameaddress);
    // memcpy(&memory[(*frame_head) + 16], &fs.frameaddress, sizeof(int));
    // sprintf(&memory[(*frame_head) + 17], "%c", fs.used);
    // memcpy(&memory[(*frame_head) + 17], &fs.used, sizeof(char));
    
    memcpy(&memory[394], framestatus_array, sizeof(framestatus_array));
}

// delete the current frame from the stack
void delete_frame(struct framestatus *framestatus_array, char *memory, int *frame_head, int *top){                                            
    printf("delete frame function called\n");
    if(top <0){
        printf("stack is empty\n");
        return;
    }
    printf("Head before deletion: %d\n", *frame_head);
    *frame_head = framestatus_array[*top].frameaddress;
    framestatus_array[*top].used = '0';
    framestatus_array[*top].number = -1;
    framestatus_array[*top].functionaddress = -1;
    framestatus_array[*top].frameaddress = -1;
    strcpy(framestatus_array[*top].name, "0");
    printf("Head after deletion: %d\n", *frame_head);

    (*top)--;
    memcpy(&memory[394], framestatus_array, sizeof(framestatus_array));

} 

// create an integer variable on the current frame
void create_integer(char *name, int *value, struct framestatus *framestatus_array, int *top, char *memory, int *frame_head) {
    printf("create integer function called\n");
    printf("integer name: %s\n", name);
    printf("integer value: %d\n", *value);

    printf("top: %d\n", *top);

    // Check if the stack is empty
    if (*top == -1) {
        printf("stack is empty\n");
        return;
    }

    // Update the frame_head
    printf("frame_head: %d\n", *frame_head);
    *frame_head -= sizeof(int);
    printf("frame_head: %d\n", *frame_head);


    sprintf(&memory[(*frame_head)], "%d", *value);
    memcpy(&memory[(*frame_head)], value, sizeof(int));
    

    printf("Integer variable '%s' created at address %d with value %d\n", name, *frame_head, *value);
    printf("frame_head: %d\n", *frame_head);
    printf("\n");
    return;
}


// create a double variable on the current frame
void create_double(char *name, double *value, struct framestatus *framestatus_array, int *top, char *memory, int *frame_head){                   
    printf("create double function called\n");
    printf("double name: %s\n", name);
    printf("double value: %f\n", *value);

        // Check if the stack is empty
    if (*top == -1) {
        printf("stack is empty\n");
        return;
    }

    // Update the frame_head
    printf("frame_head: %d\n", *frame_head);
    *frame_head -= sizeof(double);
    printf("frame_head: %d\n", *frame_head);

    // Create the DOUBLE variable at the LOCTAION OF frame_head
    // sprintf(&memory[(*frame_head)], "%f", *value);
    memcpy(&memory[(*frame_head)], value, sizeof(double));
    printf("frame_head3: %d\n", *frame_head);

    

    printf("Double variable '%s' created at address %d with value %f\n", name, *frame_head, *value);
    printf("\n");
    return;
}

// create a character variable on the current frame
void create_character(char *name, char *value, struct framestatus *framestatus_array, int *top, char *memory, int *frame_head){                  
    printf("create character function called\n");
    printf("character name: %s\n", name);
    printf("character value: %c\n", *value);

        printf("top: %d\n", *top);

    // Check if the stack is empty
    if (*top == -1) {
        printf("stack is empty\n");
        return;
    }

    // Update the frame_head
    printf("frame_head: %d\n", *frame_head);
    *frame_head -= sizeof(char);
    printf("frame_head: %d\n", *frame_head);

    // Create the CHARCTER variable at the LOCTAION OF frame_head
    // sprintf(&memory[(*frame_head)], "%c", *value);
    memcpy(&memory[(*frame_head)], value, sizeof(char));
    printf("frame_head3: %d\n", *frame_head);

    printf("Character variable '%s' created at address %d with value %c\n", name, *frame_head, *value);
    return;
    printf("\n");
} 

// print the stack or heap for debugging purposes
void print(char *memory){                                                   
    printf("print function called\n");
    // prints hex values of memory
    for(int i = 0; i < MEMSIZE; i++){
        printf("%d = %x\n", i, memory[i]);
    }
    printf("\n");

    // uncomment if using sprintf
    // printf("===============================For Checking Purposes===========================\n");
    // To see decimal values of memory
    // for(int i = 0; i < MEMSIZE; i++){
    //     printf("%d = %d\n", i, memory[i]);
    // }
    // printf("\n");
    // To see characters of memory
    // for (int i = 0; i < MEMSIZE; i++) {
    //     printf("%d = %c  ", i, memory[i]);
    //     if ((i + 1) % 10 == 0) {
    //         printf("\n");
    //     }
    // }
    printf("\n");
} 

// freelist start, size, next
// allocated address, name, next
// create a buffer on the heap

// Heap cannot be created without frame
void create_buffer_heap(char* name, int* size, struct freelist** head, struct allocated** head_allocated, char* memory, int* frame_head, int *top) {
    printf("create buffer function called\n");
    printf("buffer name: %s\n", name);
    printf("buffer size: %d\n", *size);


    // Finding free node in the freelist
    struct freelist* temp = *head;
    while (temp->size < (*size) + 8) {
        temp = temp->next;
        if (temp == NULL) {
            printf("Error: Not enough free space in heap.\n");
            return;
        }
    }

    // Checking if frame is open
    if(*top == -1){
        printf("Error: No frame is open. No function created\n");
        return;
    }
    

    // Updating allocated list
    (*head_allocated)->startaddress = temp->start;
    append_allocated(head_allocated, name, (*head_allocated)->startaddress);
    printf("Allocated list: \n");
    printAllocated(*head_allocated);
    printf("\n");

    // updating frame
    int store = (*head_allocated)->startaddress;
    *frame_head -= sizeof(int);
    memcpy(&memory[*frame_head], &store, sizeof(int));

    // Updating free list
    temp->size = temp->size - (*size) - 8;
    temp->start = temp->start + (*size) + 8;
    printf("Free list: \n");
    printFreelist(*head);
    printf("\n");

    // Create the buffer
    int magic_no = rand() % 1000;
    char* str = (char*)malloc(*size);
    srand(time(NULL));
    for (int i = 0; i < *size - 1; i++) {
        str[i] = 'A' + (rand() % 26);
        printf("%c", str[i]);
    }
    printf("\n");
    
    // Update the memory buffer. Remove comment from lines if sprintf if u want to view the memory for checking
    // sprintf(&memory[store], "%d", *size);
    memcpy(&memory[store], size, sizeof(int));
    // sprintf(&memory[store + 4], "%d", magic_no);
    memcpy(&memory[store + 4], &magic_no, sizeof(int));
    // sprintf(&memory[store + 8], "%s", str);
    memcpy(&memory[store + 8], str, *size);

    free(str);  // Free dynamically allocated memory

    printf("\n");

    return;
}


// delete a buffer from the heap
void delete_buffer_heap(char *name, struct freelist **head, struct allocated **head_allocated, char *memory, int *frame_head)
{
    printf("delete buffer function called\n");
    printf("buffer name: %s\n", name);

    // Finding the buffer in the allocated list
    struct allocated *temp = *head_allocated;
    if (temp == NULL)
    {
        printf("Error: Allocated list is empty.\n");
        return;
    }
    struct allocated *temp1 = NULL;
    while (strcmp(temp->name, name) != 0)
    {
        temp1 = temp; // buffer before the buffer to be deleted in the list
        temp = temp->next; // buffer to be deleted
        if (temp == NULL)
        {
            printf("Error: Buffer to delete not found in heap\n");
            return;
        }
    }

    // Updating allocated list
    if (temp1 != NULL){
        temp1->next = temp->next;
    }
    else{
        *head_allocated = temp->next;
    }
    temp->next = NULL;
    printf("Allocated list after deallocating buffer: \n");
    printAllocated(*head_allocated);

    // Updating free list
    int size;
    memcpy(&size, &memory[temp->startaddress], sizeof(int)); // fetching size of buffer to be deleted from its metadata
    struct freelist *temp2 = (struct freelist *)malloc(sizeof(struct freelist));
    if (temp2 == NULL){
        printf("Memory allocation failed\n");
        return;
    }
    struct freelist *temp3 = *head;
    if (temp3 == NULL)
    { // if freelist was empty
        temp2->start = temp->startaddress;
        temp2->size = size + 8;
        temp2->next = NULL;
        *head = temp2;
        printf("Free list after deallocating buffer: \n");
        printFreelist(*head);
        printf("\n");
        free(temp2); // Free the dynamically allocated temp2
        return;
    }
    temp2->size = size + 8; // updating size of the first node in free list (including the 8 bytes of metadata)
    temp2->start = temp->startaddress; // updating start address of the first node in free list
    temp2->next = temp3;               // updating next of the first node in free list
    *head = temp2;                     // updating head of free list
    printf("Free list after deallocating buffer: \n");
    printFreelist(*head);
    printf("\n");

    // Updating memory
    memset(&memory[temp->startaddress], '0', size + 8); // Zero out the memory of the deleted buffer

    printf("\n");
}



int main() {
    

    /*
     START INITIALIZATION
    */
    char command[3];           // array to store the command
    char name[MAX_NAME + 1]; // variable to store the function or variable name
    int address; // variable to store the function address
    int val; // variable to store the integer value
    double dval; // variable to store the double value
    char cval; // variable to store the character value
    char memory[MEMSIZE];     // Buffer that will emulate stack and heap memory
    struct framestatus framestatus_array[MAX_FRAMES];     // array of framestatus structures (size should be 5*21 = 105)
    memcpy(&memory[394], framestatus_array, sizeof(framestatus_array));
    int top = -1;
    struct framestatus fs;
    struct freelist freelist[300];      // array of freelist structures (size should be 500*12 = 6000)
    int frame_head = 394;                   // pointer to the head of the current frame
    struct allocated allocatedlist[300];
    int currentstacksize = 200;
    int currentheapsize = 100;

    // initializing memory buffer with '0'
    for(int i = 0; i < 500; i++) {
        memory[i] = '0';
    }

    // initializing framestatus_array
    for(int i = 0; i < 5; i++) {
        framestatus_array[i].used = '0';
        framestatus_array[i].number = -1;
        framestatus_array[i].functionaddress = -1;
        framestatus_array[i].frameaddress = -1;
        strcpy(framestatus_array[i].name, "N/A");
        printf("size 0f %ld\n", sizeof(framestatus_array[i]));
    }
    memcpy(&memory[394], framestatus_array, sizeof(framestatus_array));


    // initializing freelist
    for(int i = 0; i < 300; i++) {
        freelist[i].start = -1;
        freelist[i].size = 0;
        freelist[i].next = NULL;
    }
    
    struct freelist* head;
    struct freelist freeallocated;
    head = &freeallocated;

    
    head->start = 0;
    head->size = 300;
    head->next = NULL;


    // Initializing allocated list
    for(int i = 0; i < 300; i++) {
        strcpy(allocatedlist[i].name, "N/A");
        allocatedlist[i].startaddress = -1;
        allocatedlist[i].next = NULL;
    }

    struct allocated* head_allocated = NULL;
    struct allocated freeallocated_allocated;
    head_allocated = &freeallocated_allocated;

    
    
    /*
     END INITIALIZATION
    */

    while (1) {
    printf("prompt>");
    scanf("%s", command); // read the command

    if (strcmp(command, "CF") == 0) { // create frame command
        if (scanf("%s %d", name, &address) != 2) {
            printf("Invalid parameters for CF command\n");
            continue;
        }
        else{
            create_frame(name, address, framestatus_array, &top, memory, &frame_head); // call the create frame function
        }
    } 
    else if (strcmp(command, "DF") == 0) { // delete frame command
        delete_frame(framestatus_array, memory, &frame_head, &top); // call the delete frame function
    } 
    else if (strcmp(command, "CI") == 0) { // create integer command
        if (scanf("%s %d", name, &val) != 2) {
            printf("Invalid parameters for CI command\n");
            continue;
        }
        else{
            create_integer(name, &val, framestatus_array, &top, memory, &frame_head); // call the create integer function
        }
    } 
    else if (strcmp(command, "CD") == 0) { // create double command
        if (scanf("%s %lf", name, &dval) != 2) {
            printf("Invalid parameters for CD command\n");
            continue;
        }
        else{
            create_double(name, &dval, framestatus_array, &top, memory, &frame_head); // call the create double function
        }
    } else if (strcmp(command, "CC") == 0) { // create character command
        if (scanf("%s %c", name, &cval) != 2) {
            printf("Invalid parameters for CC command\n");
            continue;
        }
        else{
            create_character(name, &cval, framestatus_array, &top, memory, &frame_head); // call the create character function
        }
    } 
    else if (strcmp(command, "CH") == 0) { // print stack command
        if (scanf("%s %d", name, &val) != 2) {
            printf("Invalid parameters for CH command\n");
            continue;
        }
        else{
            create_buffer_heap(name, &val, &head, &head_allocated, memory, &frame_head, &top); // call the create buffer heap function
        }
    } 
    else if (strcmp(command, "DH") == 0) {
        if (scanf("%s", name) != 1) {
            printf("Invalid parameters for DH command\n");
            continue;
        }
        else{
            delete_buffer_heap(name, &head, &head_allocated, memory, &frame_head); // call the delete buffer heap function
        }
    } 
    else if (strcmp(command, "SM") == 0) { // print stack command
        print(memory); // call the print stack function
    } 
    else if (strcmp(command, "exit") == 0) {
        break; // exit the program
    } 
    else {
        printf("Invalid command\n"); // print error message if the command is invalid
    }
}
    for(int i = 0; i < 5; i++){
        if(framestatus_array[i].used == '1'){
            printf("frame number: %d\n", framestatus_array[i].number);
            printf("function name: %s\n", framestatus_array[i].name);
            printf("function address: %d\n", framestatus_array[i].functionaddress);
            printf("frame address: %d\n", framestatus_array[i].frameaddress);
            printf("frame usage: %c\n", framestatus_array[i].used);
            printf("\n");
        }
        else break;
    }

    // Free the allocated blocks
    free_allocated_list(head_allocated);
    
    return 0;
}

