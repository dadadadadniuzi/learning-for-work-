---
title: strcat
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/标准库
---
# strcat

> [!info] 功能
> 把一个字符串追加到另一个字符串末尾。

## 函数原型

- `char *strcat(char *dest, const char *src);`

## 依赖头文件

- `#include <string.h>`

## 输入参数

- `dest`
  目标字符串缓冲区。
  它原本就应该是一个合法的、以 `\0` 结尾的字符串。
  `strcat` 会先找到它原来的结尾，再把 `src` 拼接到后面。

- `src`
  要追加的源字符串。
  它也必须是一个以 `\0` 结尾的 C 风格字符串。

## 输出参数

- 无直接输出参数。
- 拼接结果会写回 `dest`。

## 返回值

- 返回目标字符串起始地址，也就是 `dest`。

## 怎么理解

`strcat` 做的事不是覆盖，而是“接在后面”。

例如：

```c
char dest[32] = "hello ";
char src[] = "linux";

strcat(dest, src);
```

执行后，`dest` 变成 `"hello linux"`。

## 常见用法

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char path[64] = "/home/";

    strcat(path, "user");
    printf("%s\n", path);

    return 0;
}
```

## 易错点

- `dest` 必须先是一个合法字符串，不能是一块未初始化的随机内存。
- `dest` 必须有足够空间容纳“原内容 + src + 结尾的 \0”。
- `strcat` 不会检查目标缓冲区大小。
- 如果空间不够，也会出现缓冲区溢出。

## 和相关函数的区别

- `strcpy`：把源字符串整体复制过去，覆盖原来的内容。
- `strcat`：在目标字符串原内容后面继续追加。
- `[[linux系统编程/概念词条/memcpy|memcpy]]`：复制的是字节，不负责字符串拼接。

## 知识点补充

- `strcat` 内部本质上会先找到 `dest` 末尾的 `\0`，再从那里开始拷贝 `src`。
- 所以它比直接已知位置写入更慢一些，因为它需要先扫描一次目标字符串。
- 拼接路径、构造命令字符串、拼消息时常见到它。

## 相关笔记

- 函数笔记：[[linux系统编程/函数笔记/标准库/strcpy|strcpy]]
- 概念词条：[[linux系统编程/概念词条/memcpy|memcpy]]
- 概念词条：[[linux系统编程/概念词条/memset|memset]]

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/01 Linux目录、路径与常用命令|01 Linux目录、路径与常用命令]]

## 相关模块

- [[linux系统编程/01 Linux基础与开发环境|01 Linux基础与开发环境]]
