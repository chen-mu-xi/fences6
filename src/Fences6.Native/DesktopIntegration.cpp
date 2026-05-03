#include "pch.h"
#include "DesktopIntegration.h"
#include <shlobj.h>
#include <shobjidl.h>
#include <winuser.h>
#include <algorithm>
#include <fstream>
#include <sstream>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")

// 常量定义
#define SHELL_TRAY_WND "Shell_TrayWnd"
#define PROGMAN_CLASS "Progman"
#define SHELLDLL_DEFVIEW_CLASS "SHELLDLL_DefView"
#define SYSLISTVIEW_CLASS "SysListView32"

// 消息定义
#define WM_COMMAND 0x0111
#define RefreshDesktopCommand 0xA065

// 窗口样式
#define WS_EX_TOOLWINDOW 0x00000080L
#define WS_EX_APPWINDOW 0x00040000L

DesktopIntegration::DesktopIntegration()
    : m_hDesktop(NULL)
    , m_hIconList(NULL)
    , m_iconsHidden(false)
{
    Initialize();
}

DesktopIntegration::~DesktopIntegration()
{
}

void DesktopIntegration::Initialize()
{
    m_hDesktop = GetDesktopWindow();
    m_hIconList = FindIconListView();
}

void DesktopIntegration::RefreshDesktop()
{
    // 方法1：使用 WM_COMMAND
    HWND hShellTray = FindWindowW(SHELL_TRAY_WND, NULL);
    if (hShellTray)
    {
        SendMessageW(hShellTray, WM_COMMAND, RefreshDesktopCommand, 0);
    }

    // 方法2：发送广播消息
    SendMessageTimeoutW(
        HWND_BROADCAST,
        WM_SETTINGCHANGE,
        0,
        (LPARAM)L"Pers",
        SMTO_ABORTIFHUNG,
        100,
        NULL
    );

    // 方法3：直接通知桌面窗口
    HWND hProgman = FindWindowW(PROGMAN_CLASS, NULL);
    if (hProgman)
    {
        SendMessageW(hProgman, WM_COMMAND, RefreshDesktopCommand, 0);
    }
}

HWND DesktopIntegration::GetDesktopWindow()
{
    if (!m_hDesktop)
    {
        // Progman 是桌面容器窗口
        m_hDesktop = FindWindowW(PROGMAN_CLASS, NULL);
        if (!m_hDesktop)
        {
            m_hDesktop = GetShellWindow();
        }
    }
    return m_hDesktop;
}

HWND DesktopIntegration::GetIconListView()
{
    if (!m_hIconList)
    {
        m_hIconList = FindIconListView();
    }
    return m_hIconList;
}

HWND DesktopIntegration::FindIconListView()
{
    HWND hDesktop = GetDesktopWindow();
    if (!hDesktop)
        return NULL;

    // 首先在 Progman 下查找 SHELLDLL_DefView
    HWND hShellDefView = FindWindowExW(hDesktop, NULL, SHELLDLL_DEFVIEW_CLASS, NULL);
    if (hShellDefView)
    {
        // SysListView32 是图标列表
        HWND hListView = FindWindowExW(hShellDefView, NULL, SYSLISTVIEW_CLASS, NULL);
        if (hListView)
            return hListView;
    }

    // 遍历所有顶层窗口查找
    struct FindData
    {
        HWND hResult;
        HWND hParent;
    } findData = { NULL, hDesktop };

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* pData = reinterpret_cast<FindData*>(lParam);
        
        // 检查是否是 Progman 或 WorkerW 的子窗口
        HWND hParent = GetAncestor(hwnd, GA_ROOT);
        if (hParent != pData->hParent)
            return TRUE;

        // 获取窗口类名
        wchar_t className[256];
        GetClassNameW(hwnd, className, 256);

        if (wcscmp(className, SYSLISTVIEW_CLASS) == 0)
        {
            // 检查是否是可见的桌面图标窗口
            DWORD style = GetWindowLongW(hwnd, GWL_STYLE);
            DWORD exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
            
            if ((style & WS_VISIBLE) && !(exStyle & WS_EX_TOOLWINDOW))
            {
                // 检查窗口标题或位置
                RECT rect;
                GetWindowRect(hwnd, &rect);
                
                // 桌面图标通常在全屏位置
                int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                int screenHeight = GetSystemMetrics(SM_CYSCREEN);
                
                if (rect.left <= 0 && rect.top <= 0 &&
                    rect.right >= screenWidth && rect.bottom >= screenHeight)
                {
                    pData->hResult = hwnd;
                    return FALSE;
                }
            }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&findData));

    return findData.hResult;
}

