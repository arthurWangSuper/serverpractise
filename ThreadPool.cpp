#include "ThreadPool.h"

pthread_mutex_t ThreadPool::lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Thread::notify = PTHREAD_MUTEX_INITIALIZER;
std::vector<pthread_t> ThreadPool::threads;
std::vector<ThreadPoolTask> ThreadPool::queue;
int ThreadPool::thread_count = 0;
int ThreadPool::queue_size = 0;
int ThreadPool::head = 0;
int ThreadPool::tail = 0;
int ThreadPool::count = 0;
int ThreadPool::shutdown = 0;
int ThreadPool::started = 0;

int ThreadPool::threadpool_create(int _thread_count, int _queue_size){
	bool err = false;

	do{
		if(_thread_count <= 0 || _thread_count > MAX_THREADS || _queue_size <=0 || _queue_size > MAX_THREADS){
			_thread_count = 4;
			_queue_size = 1024;
		}
		thread_count = 0;
		queue_size = _queue_size;
		head = tail = count = 0;
		shutdown = started = 0;
		threads.resize(_thread_count);
		queue.resize(_queue_size)

		for(int i=0; i< _thread_count; ++i){

		    //#include<pthread.h> 
		    //int pthread_create(pthread_t *thread, const pthread_attr_t* attr, void *(*start_routine)(void*),void *arg)
			if(pthread_create(&threads[i], nullptr, threadpool_thread, (void*)(0)) != 0){
				return -1;
			}
			++thread_count;
			++started;
		} while(false);
	}
	return 0;
}

int ThreadPool::threadpool_add(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)>fun ){
	int next, err  = 0;
	if(pthread_mutex_lock(&lock) != 0){
		return THREADPOOL_LOCK_FAILURE;
	}

	do{
			// 用vector来模拟queue
			next = (tail + 1) % queue_size;
			if(count == queue_size){
				err = THREADPOOL_QUEUE_FULL;
				break;
			}

			if(shutdown){
				err = THREADPOOL_SHUTDOWN;
				break;
			}

			queue[tail].fun = fun;
			queue[tail].args = args;
			tail = next;
			++ count;
			// pthread_cond_signal : 保证至少唤醒一条遭到阻塞的线程
			// pthread_cond_broadcast: 唤醒所有遭到阻塞的线程
			if(pthread_cond_signal(&notify) != 0){
				err = THREADPOOL_LOCK_FAILURE;
				break;
			}
	} while(false);

	if(pthread_mutex_unlock(&lock) != 0)
		err = THREADPOOL_LOCK_FAILURE;
	
	return err;
}

int ThreadPool::threadpool_destory(ShutDownOption shutdown_option){
	printf("Thread pool destory !\n");
	int i, err = 0;

	if(pthread_mutex_lock(&lock) != 0){
		return THREADPOOL_LOCK_FAILURE;
	}

	do{
		if(shutdown){
			err = THREADPOOL_SHUTDOWN;
			break;
		}
		shutdown = shutdown_option;

		if((pthread_cond_broadcast(&notify) != 0) || 
				(pthread_mutex_unlock(&lock) != 0)){
			err = THREADPOOL_THREAD_FAILURE;
		}

		for(int i=0; i < thread_count; ++i)
		{
			if(pthread_join(threads[i], NULL) != 0){
				err = THREADPOOL_THREAD_FAILURE;
			}
		}
	} while(false);

	if(!err){
		threadpool_free();
	}
	return err;
}

int ThreadPool::threadpool_free(){
	if(started > 0)
		return -1;
	pthread_mutex_lock(&lock);
	pthread_mutex_destory(&lock);
	pthread_cond_destory(&notify);
	return 0;
}





