using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace Fences6.App.Services
{
    public class ShellIntegrationService
    {
        #region Win32 API
        
        [DllImport("user32.dll")]
        private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);
        
        [DllImport("user32.dll")]
        private static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
        
        [DllImport("user32.dll")]
        private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, 
            int X, int Y, int cx, int cy, uint uFlags);
        
        private const int SW_HIDE = 0;
        private const int SW_SHOW = 5;
        
        private const uint SWP_NOSIZE = 0x0001;
        private const uint SWP_NOMOVE = 0x0002;
        private const uint SWP_NOACTIVATE = 0x0010;
        
        public const int HWND_BOTTOM = 1;
        public const int HWND_TOPMOST = -1;
        
        #endregion
        
        private IntPtr m_hProgman;
        private IntPtr m_hShellView;
        
        public void Initialize()
        {
            // 获取 Progman 窗口
            m_hProgman = FindWindow("Progman", null);
            
            // 找到 ShellView 窗口
            m_hShellView = FindWindow("SHELLDLL_DefView", null);
            
            if (m_hShellView == IntPtr.Zero)
            {
                // 可能需要枚举子窗口
                EnumChildWindows(m_hProgman, (hwnd, lParam) =>
                {
                    var className = new char[256];
                    GetClassName(hwnd, className, 256);
                    if (new string(className).Contains("SHELLDLL_DefView"))
                    {
                        m_hShellView = hwnd;
                        return false;
                    }
                    return true;
                }, IntPtr.Zero);
            }
        }
        
        // 在桌面上分层窗口
        public void OverlayToDesktop(Window window)
        {
            var hwnd = new WindowInteropHelper(window).Handle;
            
            // 设置为最底层
            SetWindowPos(hwnd, (IntPtr)HWND_BOTTOM, 0, 0, 0, 0, 
                SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
            
            // 设置为桌面子窗口
            SetParent(hwnd, m_hProgman);
        }
        
        // 隐藏桌面图标
        public void HideDesktopIcons(bool hide)
        {
            if (m_hShellView != IntPtr.Zero)
            {
                ShowWindow(m_hShellView, hide ? SW_HIDE : SW_SHOW);
            }
        }
        
        // 刷新桌面
        [DllImport("shell32.dll")]
        private static extern void SHChangeNotify(int wEventId, int uFlags, 
            IntPtr dwItem1, IntPtr dwItem2);
        
        public void RefreshDesktop()
        {
            const int SHCNE_ASSOCCHANGED = 0x08000000;
            const int SHCNF_IDLIST = 0x0000;
            
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, IntPtr.Zero, IntPtr.Zero);
        }
        
        #region P/Invoke
        
        [DllImport("user32.dll")]
        private static extern bool SetParent(IntPtr hWndChild, IntPtr hWndNewParent);
        
        [DllImport("user32.dll")]
        private static extern int GetClassName(IntPtr hWnd, char[] lpClassName, int nMaxCount);
        
        private delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
        
        [DllImport("user32.dll")]
        private static extern bool EnumChildWindows(IntPtr hWndParent, 
            EnumWindowsProc lpEnumFunc, IntPtr lParam);
        
        #endregion
    }
}
