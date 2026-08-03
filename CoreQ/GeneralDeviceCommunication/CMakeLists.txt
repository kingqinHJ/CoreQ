cmake_minimum_required(VERSION 3.16)

# GeneralDeviceCommunication 模块

# cl_module: 基础工具库（Utils, QML辅助, HttpManager, 日志等）
add_subdirectory(cl_module)

# cl_device: 现有设备通信库（后续将重构替换）
add_subdirectory(cl_device)

# 后续新的设备通信/管理抽象可以在此添加新的子目录
# add_subdirectory(new_device_lib)
