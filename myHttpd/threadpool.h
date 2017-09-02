#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <pthread.h>
#include <semaphore.h>

template< typename T >
class threadpool {
    public:
        /*
         * 参数thread_number是线程池中线程的数量，max_requests是请求队列中
         * 最多允许的等待处理的请求的数量
         */
        threadpool(int thread_number = 8, int max_requests = 10000);
        ~threadpool();
        // 往请求队列中添加任务
        bool append(T* request);
    private:

        //工作线程运行的函数，不断从工作队列中取出任务并执行
        static void *worker(void *arg);
        void run();

        //线程池内部参数
        int thread_number; //线程池中线程数
        int max_requests;  //请求队列中允许的最大请求数
        pthread_t *threads; //描述线程池的数组，大小为thread_number
        std::list<T*> workqueue; //请求队列
        pthread_mutex_t queuelocker; //保护请求队列的互斥锁
        sem_t queuestat; //是否有任务需要处理
        bool stop; //是否结束线程
};

template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests):
    thread_number(thread_number), max_requests(max_requests),stop(false), threads(NULL)
{

        threads = new pthread_t[thread_number];
        if (!threads)
            throw std::exception();
        //初始化互斥量和信号量
        pthread_mutex_init(&queuelocker, NULL);
        sem_init(&queuestat, 0, 0);

        for (int i = 0; i < thread_number; ++i) {
            printf("create the %dth thread\n", i);
            pthread_create(threads+i, NULL, worker, this) != 0;
            //把线程设置为脱离线程
            pthread_detach(threads[i]);
        }

        printf("create end\n");
}

template<typename T>
threadpool<T>::~threadpool() {
    delete[] threads;
    stop = true;
}

template<typename T>
bool threadpool<T>::append(T *request) {
    pthread_mutex_lock(&queuelocker);
    if (workqueue.size() > max_requests) {
        pthread_mutex_unlock(&queuelocker);
        return false;
    }
    workqueue.push_back(request);
    pthread_mutex_lock(&queuelocker);
    sem_post(&queuestat);
    return true;
}

template<typename T>
void *threadpool<T>::worker(void *arg) {
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run() {
    while(!stop) {
        sem_wait(&queuestat);
        pthread_mutex_lock(&queuelocker);
        if (workqueue.empty()) {
            pthread_mutex_unlock(&queuelocker);
            continue;
        }
        T *request = workqueue.front();
        workqueue.pop_front();
        pthread_mutex_unlock(&queuelocker);
        if (!request)
            continue;
        request->process();
    }
}

#endif
