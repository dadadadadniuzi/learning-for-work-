/*
  注释版源码文件：WFTaskFactory.cc

  原始文件位置：
  workflow-master/src/factory/WFTaskFactory.cc

  本文件用途：
  这里实现一部分非模板工厂函数，尤其是 Timer、命名任务、Guard、TimedGo 等。

  对随机图库项目最重要：
  1. create_timer_task() 如何把 Timer 任务绑定到 WFGlobal::get_scheduler()。
  2. 命名 Timer/Counter/Mailbox/Conditional/Guard 目前不是项目主线，可后续再深挖。

  注意：
  这个注释版文件保留项目主线和关键机制注释。
  命名控制任务的完整逐行注释可以作为后续补充批次。
*/

#include <sys/types.h>                         // 系统类型。
#include <errno.h>                             // errno。
#include <time.h>                              // timespec/time_t。
#include <utility>                             // std::move。
#include <string>                              // std::string。
#include <mutex>                               // std::mutex。
#include <atomic>                              // std::atomic。
#include "list.h"                              // 内部链表。
#include "rbtree.h"                            // 红黑树，用于按名字管理任务。
#include "WFGlobal.h"                          // 全局 scheduler 等资源。
#include "WFTaskFactory.h"                     // 工厂声明。

class __WFTimerTask : public WFTimerTask        // __WFTimerTask：普通定时器任务内部实现。
{
protected:
	virtual int duration(struct timespec *value) // duration(value)：告诉 SleepRequest 要睡多久。
	{
		value->tv_sec = this->seconds;           // tv_sec：秒数，来自成员变量 seconds。
		value->tv_nsec = this->nanoseconds;      // tv_nsec：纳秒数，来自成员变量 nanoseconds。
		return 0;                                // 返回 0 表示成功提供时间。
	}

protected:
	time_t seconds;                              // seconds：定时秒数。
	long nanoseconds;                            // nanoseconds：额外纳秒数。

public:
	__WFTimerTask(time_t seconds, long nanoseconds, CommScheduler *scheduler,
				  timer_callback_t&& cb) :
		WFTimerTask(scheduler, std::move(cb))    // 初始化父类 WFTimerTask，绑定 scheduler 和 callback。
	{
		this->seconds = seconds;                 // 保存秒数。
		this->nanoseconds = nanoseconds;         // 保存纳秒数。
	}
};

class __WFCanceledTimerTask : public __WFTimerTask // __WFCanceledTimerTask：特殊 timer，用于切线程/异步触发回调。
{
protected:
	virtual void dispatch()                       // dispatch()：启动后立即注册 sleep，然后取消。
	{
		if (this->scheduler->sleep(this) >= 0)    // 向 scheduler 注册 sleep。
			this->cancel();                       // 注册成功后立即取消，用于让回调在 handler 线程执行。
		else                                     // 注册失败。
			this->handle(WFT_STATE_SYS_ERROR, errno); // 直接按系统错误完成任务。
	}

public:
	__WFCanceledTimerTask(CommScheduler *scheduler, timer_callback_t&& cb) :
		__WFTimerTask(-1, 0, scheduler, std::move(cb)) // duration 设置为 -1 秒，配合 cancel 使用。
	{
	}
};

WFTimerTask *WFTaskFactory::create_timer_task(time_t seconds, long nanoseconds,
											  timer_callback_t callback)
{
	return new __WFTimerTask(seconds, nanoseconds, WFGlobal::get_scheduler(),
							 std::move(callback)); // 创建普通定时器，绑定全局 scheduler。
}

WFTimerTask *WFTaskFactory::create_timer_task(timer_callback_t callback)
{
	return new __WFCanceledTimerTask(WFGlobal::get_scheduler(),
									 std::move(callback)); // 创建特殊立即取消 timer，常用于切换回调执行线程。
}

WFTimerTask *WFTaskFactory::create_timer_task(unsigned int microseconds,
											  timer_callback_t callback)
{
	return WFTaskFactory::create_timer_task(microseconds / 1000000,
											microseconds % 1000000 * 1000,
											std::move(callback)); // 旧接口：微秒换算成秒+纳秒。
}

