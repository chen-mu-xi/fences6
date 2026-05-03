#pragma once

#include "LayoutEngine.h"
#include "RuleEngine.h"
#include "ConfigManager.h"
#include "HotkeyManager.h"

namespace Fences6
{
    /// <summary>
    /// 核心初始化类
    /// 负责整个 Fences6 核心库的初始化和生命周期管理
    /// </summary>
    public ref class FenceCore
    {
    public:
        /// <summary>
        /// 初始化核心库
        /// </summary>
        static void Initialize()
        {
            if (s_initialized)
                return;

            // 初始化布局引擎
            Engine = gcnew LayoutEngine();
            
            // 初始化规则引擎
            RuleEngine = gcnew Fences6::RuleEngine();
            
            // 初始化配置管理器
            Config = gcnew ConfigManager();
            
            // 初始化热键管理器
            Hotkeys = gcnew HotkeyManager();
            
            s_initialized = true;
        }

        /// <summary>
        /// 关闭核心库，释放所有资源
        /// </summary>
        static void Shutdown()
        {
            if (!s_initialized)
                return;

            // 清理热键
            if (Hotkeys != nullptr)
            {
                Hotkeys->UnregisterAll();
            }

            // 保存配置
            if (Config != nullptr)
            {
                Config->Save();
            }

            // 清理引擎
            Engine = nullptr;
            RuleEngine = nullptr;
            Config = nullptr;
            Hotkeys = nullptr;
            
            s_initialized = false;
        }

        /// <summary>
        /// 布局引擎实例
        /// </summary>
        static property LayoutEngine^ Engine;

        /// <summary>
        /// 规则引擎实例
        /// </summary>
        static property RuleEngine^ RuleEngine;

        /// <summary>
        /// 配置管理器实例
        /// </summary>
        static property ConfigManager^ Config;

        /// <summary>
        /// 热键管理器实例
        /// </summary>
        static property HotkeyManager^ Hotkeys;

    private:
        static bool s_initialized = false;
    };
}
