#include <iostream>
#include <chrono>  
#include <cstdlib> // srand(), rand()
#include <pthread.h>
#include <time.h> // time()
#include "../skiplist.h"

// 线程数量
#define NUM_THREADS 1
// 要插入要素的数量
#define TEST_COUNT 100000
SkipList<int, std::string> skipList(18);

// 函数参数： void*  表示当前线程id
void *insertElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    std::cout << tid << std::endl;
    // 用于存储每个线程需要插入的元素数量  
    int tmp = TEST_COUNT / NUM_THREADS; 
    // 为了避免不同线程同时修改同一个元素，
    // 每个线程都从tid*tmp开始向SkipList中插入元素。
    // 这样可以保证每个线程只修改自己负责的部分
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		skipList.insert_element(rand() % TEST_COUNT, "a"); 
	}
    // 线程退出
    // 线程调用了 pthread_exit() 函数并返回了一个指针，该指针将存储在pthread_join()的 retval 中。
    pthread_exit(NULL);
}

void *getElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    std::cout << tid << std::endl;  
    int tmp = TEST_COUNT / NUM_THREADS; 
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		skipList.search_element(rand() % TEST_COUNT); 
	}
    pthread_exit(NULL);
}

int main() {
    // 随机数种子
    srand (time(NULL));  
    {

        pthread_t threads[NUM_THREADS];
        int rc;
        int i;

        // 使用C++标准库中的chrono头文件中的high_resolution_clock类创建了一个时间点对象start，用于记录当前时间
        // 通常用于性能测试和时间测量等需要精确计时的场景
        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            std::cout << "main() : creating thread, " << i << std::endl;
            // 成功返回0，否则返回非零值
            /* pthread_create(pthread_t *thread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg);
               1. 指向 pthread_t 类型的指针，用于存储新线程的 ID。
               2. 用于指定新线程的属性。可以将其设置为 NULL，使用默认属性。
               3. 线程函数，新线程将从该函数的起始位置开始执行。
               4. 传递给线程函数的参数。
            */
           // intptr_t 类型来存储整数类型的变量 i，并将其转换为指针类型 void *。intptr_t 是一个整数类型，它可以容纳指针类型的值
            intptr_t arg = (intptr_t)i;
            rc = pthread_create(&threads[i], NULL, insertElement, (void *)arg);

            if (rc) {
                std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        // 存放 pthread_join函数等待一个线程的结束后的 返回值
        // 主线程阻塞，等待线程完成任务再继续执行
        void *ret;
        for( i = 0; i < NUM_THREADS; i++ ) {
               int pthread_join(pthread_t thread, void **retval);           
            // 第二个参数：指向指针的指针，用于存储线程的返回值，成功：0
            if (pthread_join(threads[i], &ret) !=0 )  {
                perror("pthread_create() error"); 
                exit(3);
            }
        }
        auto finish = std::chrono::high_resolution_clock::now();
        // 时间段类，表示一个时间段的长度，构造函数接受一个double的参数 
        std::chrono::duration<double> elapsed = finish - start;
        // elapsed.count() 返回时间段的长度，以秒为单位
        std::cout << "insert elapsed:" << elapsed.count() << std::endl;
    }

    // skipList.display_list();

    // {
    //     pthread_t threads[NUM_THREADS];
    //     int rc;
    //     int i;
    //     auto start = std::chrono::high_resolution_clock::now();

    //     for( i = 0; i < NUM_THREADS; i++ ) {
    //         std::cout << "main() : creating thread, " << i << std::endl;
    //         rc = pthread_create(&threads[i], NULL, getElement, (void *)i);

    //         if (rc) {
    //             std::cout << "Error:unable to create thread," << rc << std::endl;
    //             exit(-1);
    //         }
    //     }

    //     void *ret;
    //     for( i = 0; i < NUM_THREADS; i++ ) {
    //         if (pthread_join(threads[i], &ret) !=0 )  {
    //             perror("pthread_create() error"); 
    //             exit(3);
    //         }
    //     }

    //     auto finish = std::chrono::high_resolution_clock::now(); 
    //     std::chrono::duration<double> elapsed = finish - start;
    //     std::cout << "get elapsed:" << elapsed.count() << std::endl;
    // }

	pthread_exit(NULL);
    return 0;

}
