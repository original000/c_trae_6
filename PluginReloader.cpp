#include "PluginReloader.h"
#include <iostream>
#include <psapi.h>
#include <tlhelp32.h>
#include <vector>
#include <windows.h>
#include <string>
#include <chrono>
#include <ctime>

// 初始化全局函数缓存
std::unordered_map<void*, std::pair<std::string, FunctionInfo>> PluginReloader::globalFunctionCache_;

PluginReloader::PluginReloader() {
}

PluginReloader::~PluginReloader() {
    // 卸载所有插件
    for (auto& pair : plugins_) {
        unloadPlugin(pair.first);
    }
    plugins_.clear();
}

bool PluginReloader::init(const std::string& pluginDir) {
    pluginDirectory_ = pluginDir;
    
    // 检查插件目录是否存在
    if (!CreateDirectoryA(pluginDirectory_.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        std::cout << "Plugin directory " << pluginDirectory_ << " does not exist. Creating it..." << std::endl;
        std::cerr << "Failed to create plugin directory. Error: " << GetLastError() << std::endl;
        return false;
    }
    
    // 加载初始插件
    auto dllFiles = getDllFiles(pluginDirectory_);
    for (const auto& dllFile : dllFiles) {
        loadPlugin(dllFile);
    }
    
    std::cout << "PluginReloader initialized successfully." << std::endl;
    return true;
}

void PluginReloader::update() {
    // 检查插件目录中的所有DLL文件
    auto dllFiles = getDllFiles(pluginDirectory_);
    
    // 检查已加载的插件是否被修改
    for (auto it = plugins_.begin(); it != plugins_.end(); ) {
        const std::string& pluginName = it->first;
        PluginInfo& pluginInfo = it->second;
        
        // 检查文件是否存在
        bool fileExists = false;
        for (const auto& dllFile : dllFiles) {
            // 从文件路径中提取插件名称（不包含扩展名）
            std::string currentPluginName;
            size_t lastSlash = dllFile.find_last_of("\\/");
            if (lastSlash != std::string::npos) {
                currentPluginName = dllFile.substr(lastSlash + 1);
            } else {
                currentPluginName = dllFile;
            }
            
            size_t lastDot = currentPluginName.find_last_of(".");
            if (lastDot != std::string::npos) {
                currentPluginName = currentPluginName.substr(0, lastDot);
            }
            
            if (currentPluginName == pluginName) {
                fileExists = true;
                
                // 检查文件是否被修改
                if (isFileModified(dllFile, pluginInfo.lastModified)) {
                    std::cout << "Plugin " << pluginName << " has been modified. Reloading..." << std::endl;
                    
                    // 卸载旧插件
                    unloadPlugin(pluginName);
                    
                    // 加载新插件
                    loadPlugin(dllFile);
                }
                break;
            }
        }
        
        // 如果文件不存在了，卸载插件
        if (!fileExists) {
            std::cout << "Plugin " << pluginName << " has been removed. Unloading..." << std::endl;
            unloadPlugin(pluginName);
            it = plugins_.erase(it);
        } else {
            ++it;
        }
    }
    
    // 加载新添加的插件
    for (const auto& dllFile : dllFiles) {
        // 从文件路径中提取插件名称（不包含扩展名）
        std::string pluginName;
        size_t lastSlash = dllFile.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            pluginName = dllFile.substr(lastSlash + 1);
        } else {
            pluginName = dllFile;
        }
        
        size_t lastDot = pluginName.find_last_of(".");
        if (lastDot != std::string::npos) {
            pluginName = pluginName.substr(0, lastDot);
        }
        
        if (plugins_.find(pluginName) == plugins_.end()) {
            std::cout << "New plugin " << pluginName << " detected. Loading..." << std::endl;
            loadPlugin(dllFile);
        }
    }
}