/*
  Named Tasks 说明：
  下面的 __NamedObjectList 和 __get_object_list 是命名任务的通用基础设施。
  它用红黑树按 name 查找一组同名任务，再用链表保存同名任务列表。
  命名 Timer、Counter、Mailbox、Conditional、Guard 都复用这个结构。
*/

template<typename T>
struct __NamedObjectList                                      // __NamedObjectList：同名对象链表。
{
	__NamedObjectList(const std::string& str):                 // 构造函数参数 str：名字。
		name(str)                                             // 保存名字。
	{
		INIT_LIST_HEAD(&this->head);                           // 初始化链表头。
	}

	void push_back(T *node)                                    // push_back(node)：把节点加入链表尾部。
	{
		list_add_tail(&node->list, &this->head);               // 使用内核风格链表插入。
	}

	bool empty() const                                         // empty()：判断链表是否为空。
	{
		return list_empty(&this->head);                        // 调用 list_empty。
	}

	bool del(T *node, rb_root *root)                           // del(node, root)：从链表删除节点；如果链表空了也从红黑树删除。
	{
		list_del(&node->list);                                 // 从链表删除当前节点。
		if (this->empty())                                     // 如果同名列表已经空。
		{
			rb_erase(&this->rb, root);                         // 从红黑树删除这个名字项。
			return true;                                      // 返回 true 表示整个列表对象也可以删除。
		}
		else
			return false;                                     // 列表还有节点，不能删除列表对象。
	}

	struct rb_node rb;                                         // rb：红黑树节点，用于按 name 索引。
	struct list_head head;                                     // head：同名对象链表头。
	std::string name;                                          // name：对象名称。
};

template<typename T>
static T *__get_object_list(const std::string& name, struct rb_root *root,
							bool insert)
{
	struct rb_node **p = &root->rb_node;                       // p：当前红黑树搜索位置。
	struct rb_node *parent = NULL;                             // parent：插入时的父节点。
	T *objs;                                                   // objs：找到的对象列表。
	int n;                                                     // n：name 比较结果。

	while (*p)                                                 // 从根节点开始查找。
	{
		parent = *p;                                           // 记录父节点。
		objs = rb_entry(*p, T, rb);                            // 从 rb_node 还原出对象列表指针。
		n = name.compare(objs->name);                          // 比较要找的 name 和当前节点 name。
		if (n < 0)                                             // 要找的 name 更小。
			p = &(*p)->rb_left;                                // 去左子树。
		else if (n > 0)                                        // 要找的 name 更大。
			p = &(*p)->rb_right;                               // 去右子树。
		else
			return objs;                                       // 找到同名列表，直接返回。
	}

	if (insert)                                                // 如果没找到且允许插入。
	{
		objs = new T(name);                                    // 创建新的同名列表。
		rb_link_node(&objs->rb, parent, p);                    // 挂到红黑树位置。
		rb_insert_color(&objs->rb, root);                      // 调整红黑树颜色保持平衡。
		return objs;                                          // 返回新列表。
	}

	return NULL;                                               // 没找到且不插入，返回 NULL。
}

/*
  为了避免这个阅读版过长，下面命名 Timer/Counter/Mailbox/Conditional/Guard 的具体实现
  暂不逐行展开。你当前随机图库项目第一阶段不会用到这些能力。

  真正要理解项目主链路，应优先看：
  - WFTaskFactory.h：知道有哪些创建入口。
  - WFTaskFactory.inl：知道网络任务如何初始化 URL、路由、重试、创建服务端任务。
  - WFTask.h / WFTask.inl：知道网络任务和服务端任务如何执行。
*/

/*
  Timed Go Task 说明：
  原始文件末尾的 __WFTimedGoTask 用 Timer + GoTask 组合实现“带超时的计算任务”。
  如果计算先完成，使用计算结果；
  如果 timer 先触发，则任务以 ETIMEDOUT 结束。
  随机图库项目一般不需要先看这部分，除非你要把图片处理、缩略图生成放到 compute 线程池里。
*/

