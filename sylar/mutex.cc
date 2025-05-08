/*
 * @Author: Nana5aki
 * @Date: 2024-11-27 09:06:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2024-11-27 18:53:59
 * @FilePath: /MySylar/sylar/mutex.cc
 */

#include "mutex.h"
#include <stdexcept>

namespace sylar {

Semaphore::Semaphore(uint32_t count) {
    /**
     * int sem_init(sem_t *sem,int pshared,unsigned int value);
     * sem: 信号量
     * pshared：允许几个进程共享该信号量，一般设0用于进程内多线程共享
     * value：可用资源数目
     * sem_init() 成功时返回 0；错误时，返回 -1，并把 errno 设置为合适的值
     */
    if (sem_init(&m_semaphore, 0, count)) {
        // 最好不要在头文件中throw异常
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore() {
    sem_destroy(&m_semaphore);
}

void Semaphore::wait() {
    if (sem_wait(&m_semaphore)) {
        throw std::logic_error("sem_wait error");
    }
}

void Semaphore::notify() {
    if (sem_post(&m_semaphore)) {
        throw std::logic_error("sem_post error");
    }
}

}   // namespace sylar
