---
title: LCS 05 第一阶段 分割模块
aliases:
  - LungCancerScreening 分割模块
tags:
  - project
  - segmentation
  - interview
status: active
created: 2026-04-24
---

# LCS 05 第一阶段 分割模块

关联：[[LCS 04 工具层与公共模块]] | [[LCS 06 第二阶段 真假结节分类]] | [[LCS 08 整体推理与评估]]

## 这一阶段做什么

目标：尽量把所有可疑结节区域先找出来。

这一阶段宁可多报一点，也不能轻易漏掉。

## `segmentDsets.py`

### `getCandidateInfoList(requireOnDisk_bool=True)`

作用：读取带恶性标记的标注和非结节候选。

### `getCandidateInfoDict(requireOnDisk_bool=True)`

作用：按 `series_uid` 分组管理候选信息。

### `class Ct`

作用：读取一个 CT，并生成结节正样本 mask。

#### `buildAnnotationMask(self, positiveInfo_list, threshold_hu=-700)`

作用：根据标注中心扩张结节区域，生成 mask。

输出：

1. `mask_a`

#### `getRawCandidate(self, center_xyz, width_irc)`

输出：

1. `ct_chunk`
2. `pos_chunk`
3. `center_irc`

### `class Luna2dSegmentationDataset`

作用：给分割模型提供样本。

#### `getitem_fullSlice(self, series_uid, slice_ndx)`

输出：

1. `ct_t`
2. `pos_t`
3. `series_uid`
4. `slice_ndx`

这里的关键点是输入为 7 通道切片，不是单张切片。

## `segmentModel.py`

### `class UNetWrapper`

作用：对通用 U-Net 做项目化封装。

#### `forward(self, input_batch)`

输出：

1. `fn_output`

### `class SegmentationAugmentation`

作用：对图像和 mask 同时做仿射增强。

#### `forward(self, input_g, label_g)`

输出：

1. `augmented_input_g`
2. `augmented_label_g`

## `segmentTraining.py`

### `class SegmentationTrainingApp`

#### `initModel(self)`

输出：

1. `segmentation_model`
2. `augmentation_model`

#### `computeBatchLoss(...)`

作用：计算 Dice loss，并记录 TP、FN、FP、TN。

输出：

1. 总 batch 损失

#### `diceLoss(self, prediction_g, label_g, epsilon=1)`

输出：

1. 每样本 Dice loss

#### `logMetrics(self, epoch_ndx, mode_str, metrics_t)`

作用：统计分割 recall、precision、F1 等。

#### `saveModel(self, type_str, epoch_ndx, isBest=False)`

作用：保存分割模型

## `prep_seg_cache.py`

### `class LunaPrepCacheApp`

#### `main(self)`

作用：预热缓存

## 这一阶段面试怎么讲

你可以这样讲：

第一阶段用的是 2D UNet，但是输入不是单张切片，而是当前切片前后各 3 张共 7 张切片拼成的多通道输入。这样模型能看到上下文信息。训练时主要用 Dice Loss，并对假阴性做额外加权，因为在肺结节筛查里漏检代价更高。分割完以后，再通过连通域分析把 mask 转成候选结节中心，交给下一阶段去判断真假。

