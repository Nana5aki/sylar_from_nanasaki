/*
 * @Author: Nana5aki
 * @Date: 2025-01-01 15:06:39
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-03-08 10:59:40
 * @FilePath: /sylar_from_nanasaki/sylar/fiber.cc
 */
#include "fiber.h"
#include "config.h"
#include "log.h"
#include "macro.h"
#include "scheduler.h"
#include <atomic>

/*
上下文结构体定义
这个结构体是平台相关的，因为不同平台的寄存器不一样
下面列出的是所有平台都至少会包含的4个成员

typedef struct ucontext_t {
    //当前上下文结束后，下一个激活的上下文对象的指针，只在当前上下文是由makecontext创建时有效
    struct ucontext_t *uc_link;
    // 当前上下文的信号屏蔽掩码
    sigset_t          uc_sigmask;
    // 当前上下文使用的栈内存空间，只在当前上下文是由makecontext创建时有效
    stack_t           uc_stack;
    // 平台相关的上下文具体内容，包含寄存器的值
    mcontext_t        uc_mcontext;
    ...
} ucontext_t;

// 获取当前的上下文
int getcontext(ucontext_t *ucp);

//恢复ucp指向的上下文，这个函数不会返回，而是会跳转到ucp上下文对应的函数中执行，相当于变相调用了函数
int setcontext(const ucontext_t *ucp);

//修改由getcontext获取到的上下文指针ucp，将其与一个函数func进行绑定，支持指定func运行时的参数，
//在调用makecontext之前，必须手动给ucp分配一段内存空间，存储在ucp->uc_stack中，这段内存空间将作为func函数运行时的栈空间，
//同时也可以指定ucp->uc_link，表示函数运行结束后恢复uc_link指向的上下文，
//如果不赋值uc_link，那func函数结束时必须调用setcontext或swapcontext以重新指定一个有效的上下文，否则程序就跑飞了
//makecontext执行完后，ucp就与函数func绑定了，调用setcontext或swapcontext激活ucp时，func就会被运行
void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...);

// 恢复ucp指向的上下文，同时将当前的上下文存储到oucp中，
//和setcontext一样，swapcontext也不会返回，而是会跳转到ucp上下文对应的函数中执行，相当于调用了函数
//swapcontext是sylar非对称协程实现的关键，线程主协程和子协程用这个接口进行上下文切换
int swapcontext(ucontext_t *oucp, const ucontext_t *ucp);
*/

