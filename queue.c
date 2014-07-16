#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct Node
{
    int Data;
    struct Node* next;
}*rear, *front;

void deQueue() {

    struct Node *temp, *var=rear;

    if (var==rear){
        rear = rear->next;
        free(var);
    } else {
        printf("\nQueue Empty");
    }

}

void enQueue(int value){

    struct Node *temp;
    temp=(struct Node *)malloc(sizeof(struct Node));
    temp->Data=value;

    if (front == NULL){
       front=temp;
       front->next=NULL;
       rear=front;
    } else {
       front->next=temp;
       front=temp;
       front->next=NULL;
    }
}

void clear(){
    // Implement
}

//http://www.cprogramto.com/queue-using-linked-list-in-c/