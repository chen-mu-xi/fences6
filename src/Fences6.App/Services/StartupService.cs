using System;
using System.Runtime.InteropServices;
using Microsoft.Win32;

namespace Fences6.App.Services
{
    /// <summary>
    /// 开机启动服务
    /// </summary>
    public class StartupService
    {
        private const string AppName = "Fences6";
        private const string RegistryKey = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Run";
        
        /// <summary>
        /// 检查是否设置了开机启动
        /// </summary>
        public static bool IsStartupEnabled()
        {
            try
            {
                using var key = Registry.CurrentUser.OpenSubKey(RegistryKey, false);
                return key?.GetValue(AppName) != null;
            }
            catch
            {
                return false;
            }
        }
        
        /// <summary>
        /// 启用开机启动
        /// </summary>
        public static bool EnableStartup()
        {
            try
            {
                var exePath = System.Diagnostics.Process.GetCurrentProcess().MainModule?.FileName;
                if (string.IsNullOrEmpty(exePath))
                    return false;
                
                using var key = Registry.CurrentUser.OpenSubKey(RegistryKey, true);
                key?.SetValue(AppName, $"\"{exePath}\"");
                return true;
            }
            catch
            {
                return false;
            }
        }
        
        /// <summary>
        /// 禁用开机启动
        /// </summary>
        public static bool DisableStartup()
        {
            try
            {
                using var key = Registry.CurrentUser.OpenSubKey(RegistryKey, true);
                key?.DeleteValue(AppName, false);
                return true;
            }
            catch
            {
                return false;
            }
        }
        
        /// <summary>
        /// 切换开机启动状态
        /// </summary>
        public static bool ToggleStartup()
        {
            if (IsStartupEnabled())
            {
                return DisableStartup();
            }
            else
            {
                return EnableStartup();
            }
        }
    }
}
