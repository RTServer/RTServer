/**
gcc -o test_create_pthread_maxnum test_create_pthread_maxnum.c -lpthread
线程应用程序最常见导致创建线程失败的原因是线程栈大小的设置。创建一个新的线程，默认情况下系统为线程栈预留了2MB的寻址空间。线程栈起始于进程虚拟 内存的高端地址，并向虚拟内存底端地址方向扩展。取决于线程本身的大小以及其它线程内存分配的情况，进程虚拟地址空间消耗过快可能导致创建线程失败。
这里有一个测试程序可以看到，Linux下最多可以创建多少个线程。
*/

#include <pthread.h>
#include <stdio.h>
#include <string.h>
/*
void *ThreadFunc() {
    static int count = 1;
    printf ("Create thread%d\n", count);
    pthread_detach(pthread_self());
    count++;
    return NULL;
}

int main(void) {
    int     err;
    pthread_t tid;
    while (1){
           err = pthread_create(&tid, NULL, ThreadFunc, NULL);
           if(err != 0){
               printf("can't create thread: %s\n",strerror(err));
           break;
           }  
    }
    return 0;
}
*/

void *ThreadFunc() {
    static int count = 1;
    printf ("Create thread%d\n", count);
    //pthread_detach(pthread_self());
    count++;
    return NULL;
}

int main(void) {
    int     err;

    pthread_t child_thread;
    pthread_attr_t child_thread_attr;
    pthread_attr_init(&child_thread_attr);
    pthread_attr_setdetachstate(&child_thread_attr, PTHREAD_CREATE_DETACHED);

    while (1){
           err = pthread_create(&child_thread, &child_thread_attr, ThreadFunc, NULL);
           if(err != 0){
               printf("can't create thread: %s\n",strerror(err));
           break;
           }  
    }
    return 0;
}

/**
输出结果如下：
Create thread 301
Create thread 302
can't create thread: Cannot allocate memory
用 ulimit -s 可以查看到栈的默认大小为10240K
*/



/**
32位linux下的进程用户空间是3072M， 3072/10.24=300。为什么实际会比计算出来的多2个，这个原因还不太清楚。（编者注：准确算法是(3072*1024K)/10240K=307.2，实际会比计算出来的少5.2个）
可以在调用pthread_create 的时候用 pthread_attr_getstacksize 设置栈的大小，或者直接用 ulimit -s 设置栈的大小。
如果修改上面的测试代码为
*/

/**
#include <pthread.h>
#include <stdio.h>
#include <string.h>

void *ThreadFunc() {
    static int count = 1;
    printf ("Create thread%d/n", count);
    pthread_detach(pthread_self());
    count++;
}

main(void) {
    int     err;
    pthread_t tid;
    while (1){
           err= pthread_create(&tid, NULL, ThreadFunc, NULL);
           if(err != 0){
               printf("can'tcreate thread: %s/n", strerror(err));
           break;
           }  
    }
}
*/

/**
那么得到的结果将是：
Create thread 560000
Create thread 560001
……………

这里用到了pthread_detach(pthread_self())来释放线程所占用的内存资源（线程内核对象和线程堆栈）。这样就可以创建更多的线程，而不会出现Cannot allocate memory的错误。

如果进程中的某个线程执行了pthread_detach(th)，则th线程将处于DETACHED状态，这使得th线程在结束运行时自行释放所占用的内存资源。一个可join的线程所占用的内存仅当有线程对其执行了pthread_join()后才会释放，因此为了避免内存泄漏，所有线程的终止，要么已设为DETACHED，要么就需要使用pthread_join()来回收。
*/