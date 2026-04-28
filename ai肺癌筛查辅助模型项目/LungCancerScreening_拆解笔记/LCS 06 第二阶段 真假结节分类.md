---
title: LCS 06 第二阶段 真假结节分类
aliases:
  - LungCancerScreening 真假结节分类
tags:
  - project
  - classification
  - interview
status: active
created: 2026-04-24
---

# LCS 06 第二阶段 真假结节分类

关联：[[LCS 05 第一阶段 分割模块]] | [[LCS 07 第三阶段 良恶性分类]] | [[LCS 08 整体推理与评估]]

## 这一阶段做什么

分割阶段会产生很多误报，所以第二阶段的作用是：

把可疑区域再裁成 3D 小块，用 3D CNN 判断它到底是不是真结节。

## `dsets.py`

### `getCandidateInfoList(requireOnDisk_bool=True)`

作用：读取结节候选信息。

### `class CT`

#### `__init__(self, series_uid)`

作用：读取完整 CT

#### `getRawCandidate(self, center_xyz, width_irc)`

输出：

1. `ct_chunk`
2. `center_irc`

### `getCt(series_uid)`

作用：带缓存获取 CT 对象。

### `getCtRawCandidate(series_uid, center_xyz, width_irc)`

作用：带磁盘缓存获取候选块。

### `getCtAugmentedCandidate(...)`

作用：对 3D 候选块做增强。

### `class LunaDataset`

#### `__getitem__(self, ndx)`

输出：

1. `candidate_t`
2. `pos_t`
3. `series_uid`
4. `center_irc`

关键点：

1. 固定裁剪尺寸约为 `32 x 48 x 48`
2. 支持正负样本平衡采样
3. 支持 3D 数据增强

## `model.py`

### `class LunaModel`

作用：3D CNN 二分类模型。

#### `forward(self, input_batch)`

输出：

1. `linear_output`
2. `softmax_output`

### `class LunaBlock`

作用：两个 3D 卷积加池化的特征块。

## `training.py`

### `class LunaTrainingApp`

#### `computeBatchLoss(...)`

作用：计算交叉熵损失并记录预测概率。

#### `logMetrics(...)`

作用：统计：

1. loss
2. accuracy
3. precision
4. recall
5. F1

#### `saveModel(...)`

作用：保存 checkpoint 和 best model。

## `prepcache.py`

作用：训练前预热缓存。

## 这一阶段面试怎么讲

你可以这样讲：

第二阶段本质上是一个 3D 二分类任务。系统会以候选点为中心，从原始 CT 中裁出固定大小的 3D 体块，然后送进 3D CNN。输出是“不是结节”和“是结节”两个类别的概率。之所以要单独做这一层，是因为分割为了提高召回，会引入很多假阳性，所以必须用二次分类把误报筛掉。

