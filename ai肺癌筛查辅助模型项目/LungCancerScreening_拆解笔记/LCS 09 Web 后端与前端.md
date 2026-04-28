---
title: LCS 09 Web 后端与前端
aliases:
  - LungCancerScreening Web 系统
tags:
  - project
  - flask
  - react
  - interview
status: active
created: 2026-04-24
---

# LCS 09 Web 后端与前端

关联：[[LCS 02 目录与架构]] | [[LCS 08 整体推理与评估]] | [[LCS 10 面试亮点 不足与表达]]

## 后端：`web_app/app.py`

### `class Diagnosis(db.Model)`

作用：历史诊断记录表。

主要字段：

1. 文件名
2. 文件路径
3. 时间戳
4. 总体诊断
5. 置信度
6. 完整 JSON 结果

### `to_dict(self)`

作用：把数据库记录转成 API 返回格式。

## `class TumorPredictionSystem`

作用：封装完整三阶段推理流程。

### `load_models(self)`

作用：加载三个模型。

### `process_ct_files(self, mhd_data, raw_data, mhd_filename, raw_filename)`

作用：后端主推理入口。

流程：

1. 临时落盘上传文件
2. 读取 CT
3. 调用 `_segment_ct`
4. 调用 `_group_segmentation_output`
5. 调用 `_classify_candidates`
6. 汇总 JSON 结果

### `_segment_ct(self, ct_hu_a)`

作用：逐切片运行分割模型。

### `_group_segmentation_output(...)`

作用：连通域分析，得到候选结节中心。

### `_get_ct_chunk(...)`

作用：从 CT 中裁出 3D 候选块。

### `_classify_candidates(...)`

作用：对候选块做真假结节和良恶性分析。

## 主要 API

### `GET /api/health`

作用：返回系统健康状态。

### `GET /api/models/status`

作用：返回模型加载状态。

### `POST /api/upload`

作用：上传 CT 并触发完整推理。

### `GET /api/predictions`

作用：返回历史记录。

### `GET /api/statistics`

作用：返回统计信息。

### `POST /api/chat`

作用：基于报告做大模型问答。

### `GET /api/ct-slice/<series_uid>/<slice_ndx>`

作用：动态生成切片 PNG，并绘制结节框。

## 启动脚本：`web_app/start_system.py`

### `check_node_installed()`

作用：检查 Node.js

### `check_npm_installed()`

作用：检查 npm

### `install_frontend_dependencies(npm_path='npm')`

作用：安装前端依赖

### `main()`

作用：一键启动前后端

## 前端：`web_app/frontend/src`

### `App.js`

作用：路由入口。

### `components/Layout.js`

作用：页面整体框架。

### `components/CTViewer.js`

作用：CT 切片查看器。

### `pages/Upload.js`

作用：上传和展示单次诊断结果。

### `pages/Dashboard.js`

作用：仪表盘与统计图表。

### `pages/History.js`

作用：查看历史诊断记录和详情。

### `pages/ModelStatus.js`

作用：查看模型加载状态和性能基准。

### `pages/Chat.js`

作用：带报告上下文的 AI 问答页面。

## 面试时可以怎么讲

这个项目的一个亮点是，它不是只停留在训练脚本层面，而是做成了一个完整系统。后端基于 Flask，负责模型加载、文件处理、推理和历史记录持久化；前端基于 React，负责上传、结果展示、切片查看和问答交互。这样整个项目从研究型代码走到了可演示的产品原型。

