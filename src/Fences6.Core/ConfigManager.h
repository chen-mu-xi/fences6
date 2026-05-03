#pragma once
#include <Windows.h>

namespace Fences6
{
    /// <summary>
    /// 配置项
    /// </summary>
    [System::Serializable]
    public ref class ConfigItem
    {
    public:
        property System::String^ Key;
        property System::String^ Value;
        property System::String^ Type;
        property System::DateTime LastModified;

        ConfigItem() 
        {
            LastModified = System::DateTime::Now;
        }

        ConfigItem(System::String^ key, System::String^ value)
        {
            Key = key;
            Value = value;
            Type = "string";
            LastModified = System::DateTime::Now;
        }

        // 获取整数值
        property int IntValue
        {
            int get()
            {
                int result = 0;
                int::TryParse(Value, result);
                return result;
            }
        }

        // 获取布尔值
        property bool BoolValue
        {
            bool get()
            {
                bool result = false;
                bool::TryParse(Value, result);
                return result;
            }
        }

        // 获取双精度值
        property double DoubleValue
        {
            double get()
            {
                double result = 0.0;
                double::TryParse(Value, result);
                return result;
            }
        }
    };

    /// <summary>
    /// 应用配置类
    /// </summary>
    [System::Serializable]
    public ref class AppSettings
    {
    public:
        // 通用设置
        property bool StartWithWindows;
        property bool StartMinimized;
        property bool MinimizeToTray;
        property bool ShowNotifications;

        // 外观设置
        property System::String^ Theme;
        property System::String^ Language;
        property int IconSize;
        property bool ShowLabels;

        // 布局设置
        property int GridSize;
        property int GroupPadding;
        property bool AutoArrange;

        // 行为设置
        property bool EnableRules;
        property bool MonitorDesktop;
        property bool QuickAccessOnHover;
        property int QuickAccessDelay;

        // 热键设置
        property System::String^ ToggleHotkey;
        property System::String^ RefreshHotkey;

        AppSettings()
        {
            // 默认值
            StartWithWindows = false;
            StartMinimized = false;
            MinimizeToTray = true;
            ShowNotifications = true;
            Theme = "Light";
            Language = "en-US";
            IconSize = 48;
            ShowLabels = true;
            GridSize = 10;
            GroupPadding = 10;
            AutoArrange = true;
            EnableRules = true;
            MonitorDesktop = true;
            QuickAccessOnHover = false;
            QuickAccessDelay = 300;
            ToggleHotkey = "Win+Shift+F";
            RefreshHotkey = "Win+Shift+R";
        }
    };

    /// <summary>
    /// 配置管理器
    /// 负责应用程序配置的保存和加载
    /// </summary>
    public ref class ConfigManager
    {
    public:
        /// <summary>
        /// 构造函数
        /// </summary>
        ConfigManager();

        /// <summary>
        /// 加载配置
        /// </summary>
        void Load();

        /// <summary>
        /// 保存配置
        /// </summary>
        void Save();

        /// <summary>
        /// 保存配置到指定路径
        /// </summary>
        void SaveTo(System::String^ filePath);

        /// <summary>
        /// 从指定路径加载配置
        /// </summary>
        void LoadFrom(System::String^ filePath);

        /// <summary>
        /// 获取配置项
        /// </summary>
        ConfigItem^ GetConfig(System::String^ key);

        /// <summary>
        /// 设置配置项
        /// </summary>
        void SetConfig(System::String^ key, System::String^ value);

        /// <summary>
        /// 获取应用设置
        /// </summary>
        property AppSettings^ Settings
        {
            AppSettings^ get() { return m_settings; }
        }

        /// <summary>
        /// 获取配置目录
        /// </summary>
        property System::String^ ConfigPath
        {
            System::String^ get() { return m_configPath; }
        }

        /// <summary>
        /// 重置为默认配置
        /// </summary>
        void ResetToDefaults();

        /// <summary>
        /// 导出配置
        /// </summary>
        void Export(System::String^ filePath);

        /// <summary>
        /// 导入配置
        /// </summary>
        void Import(System::String^ filePath);

        /// <summary>
        /// 监听配置变更事件
        /// </summary>
        event System::EventHandler<System::String^>^ ConfigChanged;

    protected:
        /// <summary>
        /// 触发配置变更事件
        /// </summary>
        void OnConfigChanged(System::String^ key);

    private:
        System::String^ m_configPath;
        System::Collections::Generic::Dictionary<System::String^, ConfigItem^>^ m_configItems;
        AppSettings^ m_settings;
        System::Object^ m_syncRoot;
        bool m_autoSave;
        System::Threading::Timer^ m_autoSaveTimer;

        void LoadDefaultConfig();
        System::String^ GetDefaultConfigPath();
        void StartAutoSave();
        void StopAutoSave();
        void AutoSaveCallback(System::Object^ state);
    };
}
