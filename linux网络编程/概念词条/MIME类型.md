---
title: MIME类型
tags: [linux, 网络编程, 概念词条, 网络编程/Web]
---
# MIME类型

## 它是什么

MIME 类型用来告诉浏览器“响应体是什么内容类型”。

## 常见取值

- `text/html`：HTML 页面
- `text/css`：CSS 文件
- `application/javascript`：JavaScript 文件
- `image/jpeg`：JPEG 图片
- `image/png`：PNG 图片

## 怎么理解

服务器返回文件时，除了返回文件内容，还要在响应头里写 `Content-Type`。浏览器靠它决定怎么解析资源。

## 常见出现位置

- [[linux网络编程/概念词条/HTTP响应报文|HTTP 响应报文]]
- [[linux网络编程/课时笔记/08 Web大作业/04 静态Web服务器基础|04 静态Web服务器基础]]