void DesktopIntegration::HideDesktopIcons(bool hide)
{
    HWND hShellDefView = NULL;
    HWND hDesktop = GetDesktopWindow();

    if (hDesktop)
    {
        hShellDefView = FindWindowExW(hDesktop, NULL, SHELLDLL_DEFVIEW_CLASS, NULL);
    }

    if (hShellDefView)
    {
        if (hide)
        {
            // 隐藏图标视图
            ShowWindow(hShellDefView, SW_HIDE);
        }
        else
        {
            // 显示图标视图
            ShowWindow(hShellDefView, SW_SHOW);
        }
        m_iconsHidden = hide;
    }
}

bool DesktopIntegration::IsDesktopIconsHidden()
{
    return m_iconsHidden;
}

void DesktopIntegration::EnableDesktopWindow(bool enable)
{
    HWND hDesktop = GetDesktopWindow();
    if (hDesktop)
    {
        EnableWindow(hDesktop, enable ? TRUE : FALSE);
    }
}

void DesktopIntegration::RefreshIcons()
{
    HWND hListView = GetIconListView();
    if (hListView)
    {
        // 发送 LVM_UPDATE 消息刷新图标
        SendMessageW(hListView, LVM_UPDATE, 0, 0);

        // 重新排列图标
        SendMessageW(hListView, LVM_ARRANGE, LVA_ALIGNLEFT, 0);
    }
}

bool DesktopIntegration::GetDesktopWorkArea(LPRECT pRect)
{
    if (!pRect)
        return false;

    // 使用 SystemParametersInfo 获取工作区域
    return SystemParametersInfoW(
        SPI_GETWORKAREA,
        0,
        pRect,
        0
    ) != FALSE;
}

void DesktopIntegration::GetDesktopSize(int* pWidth, int* pHeight)
{
    if (pWidth)
        *pWidth = GetSystemMetrics(SM_CXSCREEN);
    if (pHeight)
        *pHeight = GetSystemMetrics(SM_CYSCREEN);
}

void DesktopIntegration::MoveIcon(HWND hwnd, int x, int y)
{
    if (hwnd)
    {
        SetWindowPos(
            hwnd,
            NULL,
            x, y,
            0, 0,
            SWP_NOSIZE | SWP_NOZORDER
        );
    }
}

std::vector<HWND> DesktopIntegration::GetAllIconWindows()
{
    std::vector<HWND> result;
    HWND hListView = GetIconListView();

    if (hListView)
    {
        // 获取图标数量
        int count = SendMessageW(hListView, LVM_GETITEMCOUNT, 0, 0);
        
        // 获取每个图标的窗口句柄
        for (int i = 0; i < count; i++)
        {
            // 使用 LVM_GETITEMPOSITION 获取位置
            POINT pt;
            SendMessageW(hListView, LVM_GETITEMPOSITION, i, reinterpret_cast<LPARAM>(&pt));

            // 枚举顶层窗口找到对应位置的图标
            HWND hDesktop = GetDesktopWindow();
            if (hDesktop)
            {
                // 查找图标窗口的简单实现
                // 实际可能需要更精确的匹配逻辑
            }
        }
    }

    return result;
}

BOOL CALLBACK DesktopIntegration::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    auto* pWindows = reinterpret_cast<std::vector<HWND>*>(lParam);
    
    if (IsIconWindow(hwnd))
    {
        pWindows->push_back(hwnd);
    }
    
    return TRUE;
}

BOOL DesktopIntegration::IsIconWindow(HWND hwnd)
{
    if (!hwnd)
        return FALSE;

    // 检查窗口类名
    wchar_t className[256];
    GetClassNameW(hwnd, className, 256);

    // 桌面图标可能是 Button 或其他的
    if (wcscmp(className, L"Button") == 0)
    {
        // 检查按钮样式
        LONG style = GetWindowLongW(hwnd, GWL_STYLE);
        LONG exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);

        // 应该是图标按钮且不是工具窗口
        if ((style & BS_ICON) && !(exStyle & WS_EX_TOOLWINDOW))
        {
            return TRUE;
        }
    }

    return FALSE;
}

bool DesktopIntegration::GetWindowRect(HWND hwnd, LPRECT pRect)
{
    if (!hwnd || !pRect)
        return false;

    return ::GetWindowRect(hwnd, pRect) != FALSE;
}

bool DesktopIntegration::SetWindowPosEx(HWND hwnd, int x, int y, int width, int height)
{
    if (!hwnd)
        return false;

    return ::SetWindowPos(
        hwnd,
        NULL,
        x, y,
        width, height,
        SWP_NOZORDER
    ) != FALSE;
}

