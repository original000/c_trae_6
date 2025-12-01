#include "ExamplePlugin.h"
#include <iostream>
#include <chrono>
#include <thread>

ExamplePlugin::ExamplePlugin() : counter_(0) {
    std::cout << "ExamplePlugin created!" << std::endl;
}

ExamplePlugin::~ExamplePlugin() {
    std::cout << "ExamplePlugin destroyed!" << std::endl;
}

std::string ExamplePlugin::name() const {
    return "ExamplePlugin";
}

void ExamplePlugin::update() {
    counter_++;    std::cout << "ExamplePlugin update() [HOT RELOADED VERSION 3] called! Counter: " << counter_ << std::endl;
    
    // 模拟一些工作
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// 插件入口函数: 创建插件实例
extern "C" __declspec(dllexport) IPlugin* CreatePlugin() {
    return new ExamplePlugin();
}

// 插件入口函数: 销毁插件实例
extern "C" __declspec(dllexport) void DestroyPlugin(IPlugin* plugin) {
    delete plugin;
}
