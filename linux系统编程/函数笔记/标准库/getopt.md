---
title: getopt
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/标准库
---
# getopt

> [!info] 功能
> 按照选项规则解析命令行参数，常用于处理 `-a`、`-p 8080`、`-f file.txt` 这类启动参数。

## 函数原型

- `int getopt(int argc, char * const argv[], const char *optstring);`

## 依赖头文件

- `#include <unistd.h>`

## 输入参数

- `argc`
  命令行参数个数，通常直接使用 `main` 函数里的 `argc`。
  它表示 `argv` 数组里一共有多少项。

- `argv`
  命令行参数数组，通常直接使用 `main` 函数里的 `argv`。
  - `argv[0]` 一般是程序名
  - `argv[1]` 开始才是用户真正输入的参数
  - `getopt` 会在这个数组里按顺序扫描选项

- `optstring`
  选项规则字符串，用来告诉 `getopt` 哪些选项合法、哪些选项后面必须带参数。

常见写法：

- `"ab"`：表示支持 `-a` 和 `-b`，它们都不带参数
- `"p:"`：表示支持 `-p`，而且 `-p` 后面必须跟一个参数
- `"abf:"`：表示支持 `-a`、`-b`、`-f`，其中 `-f` 后面必须带参数

规则要点：

- 普通字符表示一个合法短选项
- 某个字符后面跟一个 `:`，表示该选项必须带参数
- 例如 `"t:n:"` 表示 `-t` 和 `-n` 都必须带参数

## 输出参数

- 无直接输出参数。

## 返回值

- 每成功解析到一个选项，就返回该选项字符。
- 当所有选项都处理完后，返回 `-1`。
- 当遇到非法选项或缺少参数时，通常返回 `?`。

## 怎么理解

`getopt` 的核心作用是把杂乱的命令行字符串拆成“选项”和“选项参数”。

比如执行：

```bash
./app -p 8080 -f conf.txt -a
```

`getopt` 会帮你识别出：

- `-p` 是一个选项，参数是 `8080`
- `-f` 是一个选项，参数是 `conf.txt`
- `-a` 是一个不带参数的选项

这样程序就不用自己手动判断 `argv[1]`、`argv[2]`、`argv[3]` 分别代表什么。

## 常见用法

```c
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "p:f:a")) != -1) {
        switch (opt) {
            case 'p':
                printf("port = %s\n", optarg);
                break;
            case 'f':
                printf("file = %s\n", optarg);
                break;
            case 'a':
                printf("option a enabled\n");
                break;
            case '?':
                printf("invalid option\n");
                break;
        }
    }

    return 0;
}
```

如果执行：

```bash
./app -p 8080 -f conf.txt -a
```

程序就能依次解析出这些选项。

## 知识点补充

- `getopt` 解析带参数的选项时，会把对应参数放到 `[[linux系统编程/概念词条/optarg|optarg]]` 中。
- `getopt` 当前处理到哪个位置，会反映在 `[[linux系统编程/概念词条/optind|optind]]` 中。
- 当出现非法选项时，出问题的选项字符通常可以通过 `[[linux系统编程/概念词条/optopt|optopt]]` 查看。
- 它主要处理的是“短选项”，也就是 `-a`、`-b` 这种形式。
- 如果项目里有 `--port`、`--help` 这种长选项，通常会看到 `getopt_long`。

## 易错点

- `optstring` 里有没有 `:` 很关键，写错后程序会把参数解析乱。
- `getopt` 返回的是字符，不是字符串。
- `optarg` 只有在“当前选项需要参数”时才有意义。
- 解析结束后，`optind` 往往指向第一个“非选项参数”。
- `-1` 表示解析完成，不是错误。

## 相关笔记

- 概念词条：[[linux系统编程/概念词条/optarg|optarg]]
- 概念词条：[[linux系统编程/概念词条/optind|optind]]
- 概念词条：[[linux系统编程/概念词条/optopt|optopt]]
- 函数笔记：[[linux系统编程/函数笔记/标准库/atoi|atoi]]

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/01 Linux目录、路径与常用命令|01 Linux目录、路径与常用命令]]

## 相关模块

- [[linux系统编程/01 Linux基础与开发环境|01 Linux基础与开发环境]]
