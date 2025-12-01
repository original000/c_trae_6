#include <iostream>
#include <chrono>
#include <thread>
#include "PluginReloader.h"

int main() {
    std::cout << "Starting Plugin Reloader Demo..." << std::endl;
    
    // 创建插件重载器
    PluginReloader reloader;
    
    // 获取当前源文件的路径
    std::string currentFile(__FILE__);
    
    // 提取项目根目录的路径（假设main.cpp在项目根目录下）
    size_t lastSlash = currentFile.find_last_of("\\/");
    std::string projectRoot = currentFile.substr(0, lastSlash);
    
    // 构造插件目录的路径
    std::string pluginDir = projectRoot + "\\plugins";
    
    // 初始化插件重载器
    if (!reloader.init(pluginDir)) {
        std::cerr << "Failed to initialize PluginReloader. Exiting..." << std::endl;
        return 1;
    }
    
    std::cout << "Press Ctrl+C to exit." << std::endl;
    
    try {
        // 主循环，每秒更新一次
        while (true) {
            // 检查并更新插件
            reloader.update();
            
            // 获取所有插件实例并调用update()
            auto plugins = reloader.getPlugins();
            for (auto plugin : plugins) {
                std::cout << "Calling update() on plugin: " << plugin->name() << std::endl;
                plugin->update();
            }
            
            // 等待1秒
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in main loop: " << e.what() << std::endl;
    }
    
    std::cout << "Exiting..." << std::endl;
    return 0;
}