std::vector<IPlugin*> PluginReloader::getPlugins() const {
    std::vector<IPlugin*> result;
    for (const auto& pair : plugins_) {
        if (pair.second.instance != nullptr) {
            result.push_back(pair.second.instance);
        }
    }
    return result;
}

void* PluginReloader::replaceFunctionInternal(void* oldFunc) {
    auto it = globalFunctionCache_.find(oldFunc);
    if (it != globalFunctionCache_.end()) {
        FunctionInfo& funcInfo = it->second.second;
        if (funcInfo.newAddress != nullptr) {
            return funcInfo.newAddress;
        }
    }
    return oldFunc;
}

bool PluginReloader::loadPlugin(const std::string& pluginPath) {
    // 从文件路径中提取插件名称（不包含扩展名）
    std::string pluginName;
    size_t lastSlash = pluginPath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        pluginName = pluginPath.substr(lastSlash + 1);
    } else {
        pluginName = pluginPath;
    }
    
    size_t lastDot = pluginName.find_last_of(".");
    if (lastDot != std::string::npos) {
        pluginName = pluginName.substr(0, lastDot);
    }
    
    // 检查插件是否已经加载
    if (plugins_.find(pluginName) != plugins_.end()) {
        std::cout << "Plugin " << pluginName << " is already loaded." << std::endl;
        return false;
    }
    
    PluginInfo pluginInfo;
    
    // 加载DLL模块
    pluginInfo.module = LoadLibraryA(pluginPath.c_str());
    if (!pluginInfo.module) {
        std::cerr << "Failed to load plugin " << pluginPath << ". Error: " << GetLastError() << std::endl;
        return false;
    }
    
    // 获取插件创建函数
    pluginInfo.createFunc = reinterpret_cast<CreatePluginFunc>(GetProcAddress(pluginInfo.module, "CreatePlugin"));
    if (!pluginInfo.createFunc) {
        std::cerr << "Failed to find CreatePlugin function in " << pluginPath << ". Error: " << GetLastError() << std::endl;
        FreeLibrary(pluginInfo.module);
        return false;
    }
    
    // 获取插件销毁函数
    pluginInfo.destroyFunc = reinterpret_cast<DestroyPluginFunc>(GetProcAddress(pluginInfo.module, "DestroyPlugin"));
    if (!pluginInfo.destroyFunc) {
        std::cerr << "Failed to find DestroyPlugin function in " << pluginPath << ". Error: " << GetLastError() << std::endl;
        FreeLibrary(pluginInfo.module);
        return false;
    }
    
    // 创建插件实例
    pluginInfo.instance = pluginInfo.createFunc();
    if (!pluginInfo.instance) {
        std::cerr << "Failed to create plugin instance for " << pluginPath << std::endl;
        FreeLibrary(pluginInfo.module);
        return false;
    }
    
    // 获取文件最后修改时间
    HANDLE fileHandle = CreateFileA(pluginPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fileHandle != INVALID_HANDLE_VALUE) {
        FILETIME lastWriteTime;
        if (GetFileTime(fileHandle, nullptr, nullptr, &lastWriteTime)) {
            SYSTEMTIME systemTime;
            FileTimeToSystemTime(&lastWriteTime, &systemTime);
            
            // 转换为std::chrono::system_clock::time_point
            std::tm tmTime = {
                systemTime.wSecond,
                systemTime.wMinute,
                systemTime.wHour,
                systemTime.wDay,
                systemTime.wMonth - 1,
                systemTime.wYear - 1900,
                0,  // tm_wday (unused)
                0,  // tm_yday (unused)
                0   // tm_isdst (unused)
            };
            
            std::time_t timeT = std::mktime(&tmTime);
            pluginInfo.lastModified = std::chrono::system_clock::from_time_t(timeT);
        }
        CloseHandle(fileHandle);
    }
    
    // 增加版本号
    pluginInfo.version++;
    
    // 缓存插件信息
    plugins_[pluginName] = pluginInfo;
    
    std::cout << "Plugin " << pluginName << " loaded successfully. Version: " << pluginInfo.version << std::endl;
    std::cout << "plugins_ size: " << plugins_.size() << std::endl;
    
    // 检查插件实例是否为空
    if (plugins_[pluginName].instance == nullptr) {
        std::cout << "plugins_[pluginName].instance is nullptr" << std::endl;
    } else {
        std::cout << "plugins_[pluginName].instance is not nullptr" << std::endl;
    }
    
    return true;
}

