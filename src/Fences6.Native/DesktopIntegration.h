#pragma once

#include <Windows.h>
#include <string>
#include <vector>

/// <summary>
/// 桌面集成类
/// 提供与 Windows 桌面的交互功能
/// </summary>
class DesktopIntegration
{
public:
    /// <summary>
    /// 构造函数
    /// </summary>
    DesktopIntegration();

    /// <summary>
    /// 析构函数
    /// </summary>
    ~DesktopIntegration();

    /// <summary>
    /// 刷新桌面
    /// </summary>
    void RefreshDesktop();

    /// <summary>
    /// 获取桌面窗口句柄
    /// </summary>
    HWND GetDesktopWindow();

    /// <summary>
    /// 获取桌面列表视图窗口（图标容器）
    /// </summary>
    HWND GetIconListView();

    /// <summary>
    /// 隐藏桌面图标
    /// </summary>
    /// <param name="hide">true-隐藏, false-显示</param>
    void HideDesktopIcons(bool hide);

    /// <summary>
    /// 是否桌面图标已隐藏
    /// </summary>
    bool IsDesktopIconsHidden();

    /// <summary>
    /// 启用或禁用桌面窗口
    /// </summary>
    /// <param name="enable">true-启用, false-禁用</param>
    void EnableDesktopWindow(bool enable);

    /// <summary>
    /// 刷新桌面图标
    /// </summary>
    void RefreshIcons();

    /// <summary>
    /// 获取桌面工作区域
    /// </summary>
    bool GetDesktopWorkArea(LPRECT pRect);

    /// <summary>
    /// 获取桌面大小
    /// </summary>
    void GetDesktopSize(int* pWidth, int* pHeight);

    /// <summary>
    /// 移动桌面图标
    /// </summary>
    /// <param name="hwnd">图标窗口句柄</param>
    /// <param name="x">新X坐标</param>
    /// <param name="y">新Y坐标</param>
    void MoveIcon(HWND hwnd, int x, int y);

    /// <summary>
    /// 获取所有桌面图标窗口
    /// </summary>
    std::vector<HWND> GetAllIconWindows();

    /// <summary>
    /// 窗口枚举回调
    /// </summary>
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

    /// <summary>
    /// 检查窗口是否是桌面图标
    /// </summary>
    static BOOL IsIconWindow(HWND hwnd);

    /// <summary>
    /// 获取窗口位置和大小
    /// </summary>
    bool GetWindowRect(HWND hwnd, LPRECT pRect);

    /// <summary>
    /// 设置窗口位置和大小
    /// </summary>
    bool SetWindowPosEx(HWND hwnd, int x, int y, int width, int height);

    /// <summary>
    /// 获取图标索引
    /// </summary>
    int GetIconIndex(HWND hwnd);

    /// <summary>
    /// 保存当前图标布局
    /// </summary>
    void SaveIconLayout();

    /// <summary>
    /// 恢复图标布局
    /// </summary>
    void RestoreIconLayout();

    /// <summary>
    /// 清空所有桌面图标位置
    /// </summary>
    void ClearIconPositions();

private:
    HWND m_hDesktop;
    HWND m_hIconList;
    bool m_iconsHidden;

    /// <summary>
    /// 初始化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 查找桌面列表视图窗口
    /// </summary>
    HWND FindIconListView();

    /// <summary>
    /// 发送消息到桌面
    /// </summary>
    void SendToDesktop(UINT msg, WPARAM wParam, LPARAM lParam);
};

/// <summary>
/// 图标布局信息
/// </summary>
struct IconLayoutInfo
{
    std::wstring name;
    int x;
    int y;
    int index;
};

/// <summary>
/// 图标布局管理器
/// </summary>
class IconLayoutManager
{
public:
    /// <summary>
    /// 构造函数
    /// </summary>
    IconLayoutManager();

    /// <summary>
    /// 保存布局
    /// </summary>
    void Save(const std::vector<IconLayoutInfo>& layout);

    /// <summary>
    /// 加载布局
    /// </summary>
    std::vector<IconLayoutInfo> Load();

    /// <summary>
    /// 应用布局到桌面
    /// </summary>
    void Apply(const std::vector<IconLayoutInfo>& layout);

private:
    std::wstring m_layoutPath;

    std::wstring GetLayoutFilePath();
};

/// <summary>
/// 桌面背景管理
/// </summary>
class DesktopBackground
{
public:
    /// <summary>
    /// 设置桌面背景
    /// </summary>
    static bool SetWallpaper(const wchar_t* path);

    /// <summary>
    /// 获取当前桌面背景
    /// </summary>
    static std::wstring GetWallpaper();

    /// <summary>
    /// 设置壁纸样式
    /// </summary>
    static void SetWallpaperStyle(int style, int tile);

    /// <summary>
    /// 刷新壁纸
    /// </summary>
    static void Refresh();
};
