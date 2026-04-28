---
title: LCS 07 第三阶段 良恶性分类
aliases:
  - LungCancerScreening 良恶性分类
tags:
  - project
  - malignancy
  - interview
status: active
created: 2026-04-24
---

# LCS 07 第三阶段 良恶性分类

关联：[[LCS 06 第二阶段 真假结节分类]] | [[LCS 08 整体推理与评估]] | [[LCS 10 面试亮点 不足与表达]]

## 这一阶段做什么

在已经确认是真结节之后，再进一步判断它更偏良性还是恶性。

## `TumorDatasets.py`

### `getCandidateInfoList(requireOnDisk_bool=True)`

作用：读取带恶性标签的候选信息。

### `getCandidateInfoDict(requireOnDisk_bool=True)`

作用：按 `series_uid` 分组。

### `class Ct`

#### `getRawCandidate(self, center_xyz, width_irc)`

输出：

1. `ct_chunk`
2. `center_irc`

### `getCtAugmentedCandidate(...)`

作用：对 3D 候选样本做增强。

### `class LunaDataset`

作用：良恶性任务基础数据集。

### `class MalignantLunaDataset`

作用：良恶性任务专用采样逻辑。

关键点：

1. 区分 `ben_list`
2. 区分 `mal_list`
3. 会做一定的平衡采样

## `TumorModel.py`

### `augment3d(inp)`

作用：做 3D 翻转、偏移、旋转增强。

### `class LunaModel`

作用：良恶性任务的 3D CNN 主体。

#### `forward(self, input_batch)`

输出：

1. `linear_output`
2. `head_activation`

## `TumorTraining.py`

### `class ClassificationTrainingApp`

#### `initModel(self)`

这里最关键。

作用：

1. 动态加载模型类
2. 加载已有结节分类模型参数
3. 按 `finetune-depth` 冻结前面层
4. 只微调后面几层

#### `computeBatchLoss(...)`

作用：计算交叉熵，并记录预测类别和预测概率。

#### `logMetrics(...)`

作用：统计：

1. accuracy
2. precision
3. recall
4. F1
5. AUC
6. ROC

#### `saveModel(...)`

作用：保存良恶性模型。

## `TumorPrepCache.py`

作用：为良恶性训练预热缓存。

## 这一阶段面试怎么讲

你可以这样讲：

第三阶段和第二阶段结构上很像，也是在 3D 候选块上做分类，只不过标签从“是否结节”变成了“良性还是恶性”。这个项目没有完全从零训练，而是基于已有的结节分类模型做微调，这样可以复用已经学到的 3D 结构特征，同时减少小样本任务直接训练的不稳定性。评估上除了 F1，还会看 AUC，因为这是更典型的风险评分任务。

