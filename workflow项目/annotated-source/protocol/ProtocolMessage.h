/*
  注释版源码文件：ProtocolMessage.h

  原始文件位置：
  workflow-master/src/protocol/ProtocolMessage.h

  本文件用途：
  ProtocolMessage 是 Workflow 协议消息的基础接口。

  关键理解：
  - CommMessageOut 负责“怎么把请求编码成字节发出去”。
  - CommMessageIn 负责“怎么把收到的字节解析成响应对象”。
  - ProtocolMessage 同时继承两者，因此一个协议消息既可以输出，也可以输入。

  对随机图库项目的意义：
  - HttpRequest/HttpResponse、RedisRequest/RedisResponse、MySQLRequest/MySQLResponse 都建立在这个抽象之上。
  - 面试讲协议层时，可以从 encode() 和 append() 解释框架如何支持多协议。
*/

#ifndef _PROTOCOLMESSAGE_H_                                   // 头文件保护宏。
#define _PROTOCOLMESSAGE_H_                                   // 定义头文件保护宏。

#include <errno.h>                                            // errno、ENOSYS。
#include <stddef.h>                                           // size_t。
#include <utility>                                            // std::move。
#include "Communicator.h"                                     // CommMessageOut、CommMessageIn。

/**
 * @file   ProtocolMessage.h
 * @brief  General Protocol Interface
 */

namespace protocol                                             // protocol 命名空间，保存所有协议相关类。
{

class ProtocolMessage : public CommMessageOut, public CommMessageIn // ProtocolMessage：协议消息基类，同时支持输出编码和输入解析。
{
protected:
	virtual int encode(struct iovec vectors[], int max)          // encode(vectors,max)：把消息编码成 iovec 数组用于发送。
	{
		errno = ENOSYS;                                         // 默认表示“函数未实现”。
		return -1;                                             // 返回 -1 表示编码失败；子类必须覆盖。
	}

	/* You have to implement one of the 'append' functions, and the first one
	 * with arguement 'size_t *size' is recommmended. */

	/* Argument 'size' indicates bytes to append, and returns bytes used. */
	virtual int append(const void *buf, size_t *size)            // append(buf,size*)：解析收到的数据，并通过 size 返回实际消费字节数。
	{
		return this->append(buf, *size);                        // 默认转调用 append(buf,size)，表示尝试消费全部数据。
	}

	/* When implementing this one, all bytes are consumed. Cannot support
	 * streaming protocol. */
	virtual int append(const void *buf, size_t size)             // append(buf,size)：解析收到的数据，默认认为全部字节都会被消费。
	{
		errno = ENOSYS;                                         // 默认未实现。
		return -1;                                             // 返回 -1 表示解析失败；子类通常要覆盖。
	}

public:
	void set_size_limit(size_t limit) { this->size_limit = limit; } // set_size_limit(limit)：设置消息最大尺寸限制。
	size_t get_size_limit() const { return this->size_limit; }      // get_size_limit()：获取消息最大尺寸限制。

public:
	class Attachment                                            // Attachment：协议消息的附加数据基类。
	{
	public:
		virtual ~Attachment() { }                               // 虚析构函数，允许通过基类指针释放子类附件。
	};

	void set_attachment(Attachment *att) { this->attachment = att; } // set_attachment(att)：给消息挂载附加对象。
	Attachment *get_attachment() { return this->attachment; }        // get_attachment()：获取可修改附件。
	const Attachment *get_attachment() const { return this->attachment; } // get_attachment() const：获取只读附件。

protected:
	virtual int feedback(const void *buf, size_t size)           // feedback(buf,size)：解析器向通信层反馈已处理数据。
	{
		if (this->wrapper)                                      // 如果当前消息被 ProtocolWrapper 包装。
			return this->wrapper->feedback(buf, size);           // 把反馈转交给 wrapper。
		else                                                    // 如果没有 wrapper。
			return this->CommMessageIn::feedback(buf, size);     // 调用底层 CommMessageIn 默认反馈。
	}

	virtual void renew()                                        // renew()：重置输入解析状态，准备复用消息对象。
	{
		if (this->wrapper)                                      // 如果存在 wrapper。
			return this->wrapper->renew();                      // 转交给 wrapper 重置。
		else                                                    // 如果没有 wrapper。
			return this->CommMessageIn::renew();                // 调用底层输入消息重置。
	}

