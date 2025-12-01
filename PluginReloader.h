#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <windows.h>
#include "IPlugin.h"

// 函数信息结构体
struct FunctionInfo {
    void* newAddress;  // 新版本函数地址
    bool isUpdating;   // 是否正在更新
};

// 插件信息结构体
struct PluginInfo {
    HMODULE module;                     // 模块句柄
    CreatePluginFunc createFunc;       // 创建函数
    DestroyPluginFunc destroyFunc;     // 销毁函数
    IPlugin* instance;                  // 插件实例
    std::chrono::system_clock::time_point lastModified;  // 最后修改时间
    int version;                        // 版本号
    
    // 函数指针缓存: 旧函数地址 -> 新函数地址
    std::unordered_map<void*, FunctionInfo> functionCache;
    
    PluginInfo() : module(nullptr), createFunc(nullptr), destroyFunc(nullptr), 
                   instance(nullptr), version(0) {}
};

class PluginReloader {
public:
    PluginReloader();
    ~PluginReloader();
    
    // 初始化插件重载器
    bool init(const std::string& pluginDir = "plugins");
    
    // 检查并更新插件
    void update();
    
    // 获取所有插件实例
    std::vector<IPlugin*> getPlugins() const;
    
    // 替换函数指针（供插件内部使用）
    template<typename FuncType>
    static FuncType replaceFunction(FuncType oldFunc) {
        return reinterpret_cast<FuncType>(replaceFunctionInternal(reinterpret_cast<void*>(oldFunc)));
    }
    
private:
    // 内部函数替换实现
    static void* replaceFunctionInternal(void* oldFunc);
    
    // 加载插件
    bool loadPlugin(const std::string& pluginPath);
    
    // 卸载插件
    bool unloadPlugin(const std::string& pluginName);
    
    // 检查文件是否被修改
    bool isFileModified(const std::string& filePath, const std::chrono::system_clock::time_point& lastModified) const;
    
    // 遍历目录下的所有DLL文件
    std::vector<std::string> getDllFiles(const std::string& directory) const;
    
    // 挂钩正在执行的函数的返回地址
    void hookReturnAddresses(void* oldFunc, void* newFunc);
    
private:
    std::string pluginDirectory_;  // 插件目录
    std::unordered_map<std::string, PluginInfo> plugins_;  // 插件名称 -> 插件信息
    
    // 全局函数缓存: 旧函数地址 -> 插件名称 + 新函数地址
    static std::unordered_map<void*, std::pair<std::string, FunctionInfo>> globalFunctionCache_;
};
