#pragma once

#include <string>

// 插件接口类
class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    // 插件名称
    virtual std::string name() const = 0;
    
    // 更新函数，每秒被main调用
    virtual void update() = 0;
};

// 插件创建函数类型定义
typedef IPlugin* (*CreatePluginFunc)();

// 插件销毁函数类型定义
typedef void (*DestroyPluginFunc)(IPlugin*);
