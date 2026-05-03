using System;
using Microsoft.Win32;

namespace Fences6.App.Services
{
    public class AutoStartService
    {
        private const string AppName = "Fences6";
        private const string RegistryKey = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Run";
        
        public bool IsEnabled
        {
            get
            {
                using var key = Registry.CurrentUser.OpenSubKey(RegistryKey, false);
                return key?.GetValue(AppName) != null;
            }
        }
        
        public void Enable()
        {
            using var key = Registry.CurrentUser.OpenSubKey(RegistryKey, true);
            var exePath = System.Diagnostics.Process.GetCurrentProcess().MainModule?.FileName;
            
            if (key != null && exePath != null)
            {
                key.SetValue(AppName, $"\"{exePath}\" --minimized");
            }
        }
        
        public void Disable()
        {
            using var key = Registry.CurrentUser.OpenSubKey(RegistryKey, true);
            key?.DeleteValue(AppName, false);
        }
        
        public void Toggle()
        {
            if (IsEnabled)
                Disable();
            else
                Enable();
        }
    }
}