namespace sylar {

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

/// 全局静态变量，用于生成协程id
static std::atomic<uint64_t> s_fiber_id{0};
/// 全局静态变量，用于统计当前的协程数
static std::atomic<uint64_t> s_fiber_count{0};

/// 线程局部变量，当前线程正在运行的协程
static thread_local Fiber* t_fiber = nullptr;
/// 线程局部变量，当前线程的主协程，切换到这个协程，就相当于切换到了主线程中运行，智能指针形式
static thread_local Fiber::ptr t_thread_fiber = nullptr;

// 协程栈大小，可通过配置文件获取，默认128k
static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
  Config::Lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

/**
 * @brief malloc栈内存分配器
 */
class MallocStackAllocator {
public:
  static void* Alloc(size_t size) {
    return malloc(size);
  }
  static void Dealloc(void* vp, size_t size) {
    return free(vp);
  }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId() {
  if (t_fiber) {
    return t_fiber->getId();
  }
  return 0;
}

Fiber::Fiber() {
  SetThis(this);
  m_state = RUNNING;

  if (getcontext(&m_ctx)) {
    SYLAR_ASSERT2(false, "getcontext");
  }

  ++s_fiber_count;
  m_id = s_fiber_id++;   // 协程id从0开始，用完加1

  SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber() main id = " << m_id;
}

void Fiber::SetThis(Fiber* f) {
  t_fiber = f;
}

/**
 * 获取当前协程，同时充当初始化当前线程主协程的作用，这个函数在使用协程之前要调用一下
 */
Fiber::ptr Fiber::GetThis() {
  if (t_fiber) {
    return t_fiber->shared_from_this();
  }

  Fiber::ptr main_fiber(new Fiber);
  SYLAR_ASSERT(t_fiber == main_fiber.get());
  t_thread_fiber = main_fiber;
  return t_fiber->shared_from_this();
}

/**
 * 带参数的构造函数用于创建其他协程，需要分配栈
 */
Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool run_in_scheduler)
  : m_id(s_fiber_id++)
  , m_cb(cb)
  , m_runInScheduler(run_in_scheduler) {
  ++s_fiber_count;
  m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
  m_stack = StackAllocator::Alloc(m_stacksize);

  if (getcontext(&m_ctx)) {
    SYLAR_ASSERT2(false, "getcontext");
  }

  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  makecontext(&m_ctx, &Fiber::MainFunc, 0);

  SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber() id = " << m_id;
}

/**
 * 线程的主协程析构时需要特殊处理，因为主协程没有分配栈和cb
 */
Fiber::~Fiber() {
  SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber() id = " << m_id;
  --s_fiber_count;
  if (m_stack) {
    // 有栈，说明是子协程，需要确保子协程一定是结束状态
    SYLAR_ASSERT(m_state == TERM);
    StackAllocator::Dealloc(m_stack, m_stacksize);
    SYLAR_LOG_DEBUG(g_logger) << "dealloc stack, id = " << m_id;
  } else {
    // 没有栈，说明是线程的主协程
    SYLAR_ASSERT(!m_cb);                // 主协程没有cb
    SYLAR_ASSERT(m_state == RUNNING);   // 主协程一定是执行状态

    Fiber* cur = t_fiber;   // 当前协程就是自己
    if (cur == this) {
      SetThis(nullptr);
    }
  }
}

/**
 * 这里为了简化状态管理，强制只有TERM状态的协程才可以重置，但其实刚创建好但没执行过的协程也应该允许重置的
 */
void Fiber::reset(std::function<void()> cb) {
  SYLAR_ASSERT(m_stack);
  SYLAR_ASSERT(m_state == TERM);
  m_cb = cb;
  if (getcontext(&m_ctx)) {
    SYLAR_ASSERT2(false, "getcontext");
  }

  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  makecontext(&m_ctx, &Fiber::MainFunc, 0);
  m_state = READY;
}

void Fiber::resume() {
  SYLAR_ASSERT(m_state != TERM && m_state != RUNNING);
  SetThis(this);
  m_state = RUNNING;

  // 如果协程参与调度器调度，那么应该和调度器的主协程进行swap，而不是线程主协程
  if (m_runInScheduler) {
    if (swapcontext(&(Scheduler::GetMainFiber()->m_ctx), &m_ctx)) {
      SYLAR_ASSERT2(false, "swapcontext");
    }
  } else {
    if (swapcontext(&(t_thread_fiber->m_ctx), &m_ctx)) {
      SYLAR_ASSERT2(false, "swapcontext");
    }
  }
}

void Fiber::yield() {
  /// 协程运行完之后会自动yield一次，用于回到主协程，此时状态已为结束状态
  SYLAR_ASSERT(m_state == RUNNING || m_state == TERM);
  if (m_state != TERM) {
    m_state = READY;
  }

  // 如果协程参与调度器调度，那么应该和调度器的主协程进行swap，而不是线程主协程
  if (m_runInScheduler) {
    SetThis(Scheduler::GetMainFiber());
    if (swapcontext(&m_ctx, &(Scheduler::GetMainFiber()->m_ctx))) {
      SYLAR_ASSERT2(false, "swapcontext");
    }
  } else {
    SetThis(t_thread_fiber.get());
    if (swapcontext(&m_ctx, &(t_thread_fiber->m_ctx))) {
      SYLAR_ASSERT2(false, "swapcontext");
    }
  }
}

/**
 * 这里没有处理协程函数出现异常的情况，同样是为了简化状态管理，并且个人认为协程的异常不应该由框架处理，应该由开发者自行处理
 */
void Fiber::MainFunc() {
  Fiber::ptr cur = GetThis();   // GetThis()的shared_from_this()方法让引用计数加1
  SYLAR_ASSERT(cur);

  cur->m_cb();
  cur->m_cb = nullptr;
  cur->m_state = TERM;

  auto raw_ptr = cur.get();   // 手动让t_fiber的引用计数减1
  cur.reset();
  raw_ptr->yield();
}

}   // namespace sylar