bool PluginReloader::unloadPlugin(const std::string& pluginName) {
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        std::cout << "Plugin " << pluginName << " is not loaded." << std::endl;
        return false;
    }
    
    PluginInfo& pluginInfo = it->second;
    
    // 销毁插件实例
    if (pluginInfo.instance && pluginInfo.destroyFunc) {
        pluginInfo.destroyFunc(pluginInfo.instance);
        pluginInfo.instance = nullptr;
    }
    
    // 卸载DLL模块
    if (pluginInfo.module) {
        // 从全局函数缓存中移除该插件的所有函数
        for (auto globalIt = globalFunctionCache_.begin(); globalIt != globalFunctionCache_.end(); ) {
            if (globalIt->second.first == pluginName) {
                globalIt = globalFunctionCache_.erase(globalIt);
            } else {
                ++globalIt;
            }
        }
        
        FreeLibrary(pluginInfo.module);
        pluginInfo.module = nullptr;
    }
    
    std::cout << "Plugin " << pluginName << " unloaded successfully." << std::endl;
    return true;
}

bool PluginReloader::isFileModified(const std::string& filePath, const std::chrono::system_clock::time_point& lastModified) const {
    HANDLE fileHandle = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    FILETIME lastWriteTime;
    if (!GetFileTime(fileHandle, nullptr, nullptr, &lastWriteTime)) {
        CloseHandle(fileHandle);
        return false;
    }
    
    CloseHandle(fileHandle);
    
    // 转换为std::chrono::system_clock::time_point
    SYSTEMTIME systemTime;
    FileTimeToSystemTime(&lastWriteTime, &systemTime);
    
    std::tm tmTime = {
        systemTime.wSecond,
        systemTime.wMinute,
        systemTime.wHour,
        systemTime.wDay,
        systemTime.wMonth - 1,
        systemTime.wYear - 1900,
        0,  // tm_wday (unused)
        0,  // tm_yday (unused)
        0   // tm_isdst (unused)
    };
    
    std::time_t timeT = std::mktime(&tmTime);
    std::chrono::system_clock::time_point currentLastModified = std::chrono::system_clock::from_time_t(timeT);
    
    // 检查是否有修改
    return currentLastModified > lastModified;
}

std::vector<std::string> PluginReloader::getDllFiles(const std::string& directory) const {
    std::vector<std::string> dllFiles;
    
    std::string searchPath = directory + "\\*.dll";
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (findHandle != INVALID_HANDLE_VALUE) {
        do {
            // 跳过目录
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string dllPath = directory + "\\" + findData.cFileName;
                dllFiles.push_back(dllPath);
            }
        } while (FindNextFileA(findHandle, &findData));
        
        FindClose(findHandle);
    }
    
    return dllFiles;
}

void PluginReloader::hookReturnAddresses(void* oldFunc, void* newFunc) {
    // 这个函数需要实现对正在执行的函数的返回地址的修改
    // 由于实现起来比较复杂且需要处理很多边缘情况，这里只提供一个简化的实现思路
    // 实际生产环境中需要更完善的实现
    
    std::cout << "Hooking return addresses for function " << oldFunc << " to " << newFunc << std::endl;
    
    // 1. 获取当前进程的所有线程
    // 2. 对每个线程，获取其上下文
    // 3. 检查栈中的返回地址是否指向旧函数
    // 4. 如果是，将返回地址修改为新函数的地址
    // 5. 设置线程的上下文
}