int DesktopIntegration::GetIconIndex(HWND hwnd)
{
    if (!hwnd)
        return -1;

    // 发送 LVM_GETITEMINDEX 获取索引
    return SendMessageW(hwnd, LVM_GETITEMINDEX, 0, 0);
}

void DesktopIntegration::SaveIconLayout()
{
    // 实现图标布局保存逻辑
}

void DesktopIntegration::RestoreIconLayout()
{
    // 实现图标布局恢复逻辑
}

void DesktopIntegration::ClearIconPositions()
{
    HWND hListView = GetIconListView();
    if (hListView)
    {
        // 发送消息清除位置
        SendMessageW(hListView, 0x1000 + 29, 0, 0); // LVM_RESETITEMPOSITION
    }
}

void DesktopIntegration::SendToDesktop(UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hDesktop = GetDesktopWindow();
    if (hDesktop)
    {
        SendMessageW(hDesktop, msg, wParam, lParam);
    }
}

// IconLayoutManager 实现

IconLayoutManager::IconLayoutManager()
{
}

std::wstring IconLayoutManager::GetLayoutFilePath()
{
    wchar_t appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath)))
    {
        std::wstring path = appDataPath;
        path += L"\\Fences6\\icon_layout.json";
        return path;
    }
    return L"icon_layout.json";
}

void IconLayoutManager::Save(const std::vector<IconLayoutInfo>& layout)
{
    std::wstring path = GetLayoutFilePath();
    
    // 确保目录存在
    wchar_t dir[MAX_PATH];
    wcscpy_s(dir, path.c_str());
    wchar_t* lastSlash = wcsrchr(dir, L'\\');
    if (lastSlash)
    {
        *lastSlash = L'\0';
        CreateDirectoryW(dir, NULL);
    }

    // 简单实现，实际应使用 JSON 库
    std::wofstream file(path);
    if (file.is_open())
    {
        file << L"{\"icons\":[" << std::endl;
        for (size_t i = 0; i < layout.size(); i++)
        {
            const auto& icon = layout[i];
            file << L"  {\"name\":\"" << icon.name << L"\",\"x\":" << icon.x << L",\"y\":" << icon.y << L"}";
            if (i < layout.size() - 1)
                file << L",";
            file << std::endl;
        }
        file << L"]}" << std::endl;
        file.close();
    }
}

std::vector<IconLayoutInfo> IconLayoutManager::Load()
{
    std::vector<IconLayoutInfo> layout;
    std::wstring path = GetLayoutFilePath();

    std::wifstream file(path);
    if (file.is_open())
    {
        // 简单解析 JSON
        // 实际应使用 JSON 库
        file.close();
    }

    return layout;
}

void IconLayoutManager::Apply(const std::vector<IconLayoutInfo>& layout)
{
    // 应用布局到桌面图标
}

// DesktopBackground 实现

bool DesktopBackground::SetWallpaper(const wchar_t* path)
{
    if (!path)
        return false;

    // 使用 SystemParametersInfo 设置壁纸
    return SystemParametersInfoW(
        SPI_SETDESKWALLPAPER,
        0,
        (void*)path,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
    ) != FALSE;
}

std::wstring DesktopBackground::GetWallpaper()
{
    wchar_t wallpaper[MAX_PATH];
    
    if (SystemParametersInfoW(
        SPI_GETDESKWALLPAPER,
        MAX_PATH,
        wallpaper,
        0
    ))
    {
        return std::wstring(wallpaper);
    }
    
    return std::wstring();
}

void DesktopBackground::SetWallpaperStyle(int style, int tile)
{
    HKEY hKey;
    
    if (RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Control Panel\\Desktop",
        0,
        KEY_SET_VALUE,
        &hKey
    ) == ERROR_SUCCESS)
    {
        wchar_t styleStr[16];
        wchar_t tileStr[16];
        
        swprintf_s(styleStr, L"%d", style);
        swprintf_s(tileStr, L"%d", tile);
        
        RegSetValueExW(hKey, L"WallpaperStyle", 0, REG_SZ,
            (const BYTE*)styleStr, (DWORD)(wcslen(styleStr) + 1) * sizeof(wchar_t));
        RegSetValueExW(hKey, L"TileWallpaper", 0, REG_SZ,
            (const BYTE*)tileStr, (DWORD)(wcslen(tileStr) + 1) * sizeof(wchar_t));
        
        RegCloseKey(hKey);
        
        Refresh();
    }
}

void DesktopBackground::Refresh()
{
    // 广播 WM_SETTINGCHANGE 消息
    SendMessageTimeoutW(
        HWND_BROADCAST,
        WM_SETTINGCHANGE,
        0,
        (LPARAM)L"Wallpaper",
        SMTO_ABORTIFHUNG,
        100,
        NULL
    );
}
