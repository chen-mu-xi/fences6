#pragma once
#include "pch.h"

namespace Fences6
{
    // Forward declarations
    ref class LayoutEngine;
    ref class RuleEngine;
    ref class ConfigManager;
    ref class HotkeyManager;
    
    // 矩形结构
    public value struct FenceRect
    {
        double X;
        double Y;
        double Width;
        double Height;
        
        FenceRect(double x = 0, double y = 0, double width = 0, double height = 0)
            : X(x), Y(y), Width(width), Height(height) {}
        
        property bool IsEmpty
        {
            bool get() { return Width <= 0 || Height <= 0; }
        }
        
        bool Contains(double x, double y)
        {
            return x >= X && x <= X + Width && y >= Y && y <= Y + Height;
        }
    };
    
    // 应用设置
    public ref class AppSettings
    {
    public:
        bool StartWithWindows;
        bool MinimizeToTray;
        bool ShowNotifications;
        bool AutoArrange;
        bool SnapToEdges;
        int GridSize;
        String^ Hotkey;
        int Opacity;
        String^ Theme;
        
        AppSettings()
        {
            StartWithWindows = true;
            MinimizeToTray = true;
            ShowNotifications = true;
            AutoArrange = false;
            SnapToEdges = true;
            GridSize = 25;
            Hotkey = "Win+Shift+F";
            Opacity = 100;
            Theme = "Light";
        }
    };
    
    // Fence 分组
    public ref class FenceGroup
    {
    public:
        property String^ Id;
        property String^ Name;
        property FenceRect Bounds;
        property List<String^>^ Icons;
        property String^ BackgroundColor;
        property double Opacity;
        property String^ IconRule;
        
        FenceGroup()
        {
            Id = Guid::NewGuid().ToString();
            Name = "新建分组";
            Icons = gcnew List<String^>();
            BackgroundColor = "#FFFFFF";
            Opacity = 1.0;
            IconRule = "";
        }
    };
    
    // 桌面图标信息
    public ref class DesktopIcon
    {
    public:
        property String^ Path;
        property String^ Name;
        property String^ TargetPath;
        property String^ Arguments;
        property String^ WorkingDirectory;
        property int IconIndex;
        
        DesktopIcon()
        {
            Path = "";
            Name = "";
            TargetPath = "";
            Arguments = "";
            WorkingDirectory = "";
            IconIndex = 0;
        }
    };
    
    // 核心库主类
    public ref class FenceCore
    {
    private:
        static bool s_initialized = false;
        static Object^ s_lock = gcnew Object();
        
    public:
        static void Initialize()
        {
            if (s_initialized) return;
            
            Lock(s_lock);
            try
            {
                if (!s_initialized)
                {
                    // 初始化日志系统
                    InitializeLogging();
                    
                    // 加载配置
                    auto config = ConfigManager();
                    config.LoadSettings();
                    
                    // 注册全局热键
                    auto hotkeyMgr = HotkeyManager();
                    hotkeyMgr.RegisterHotkey();
                    
                    // 启动文件监控
                    auto fileWatcher = gcnew Fences6::Native::FileWatcher(
                        Environment::GetFolderPath(Environment::SpecialFolder::Desktop));
                    fileWatcher->Start();
                    
                    s_initialized = true;
                }
            }
            finally
            {
                Monitor::Exit(s_lock);
            }
        }
        
        static void Shutdown()
        {
            if (!s_initialized) return;
            
            Lock(s_lock);
            try
            {
                if (s_initialized)
                {
                    // 取消全局热键
                    auto hotkeyMgr = HotkeyManager();
                    hotkeyMgr.UnregisterHotkey();
                    
                    // 保存配置
                    auto config = ConfigManager();
                    config.SaveSettings();
                    
                    s_initialized = false;
                }
            }
            finally
            {
                Monitor::Exit(s_lock);
            }
        }
        
    private:
        static void InitializeLogging()
        {
            String^ logPath = Path::Combine(
                Environment::GetFolderPath(Environment::SpecialFolder::LocalApplicationData),
                "Fences6", "fences6.log");
            
            String^ logDir = Path::GetDirectoryName(logPath);
            if (!String::IsNullOrEmpty(logDir) && !Directory::Exists(logDir))
            {
                Directory::CreateDirectory(logDir);
            }
        }
    };
}
