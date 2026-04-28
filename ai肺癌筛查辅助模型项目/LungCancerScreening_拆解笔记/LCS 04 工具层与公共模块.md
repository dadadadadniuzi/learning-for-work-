---
title: LCS 04 工具层与公共模块
aliases:
  - LungCancerScreening 工具层
tags:
  - project
  - util
  - interview
status: active
created: 2026-04-24
---

# LCS 04 工具层与公共模块

关联：[[LCS 02 目录与架构]] | [[LCS 03 从零实现流程]] | [[LCS 05 第一阶段 分割模块]] | [[LCS 06 第二阶段 真假结节分类]] | [[LCS 07 第三阶段 良恶性分类]]

## `util/util.py`

### `voxelCoord2patientCoord(coord_irc, origin_xyz, vxSize_xyz, direction_a)`

作用：把体素坐标转成病人坐标。

输入：

1. `coord_irc`
2. `origin_xyz`
3. `vxSize_xyz`
4. `direction_a`

输出：

1. `PatientCoordTuple`

### `patientCoord2voxelCoord(coord_xyz, origin_xyz, vxSize_xyz, direction_a)`

作用：把病人坐标转成体素坐标。

输入：

1. `coord_xyz`
2. `origin_xyz`
3. `vxSize_xyz`
4. `direction_a`

输出：

1. `VoxelCoordTuple`

### `importstr(module_str, from_=None)`

作用：动态导入模块或对象。

### `enumerateWithEstimate(iter, desc_str, start_ndx=0, print_ndx=4, backoff=None, iter_len=None)`

作用：在训练和缓存时打印进度估计。

## `util/disk.py`

### `class GzipDisk(Disk)`

作用：在缓存写入时压缩，在读取时解压。

#### `store(self, value, read, key=None)`

输出：压缩后写入缓存

#### `fetch(self, mode, filename, value, read)`

输出：解压后的缓存内容

### `getCache(scope_str)`

作用：创建不同作用域的缓存，比如：

1. `raw_data`
2. `seg_data`
3. `tumor_data`

## `util/unet.py`

### `class UNet`

作用：2D U-Net 主体。

#### `forward(self, x)`

输出：分割 logits

### `class UNetConvBlock`

作用：双卷积块。

### `class UNetUpBlock`

作用：上采样块。

#### `center_crop(self, layer, target_size)`

作用：裁剪跳连特征图

#### `forward(self, x, bridge)`

作用：特征融合

## 工具层的面试说法

你可以这样讲：

这个项目最底层先解决了几个基础问题，包括医学坐标和体素坐标之间的转换、样本缓存、以及分割网络的公共实现。这样上层训练和推理模块就可以专注于业务逻辑，而不是重复处理底层细节。

