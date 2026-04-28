---
title: LCS 08 整体推理与评估
aliases:
  - LungCancerScreening 联合评估
tags:
  - project
  - inference
  - evaluation
status: active
created: 2026-04-24
---

# LCS 08 整体推理与评估

关联：[[LCS 05 第一阶段 分割模块]] | [[LCS 06 第二阶段 真假结节分类]] | [[LCS 07 第三阶段 良恶性分类]] | [[LCS 09 Web 后端与前端]]

## `model_evel.py`

> [!note]
> 文件名看起来像 `eval` 的拼写错误，这本身也是你面试里可以顺手指出的一个代码细节。

## 这个文件为什么重要

因为它评估的不是某一个模型，而是整条三阶段 pipeline。

## `print_confusion(label, confusions, do_mal)`

作用：打印混淆矩阵。

## `match_and_score(detections, truth, threshold=0.5)`

作用：把预测结果和真实结节做空间匹配，再生成混淆矩阵。

关键思想：

1. 不是简单按类别比较
2. 而是先比较预测中心和真实结节中心的距离
3. 距离足够近才算匹配成功

## `class NoduleAnalysisApp`

### `initModels(self)`

作用：加载：

1. 分割模型
2. 真假结节分类模型
3. 良恶性模型

### `initSegmentationDl(self, series_uid)`

作用：为某个 CT 构建分割 DataLoader。

### `initClassificationDl(self, candidateInfo_list)`

作用：为候选列表构建分类 DataLoader。

### `segmentCt(self, ct, series_uid)`

作用：对完整 CT 做逐切片分割。

输出：

1. `mask_a`

### `groupSegmentationOutput(self, series_uid, ct, clean_a)`

作用：对分割结果做连通域分析。

输出：

1. `candidateInfo_list`

### `classifyCandidates(self, ct, candidateInfo_list)`

作用：对候选区域做真假结节和恶性分类。

输出：

1. `classifications_list`

### `main(self)`

作用：跑整条评估流程。

## 面试时可以怎么讲

这个脚本说明作者不仅训练了三个模型，还认真考虑了如何评估整套系统。因为单个模型指标高，不代表组合起来一定好，所以这里会把分割、候选提取、分类和恶性分析全部串起来，再和真实标注做空间匹配，最终输出系统级别的混淆矩阵。