	virtual ProtocolMessage *inner() { return this; }            // inner()：返回最内层真实协议消息；普通消息就是自己。

protected:
	size_t size_limit;                                          // size_limit：消息大小上限，防止超大包撑爆内存。

private:
	Attachment *attachment;                                     // attachment：用户或协议层挂载的附加数据。
	ProtocolMessage *wrapper;                                  // wrapper：外层协议包装器指针。

public:
	ProtocolMessage()                                           // 构造函数：初始化协议消息。
	{
		this->size_limit = (size_t)-1;                          // 默认大小限制为最大 size_t，相当于不限制。
		this->attachment = NULL;                                // 默认没有附件。
		this->wrapper = NULL;                                   // 默认没有包装器。
	}

	virtual ~ProtocolMessage() { delete this->attachment; }      // 析构函数：释放附件。

public:
	ProtocolMessage(ProtocolMessage&& message)                  // 移动构造函数：移动另一个消息的资源。
	{
		this->size_limit = message.size_limit;                  // 复制消息大小限制。
		this->attachment = message.attachment;                  // 接管附件指针。
		message.attachment = NULL;                              // 清空源对象附件，避免重复释放。
		this->wrapper = NULL;                                   // 新对象默认不继承 wrapper。
	}

	ProtocolMessage& operator = (ProtocolMessage&& message)      // 移动赋值函数。
	{
		if (&message != this)                                   // 防止自我移动赋值。
		{
			this->size_limit = message.size_limit;              // 复制大小限制。
			delete this->attachment;                            // 释放当前已有附件，避免内存泄漏。
			this->attachment = message.attachment;              // 接管源对象附件。
			message.attachment = NULL;                          // 清空源对象附件指针。
		}

		return *this;                                           // 返回当前对象引用，支持链式赋值。
	}

	friend class ProtocolWrapper;                               // 允许 ProtocolWrapper 访问私有 wrapper 字段。
};

class ProtocolWrapper : public ProtocolMessage                 // ProtocolWrapper：协议包装器，用于在原始消息外包一层逻辑。
{
protected:
	virtual int encode(struct iovec vectors[], int max)          // encode(vectors,max)：包装器的编码。
	{
		return this->message->encode(vectors, max);             // 默认直接转发给被包装的 message。
	}

	virtual int append(const void *buf, size_t *size)            // append(buf,size*)：包装器的解析入口。
	{
		return this->message->append(buf, size);                // 默认直接转发给被包装的 message。
	}

protected:
	virtual ProtocolMessage *inner()                            // inner()：获取最内层消息。
	{
		return this->message->inner();                          // 递归向内查找真实消息。
	}

protected:
	void set_message(ProtocolMessage *message)                  // set_message(message)：设置被包装消息。
	{
		this->message = message;                                // 保存被包装消息指针。
		if (message)                                            // 如果消息不为空。
			message->wrapper = this;                            // 让内层消息知道自己的外层 wrapper。
	}

protected:
	ProtocolMessage *message;                                  // message：被包装的协议消息。

public:
	ProtocolWrapper(ProtocolMessage *message)                   // 构造函数：输入要包装的消息。
	{
		this->set_message(message);                             // 设置被包装消息并建立反向 wrapper 指针。
	}

public:
	ProtocolWrapper(ProtocolWrapper&& wrapper) :                // 移动构造函数：移动另一个 wrapper。
		ProtocolMessage(std::move(wrapper))                     // 先移动父类 ProtocolMessage 部分。
	{
		this->set_message(wrapper.message);                     // 接管源 wrapper 的 message。
		wrapper.message = NULL;                                 // 清空源 wrapper，避免重复指向。
	}

	ProtocolWrapper& operator = (ProtocolWrapper&& wrapper)      // 移动赋值函数。
	{
		if (&wrapper != this)                                   // 防止自我移动赋值。
		{
			*(ProtocolMessage *)this = std::move(wrapper);       // 移动父类部分。
			this->set_message(wrapper.message);                 // 接管被包装消息。
			wrapper.message = NULL;                             // 清空源 wrapper。
		}

		return *this;                                           // 返回当前对象引用。
	}
};

}                                                             // namespace protocol

#endif                                                        // 结束头文件保护。

