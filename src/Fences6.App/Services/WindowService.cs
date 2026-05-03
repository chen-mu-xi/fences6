using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace Fences6.App.Services
{
    /// <summary>
    /// 窗口管理服务
    /// </summary>
    public class WindowService
    {
        private static readonly IntPtr HWND_TOPMOST = new IntPtr(-1);
        private static readonly IntPtr HWND_NOTOPMOST = new IntPtr(-2);
        private const uint SWP_NOMOVE = 0x0002;
        private const uint SWP_NOSIZE = 0x0001;
        private const uint SWP_SHOWWINDOW = 0x0040;
        
        public static void SetTopMost(Window window)
        {
            var hwnd = new WindowInteropHelper(window).Handle;
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        }
        
        public static void SetNormal(Window window)
        {
            var hwnd = new WindowInteropHelper(window).Handle;
            SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        }
        
        public static void MinimizeToTray(Window window)
        {
            window.Hide();
            window.WindowState = WindowState.Minimized;
        }
        
        public static void RestoreFromTray(Window window)
        {
            window.Show();
            window.WindowState = WindowState.Normal;
            window.Activate();
        }
        
        [DllImport("user32.dll")]
        private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);
    }
}
