#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
struct node1{
    int data1;
    int data2;
    struct node1 * next;
};
struct queue1 {
    struct node1* head;
    struct node1* tail;
};
struct node2{
    int data;
    struct node2* next;
};
struct queue2{
    struct node2* head;
    struct node2* tail;
};
struct robotArgStruct{
    int n;
    int m;
    int k;
    int robotID;
};
struct servingTableArgStruct{
    int n;
    int m;
    int k;
    int tableID;
};
struct studentArgsStruct{
    int n;
    int m;
    int k;
    int studentID;
};
struct servingTableStatus{
    int robotID;
    int biryaniLeft;
};
int numberOfStudentsServed;
pthread_mutex_t robotUnassignedLock;
pthread_mutex_t chefQueueLock;
pthread_mutex_t tableStatusLock;
pthread_mutex_t studentQueueLock;
pthread_mutex_t studentServedLock;
int robotUnassignedBiryaniProduce[10000];
int studentStatusArray[10000];
struct queue1 *createQueue1();
struct node1 *newNode1(int data1, int data2);
void pushQueue1(struct queue1* curQueue, int data1, int data2);
struct node1 *popQueue1(struct queue1* curQueue);
struct queue2 *createQueue2();
struct node2 *newNode2(int data);
void pushQueue2(struct queue2* curQueue, int data);
struct node2 *popQueue2(struct queue2* curQueue);
struct servingTableStatus tableStatusArray[10000];
void spawnRobots(int m, int n, int k);
void spawnServingTables(int n, int m, int k);
void *robotFunction(void *robotArg);
void *servingTableFunction(void *servingTableArg);
int randomValueGenerator(int lowerLimit, int upperLimit);
void makeBiryani(int r, int w, int p, int robotID);
void biryaniReady(int robotID);
void spawnStudents(int k, int n, int m);
void *studentsFunction(void *studentsArgs);
void readyToServe(int numberOfSLots, int tableID);
int min(int a, int b);
struct queue1* biryaniCreatedQueue;
struct queue2* studentArrivalQueue;
int main(){
    biryaniCreatedQueue = createQueue1();
    studentArrivalQueue = createQueue2();
    int n, m, k;
    if(pthread_mutex_init(&robotUnassignedLock, NULL)!=0){
        printf("robotUnassignedLock mutex has failed\n");
    }
    if(pthread_mutex_init(&chefQueueLock, NULL)!=0){
        printf("chefQueueLock mutex has failed\n");
    }
    if(pthread_mutex_init(&tableStatusLock, NULL)!=0){
        printf("chefQueueLock mutex has failed\n");
    }
    if(pthread_mutex_init(&studentQueueLock, NULL)!=0){
        printf("chefQueueLock mutex has failed\n");
    }
    if(pthread_mutex_init(&studentServedLock, NULL)!=0){
        printf("chefQueueLock mutex has failed\n");
    }
    printf("input format : n, m, k\n");
    scanf("%d %d %d", &n, &m, &k);
    spawnRobots(m, n, k);
    spawnServingTables(n, m, k);
    spawnStudents(k, n, m);
    return 0;
}
void spawnStudents(int k, int n, int m){
    int randomWaitTime;
    int randomNumberPeople;
    int peopleleft = k;
    pthread_t * tid = (pthread_t *)malloc(sizeof(pthread_t)*k);
    for(int i=0;i<k;){
        randomWaitTime = randomValueGenerator(1, 5);
        randomNumberPeople = randomValueGenerator(1, peopleleft);
        sleep(randomWaitTime);
        for(int j=i;j<(i+randomNumberPeople);j++){
            struct studentArgsStruct *studentArgs = (struct studentArgsStruct *)malloc(sizeof(struct studentArgsStruct));
            studentArgs->k = k;
            studentArgs->n = n;
            studentArgs->m = m;
            studentArgs->studentID = j;
            pthread_create(&tid[j], NULL, studentsFunction, studentArgs);
        }
        i+=randomNumberPeople;
        peopleleft-=randomNumberPeople;
    }
    for(int i=0;i<k;i++){
        pthread_join(tid[i], NULL);
    }
    printf("Simulation Over\n");
    return;
}
void spawnRobots(int m, int n, int k){
    pthread_t *tid = (pthread_t *)malloc(sizeof(pthread_t)*m);
    for(int i=0;i<m;i++){
        struct robotArgStruct *robotArg = (struct robotArgStruct *)malloc(sizeof(struct robotArgStruct));
        robotArg->n=n;
        robotArg->k=k;
        robotArg->robotID=i;
        pthread_create(&tid[i], NULL, robotFunction, robotArg);
    }
    return;
}
void spawnServingTables(int n, int m, int k){
    pthread_t *tid = (pthread_t *)malloc(sizeof(pthread_t)*n);
    for(int i=0;i<n;i++){
        struct servingTableArgStruct *servingTableArg= (struct servingTableArgStruct *)malloc(sizeof(struct servingTableArgStruct));
        servingTableArg->n = n;
        servingTableArg->k = k;
        servingTableArg->m = m;
        servingTableArg->tableID = i;
        pthread_create(&tid[i], NULL, servingTableFunction, servingTableArg);
    }
    return;
}
void *studentsFunction(void *studentArgs){
    struct studentArgsStruct *studentArgsTypec = (struct studentArgsStruct *)studentArgs;
    printf("Student %d has arrived\n", studentArgsTypec->studentID);
    sleep(1);
    printf("Student %d is waiting for a slot at any serving table\n", studentArgsTypec->studentID);
    sleep(1);
    pthread_mutex_lock(&studentQueueLock);
    pushQueue2(studentArrivalQueue, studentArgsTypec->studentID);
    pthread_mutex_unlock(&studentQueueLock);
    while(1){
        pthread_mutex_lock(&studentServedLock);
        int tempfd = studentStatusArray[studentArgsTypec->studentID];
        pthread_mutex_unlock(&studentServedLock);
        if(tempfd==1){
            break;
        }
    }
    pthread_exit(NULL);
}
void *robotFunction(void *robotArg){
    struct robotArgStruct * robotArgTypec = (struct robotArgStruct *)robotArg;
    printf("Robot %d reporting for duty\n", robotArgTypec->robotID);
    int p = randomValueGenerator(25, 50);
    while(1){
        sleep(1);
        pthread_mutex_lock(&studentServedLock);
        int tempor=numberOfStudentsServed;
        pthread_mutex_unlock(&studentServedLock);
        if(tempor==robotArgTypec->k){
            break;
        }
        int r = randomValueGenerator(1, 10);
        int w = randomValueGenerator(2, 5);
        sleep(1);
        printf("Robot Chef %d is preparing %d vessels of biryani\n", robotArgTypec->robotID, r);
        makeBiryani(r, w, p, robotArgTypec->robotID);
        sleep(1);
        printf("Robot Chef %d has made %d number of vessels of biryani, and is waiting for them to be assigned to tables to resume cooking\n", robotArgTypec->robotID, r);
        biryaniReady(robotArgTypec->robotID);
    }
    pthread_exit(NULL);
}
void *servingTableFunction(void *servingTableArg){
    struct servingTableArgStruct *servingTableArgTypec = (struct servingTableArgStruct *)servingTableArg;
    printf("Serving Table %d reporting for duty\n", servingTableArgTypec->tableID);
    while(1){
        pthread_mutex_lock(&studentServedLock);
        int storexyz=numberOfStudentsServed;
        pthread_mutex_unlock(&studentServedLock);
        if(storexyz==servingTableArgTypec->k){
            break;
        }
        pthread_mutex_lock(&tableStatusLock);
        int store = tableStatusArray[servingTableArgTypec->tableID].biryaniLeft;
        pthread_mutex_unlock(&tableStatusLock);
        if(store==0){
            printf("Serving Container of Table %d is empty, waiting for refill\n", servingTableArgTypec->tableID);
            sleep(1);
            struct node1* temp;
            pthread_mutex_lock(&chefQueueLock);
            temp = popQueue1(biryaniCreatedQueue);
            pthread_mutex_unlock(&chefQueueLock);
            sleep(1);
            if(temp!=NULL){
                printf("Robot Chef %d is refilling Serving Container of Serving Table %d\n", temp->data1, servingTableArgTypec->tableID);
                pthread_mutex_lock(&tableStatusLock);
                tableStatusArray[servingTableArgTypec->tableID].biryaniLeft = temp->data2;
                tableStatusArray[servingTableArgTypec->tableID].robotID = temp->data1;
                pthread_mutex_unlock(&tableStatusLock);
                printf("Serving table %d is refilled by Robot Chef %d;Table %d is resuming serving now\n", servingTableArgTypec->tableID, temp->data1, servingTableArgTypec->tableID);
                sleep(1);
                printf("Serving Table %d entering Serving Phase\n", servingTableArgTypec->tableID); 
                sleep(1);
            }
        }
        pthread_mutex_lock(&tableStatusLock);
        int store2 = tableStatusArray[servingTableArgTypec->tableID].biryaniLeft;
        pthread_mutex_unlock(&tableStatusLock);
        if(store2>0){
            int numberOfSlots = randomValueGenerator(1, min(10, store2));
            sleep(1);
            readyToServe(numberOfSlots, servingTableArgTypec->tableID);
        }
        else{
            continue;
        }
    }
    pthread_exit(NULL);
}
void makeBiryani(int r, int w, int p, int robotID){
    for(int i=0;i<r;i++){
        pthread_mutex_lock(&chefQueueLock);
        // biryaniCreatedQueue.push(make_pair(robotID, p));
        pushQueue1(biryaniCreatedQueue, robotID, p);
        pthread_mutex_unlock(&chefQueueLock);
    }
    pthread_mutex_lock(&robotUnassignedLock);
    robotUnassignedBiryaniProduce[robotID]=r;
    pthread_mutex_unlock(&robotUnassignedLock);
    sleep(1);
    usleep(w*100000);
}
void biryaniReady(int robotID){
    while(1){
        pthread_mutex_lock(&robotUnassignedLock);
        int temp = robotUnassignedBiryaniProduce[robotID];
        pthread_mutex_unlock(&robotUnassignedLock);
        if(temp==0){
            break;
        }
    }
    printf("All the vessels prepared by Robot Chef %d are emptied; resuming cooking now\n", robotID);
    sleep(1);
    return;
}
void readyToServe(int numberOfSlots, int tableID){
    printf("Serving Table %d is ready to serve with %d number of slots\n", tableID, numberOfSlots);
    sleep(1);
    int count = 0;
    int *slotArray = (int *)malloc(sizeof(int)*numberOfSlots);
    for(int i=0;i<numberOfSlots;i++){
        slotArray[i]=-1;
    } 
    while(1){
        // pthread_mutex_lock(&studentQueueLock);
        // struct node2* temp = studentArrivalQueue->head;
        // pthread_mutex_unlock(&studentQueueLock);
        while(1){
            pthread_mutex_lock(&studentQueueLock);
            struct node2* temp2 = studentArrivalQueue->head;
            pthread_mutex_unlock(&studentQueueLock);
            if(temp2!=NULL){
                break;
            }    
        }
        // if(temp==NULL){
        //     break;
        // }
        sleep(1);
        pthread_mutex_lock(&tableStatusLock);
        int store=tableStatusArray[tableID].biryaniLeft;
        pthread_mutex_unlock(&tableStatusLock);
        if(store==0){
            break;
        }
        if(count==numberOfSlots){
            break;
        }
        sleep(1);
        pthread_mutex_lock(&studentQueueLock);
        int tempo= popQueue2(studentArrivalQueue)->data;
        pthread_mutex_unlock(&studentQueueLock);
        slotArray[count]=tempo;
        count++;
        sleep(1);
        printf("Student %d is alloted a slot on table %d and is waiting to be served\n", tempo, tableID);
        sleep(2);
        pthread_mutex_lock(&studentServedLock);
        numberOfStudentsServed++;
        studentStatusArray[tempo]=1;
        printf("Student %d on table %d has been served\n", tempo, tableID);
        pthread_mutex_unlock(&studentServedLock);
        pthread_mutex_lock(&tableStatusLock);
        tableStatusArray[tableID].biryaniLeft--;
        pthread_mutex_unlock(&tableStatusLock);
    }
}
int min(int a, int b){
    if(a>b){
        return b;
    }
    else{
        return a;
    }
}
int randomValueGenerator(int lowerLimit, int upperLimit){
    srand(time(0));
    return lowerLimit + (rand()%(upperLimit-lowerLimit+1));
}
struct queue1* createQueue1(){
    struct queue1* temp = (struct queue1*)malloc(sizeof(struct queue1));
    temp->head=NULL;
    temp->tail=NULL;
    return temp;
}
struct node1* newNode1(int data1, int data2){
    struct node1 * temp = (struct node1*)malloc(sizeof(struct node1));
    temp->data1=data1;
    temp->data2=data2;
    temp->next=NULL;
    return temp;
}
void pushQueue1(struct queue1* curQueue, int data1, int data2){
    struct node1* temp = newNode1(data1, data2);
    if(curQueue->tail==NULL){
        curQueue->head = temp;
        curQueue->tail = temp;
        return;
    }
    curQueue->tail->next=temp;
    curQueue->tail=temp;
    return;
}
struct node1* popQueue1(struct queue1* curQueue){
    if(curQueue->head==NULL){
        return NULL;
    }
    struct node1* temp = curQueue->head;
    curQueue->head=curQueue->head->next;
    if(curQueue->head==NULL){
        curQueue->tail=NULL;
    }
    return temp;
}
struct queue2* createQueue2(){
    struct queue2* temp = (struct queue2*)malloc(sizeof(struct queue2));
    temp->head=NULL;
    temp->tail=NULL;
    return temp;
}
struct node2* newNode2(int data){
    struct node2 * temp = (struct node2*)malloc(sizeof(struct node2));
    temp->data = data;
    temp->next=NULL;
    return temp;
}
void pushQueue2(struct queue2* curQueue, int data){
    struct node2* temp = newNode2(data);
    if(curQueue->tail==NULL){
        curQueue->head = temp;
        curQueue->tail = temp;
        return;
    }
    curQueue->tail->next=temp;
    curQueue->tail=temp;
    return;
}
struct node2* popQueue2(struct queue2* curQueue){
    if(curQueue->head==NULL){
        return NULL;
    }
    struct node2* temp = curQueue->head;
    curQueue->head=curQueue->head->next;
    if(curQueue->head==NULL){
        curQueue->tail=NULL;
    }
    return temp;
}