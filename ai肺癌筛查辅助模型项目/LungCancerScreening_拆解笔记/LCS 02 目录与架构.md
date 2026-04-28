---
title: LCS 02 目录与架构
aliases:
  - LungCancerScreening 目录结构
tags:
  - project
  - architecture
  - interview
status: active
created: 2026-04-24
---

# LCS 02 目录与架构

关联：[[LCS 01 项目总览]] | [[LCS 03 从零实现流程]] | [[LCS 04 工具层与公共模块]] | [[LCS 09 Web 后端与前端]]

## 目录应该怎么讲

面试时最好不要逐个文件硬背，而是按“层”讲：

```text
LungCancerScreening-main/
├─ util/                  公共工具层
├─ segment*               第一阶段：分割
├─ dsets.py/model.py/...  第二阶段：真假结节分类
├─ Tumor*                 第三阶段：良恶性分类
├─ model_evel.py          三阶段联合评估
└─ web_app/               系统落地层
```

## 各层职责

### 1. 工具层

对应：[[LCS 04 工具层与公共模块]]

作用：

1. 坐标转换
2. 缓存
3. U-Net 实现
4. 训练日志辅助

### 2. 分割层

对应：[[LCS 05 第一阶段 分割模块]]

核心文件：

1. `segmentDsets.py`
2. `segmentModel.py`
3. `segmentTraining.py`
4. `prep_seg_cache.py`

### 3. 真假结节分类层

对应：[[LCS 06 第二阶段 真假结节分类]]

核心文件：

1. `dsets.py`
2. `model.py`
3. `training.py`
4. `prepcache.py`

### 4. 良恶性分类层

对应：[[LCS 07 第三阶段 良恶性分类]]

核心文件：

1. `TumorDatasets.py`
2. `TumorModel.py`
3. `TumorTraining.py`
4. `TumorPrepCache.py`

### 5. 联合评估层

对应：[[LCS 08 整体推理与评估]]

核心文件：

1. `model_evel.py`

### 6. 系统落地层

对应：[[LCS 09 Web 后端与前端]]

核心文件：

1. `web_app/app.py`
2. `web_app/start_system.py`
3. `web_app/frontend/src/...`

## 你可以怎么概括架构

这个项目的架构不是“一个模型 + 一个接口”，而是“训练代码、推理代码、评估代码、服务层、展示层”全都有。也就是说，它已经从研究型代码走到系统原型阶段了。

