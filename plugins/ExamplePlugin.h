#pragma once

#include "../IPlugin.h"

// 示例插件类
class ExamplePlugin : public IPlugin {
public:
    ExamplePlugin();
    ~ExamplePlugin() override;
    
    std::string name() const override;
    void update() override;
    
private:
    int counter_;
};
