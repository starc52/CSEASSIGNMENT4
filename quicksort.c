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
#define _POSIX_C_SOURCE 199309L //required for clock
struct quickSortArg{
int* array;
int l;
int h;
int n;
};
void* threadedQuickAndInsertSort(void* struc);
void proQuickAndInsertSort(int *array, int l, int h, int n);
int partition(int *array, int l, int h, int n);
void normalQuickAndInsertSort(int *array, int l, int h, int n);
int partition(int *array, int l, int h, int n);
int main(){
    int n;
    scanf("%d", &n);
    int shm_id = shmget(IPC_PRIVATE, sizeof(int)*(n+1), IPC_CREAT | 0666);
    int * array = (int*)shmat(shm_id, NULL, 0);
    int * array2= (int*)malloc(sizeof(int)*n);
    int * array3= (int*)malloc(sizeof(int)*n);
    for(int i=0;i<n;i++){
        scanf("%d", &array[i]);
        array3[i]=array[i];
        array2[i]=array[i];
    }
    int l=0, h=n-1;
    struct timespec ts;
    printf("Running concurrent_quicksort for n = %d\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec/(1e9)+ts.tv_sec;
    proQuickAndInsertSort(array, l, h, n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t1 = en-st;
    struct quickSortArg arg;
    arg.h=n-1;
    arg.l=0;
    arg.n=n;
    arg.array=array2;
    printf("Running threaded_quicksort for n = %d\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;
    threadedQuickAndInsertSort(&arg);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t2 = en-st;
    l=0, h=n-1;
    printf("Running normal_quicksort for n = %d\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;
    normalQuickAndInsertSort(array3, l, h, n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t3 = en - st;
    printf("normal_quicksort ran %Lf times faster than concurrent_quicksort and %Lf times faster than threaded_quicksort and threaded_quicksort runs %Lf times faster than concurrent_quicksort\n", t1/t3, t2/t3, t1/t2;
    shmdt(array);
    return 0;
}
void normalQuickAndInsertSort(int *array, int l, int h, int n){
    if(l+3<h){
        int p=partition(array, l, h, n);
        normalQuickAndInsertSort(array, l, p-1, n);
        normalQuickAndInsertSort(array, p+1, h, n);
    }
    else if(l<h){
        int j, key;
        for(int i=l;i<=h;i++){
            key = array[i];  
            j = i - 1;  
            while (j >= 0 && array[j] > key) {  
                array[j + 1] = array[j];  
                j = j - 1;  
            }  
            array[j + 1] = key;
        }
    }
}
int partition(int *array, int l, int h, int n){
    srand(time(0));
    int randomValue = rand();
    randomValue = randomValue%(h-l+1);
    randomValue+=l;
    int temp12= array[randomValue];
    array[randomValue]=array[l];
    array[l]=temp12;
    int pivot = array[l];
    int i=l-1;
    for(int j=l+1;j<=h;j++){
        if(array[j]<pivot){
            i++;
            int temp = array[i+1];
            array[i+1]=array[j];
            array[j]=temp;
        }
    }
    int temp=array[i+1];
    array[i+1]=array[l];
    array[l]=temp;
    return i+1;
}
void *threadedQuickAndInsertSort(void *struc){
    struct quickSortArg * arg = (struct quickSortArg *) struc;
    int l = arg->l;
    int h = arg->h;
    int n = arg->n;
    int * array = arg->array;
    if(l+3<h){
        int p=partition(array, l, h, n);
        struct quickSortArg left;
        left.l=l;
        left.h=p-1;
        left.n=n;
        left.array=array;
        pthread_t tid1;
        int p1 = pthread_create(&tid1, NULL, threadedQuickAndInsertSort, &left);
        if(p1!=0){
            perror("Thread Creation");
        }
        struct quickSortArg right;
        right.l=p+1;
        right.h=h;
        right.n=n;
        right.array=array;
        pthread_t tid2;
        int p2 = pthread_create(&tid2, NULL, threadedQuickAndInsertSort, &right);
        if(p2!=0){
            perror("Thread Creation");
        }
        pthread_join(tid1, NULL);
        pthread_join(tid2, NULL);
    }
    else if(l<h){
        int j, key;
        for(int i=l;i<=h;i++){
            key = array[i];  
            j = i - 1;  
            while (j >= 0 && array[j] > key) {  
                array[j + 1] = array[j];  
                j = j - 1;  
            }  
            array[j + 1] = key;
        }
    }
    else{
        return NULL;
    }
    return NULL;
}
void proQuickAndInsertSort(int *array, int l, int h, int n){
    if(l+3<h){
        int p=partition(array, l, h, n);
        pid_t pid1 = fork();
        if(pid1<0){
            perror("Process Creation Error");
        }
        int status1;
        if(pid1==0){
            proQuickAndInsertSort(array, l, p-1, n);
            exit(1);
        }
        else{
            int status2;
            pid_t pid2 = fork();
            if(pid2<0){
                perror("Process Creation Error");
            }
            if(pid2==0){
                proQuickAndInsertSort(array, p+1, h, n);
                exit(1);
            }
            else{
                waitpid(pid1, &status1, 0);
                waitpid(pid2, &status2, 0);        
            }
        }
    }
    else if(l<h){
        int j, key;
        for(int i=l;i<=h;i++){
            key = array[i];  
            j = i - 1;  
            while (j >= 0 && array[j] > key) {  
                array[j + 1] = array[j];  
                j = j - 1;  
            }  
            array[j + 1] = key;
        }
    }
    else{
        exit(2);
    }
    return;
}
