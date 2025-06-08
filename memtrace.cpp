//#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <chrono>
#include <fstream>
#include <signal.h>
#include <sys/prctl.h>
#include <unwind.h>
#include <dlfcn.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <ctype.h>
#include <regex.h>
#include <setjmp.h>
#include <assert.h>
#include <malloc.h>
#include <iostream>

// 内存分配记录结构
typedef struct mem_record {
    void* ptr;               // 分配的内存地址
    size_t size;             // 分配的内存大小
    struct mem_record* next; // 下一个记录
} mem_record;

// 全局变量
static mem_record* records = NULL;       // 内存记录链表头
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 互斥锁
static int tracing_enabled = 1;          // 跟踪开关（防止初始化时递归）
static int real_init = 0;

typedef void* (*real_malloc_T)(size_t);
typedef void* (*real_calloc_T)(size_t, size_t);
typedef void* (*real_realloc_T)(void*, size_t);
typedef void (*real_free_T)(void *);


// 原始函数指针
static void* (*orig_malloc)(size_t) = NULL;
static void* (*orig_calloc)(size_t, size_t) = NULL;
static void* (*orig_realloc)(void*, size_t) = NULL;
static void (*orig_free)(void*) = NULL;

extern "C" {

void *__real_malloc(size_t size);
void *__real_calloc(size_t, size_t);
void *__real_realloc(void*, size_t);
void __real_free(void *ptr);



// 添加内存记录
static void add_record(void* ptr, size_t size) {
    if (!ptr || !tracing_enabled) return;
    
    mem_record* new_rec = (mem_record*)__real_malloc(sizeof(mem_record));
    if (!new_rec) return;
    
    new_rec->ptr = ptr;
    new_rec->size = size;
    
    pthread_mutex_lock(&mutex);
    new_rec->next = records;
    records = new_rec;
    pthread_mutex_unlock(&mutex);
}

// 移除内存记录
static void remove_record(void* ptr) {
    if (!ptr || !tracing_enabled) return;
    
    pthread_mutex_lock(&mutex);
    
    mem_record** curr = &records;
    while (*curr) {
        if ((*curr)->ptr == ptr) {
            mem_record* temp = *curr;
            *curr = temp->next;
            __real_free(temp);
            pthread_mutex_unlock(&mutex);
            return;
        }
        curr = &(*curr)->next;
    }
    
    pthread_mutex_unlock(&mutex);
}


/*
-Wl,--wrap=malloc,--wrap=free,--wrap=calloc,--wrap=realloc
*/
// 重载的malloc函数



void * __wrap_malloc(size_t size) {
	
    void* ptr = __real_malloc(size);
    if (ptr) add_record(ptr, size);
    return ptr;
}

// 重载的calloc函数
void* __wrap_calloc(size_t num, size_t size) {
    
    void* ptr = __real_calloc(num, size);
    if (ptr) add_record(ptr, num * size);
    return ptr;
}

// 重载的realloc函数
void* __wrap_realloc(void* ptr, size_t new_size) {
    
    // 处理特殊情况
    if (new_size == 0) {
        __real_free(ptr);
        remove_record(ptr);
        return NULL;
    }
    if (!ptr) return __wrap_malloc(new_size);
    
    // 执行realloc
    void* new_ptr = __real_realloc(ptr, new_size);
    if (new_ptr) {
        remove_record(ptr);
        add_record(new_ptr, new_size);
    }
    return new_ptr;
}

// 重载的free函数
void __wrap_free(void* ptr) {
    if (!ptr) return;
    
    remove_record(ptr);
    __real_free(ptr);
}


// 打印内存泄漏报告
//__attribute__((destructor))
void report_leaks() {
    tracing_enabled = 0; // 禁用跟踪防止递归
    
    pthread_mutex_lock(&mutex);
    mem_record* current = records;
    size_t leak_count = 0;
    size_t total_leaked = 0;
    
    while (current) {
        leak_count++;
        total_leaked += current->size;
        fprintf(stderr, "Memory leak at %p: %zu bytes\n", 
                current->ptr, current->size);
        
        mem_record* next = current->next;
        __real_free(current);
        current = next;
    }
    
    if (leak_count > 0) {
        fprintf(stderr, "\nTotal leaks: %zu, Total bytes: %zu\n", 
                leak_count, total_leaked);
    } else {
        fprintf(stderr, "No memory leaks detected\n");
    }
    
    records = NULL;
    pthread_mutex_unlock(&mutex);
    
    //tracing_enabled = 1; // 禁用跟踪防止递归
}

void * MEMLEAK01(void *argv)
{
    //while(1)
    {
	sleep( 10 );
	
	report_leaks();
    }

    return NULL;
}

// 初始化原始函数指针
__attribute__((constructor)) static void init_orig_functions() {
//void init_orig_functions() {
    //orig_malloc = (real_malloc_T)dlsym(RTLD_NEXT, "malloc");
    //orig_calloc = (real_calloc_T)dlsym(RTLD_NEXT, "calloc");
    //orig_realloc = (real_realloc_T)dlsym(RTLD_NEXT, "realloc");
    //orig_free = (real_free_T)dlsym(RTLD_NEXT, "free");
    
    //if (!orig_malloc || !orig_calloc || !orig_realloc || !orig_free) {
    //    ;
    //}
    //else
    {
    	real_init = 1;
    }
    
    pthread_t thread_1;
    pthread_create(&thread_1, NULL, MEMLEAK01, (void*)NULL);
    
}


}
