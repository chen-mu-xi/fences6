using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace Fences6.App.Services
{
    public class HotkeyService : IDisposable
    {
        #region Win32 API
        
        [DllImport("user32.dll")]
        private static extern bool RegisterHotKey(IntPtr hWnd, int id, uint fsModifiers, uint vk);
        
        [DllImport("user32.dll")]
        private static extern bool UnregisterHotKey(IntPtr hWnd, int id);
        
        private const int WM_HOTKEY = 0x0312;
        
        // 修饰键
        public const uint MOD_ALT = 0x0001;
        public const uint MOD_CONTROL = 0x0002;
        public const uint MOD_SHIFT = 0x0004;
        public const uint MOD_WIN = 0x0008;
        public const uint MOD_NOREPEAT = 0x4000;
        
        // 虚拟键码
        public const uint VK_F = 0x46;
        public const uint VK_D = 0x44;
        public const uint VK_L = 0x4C;
        
        #endregion
        
        private IntPtr m_hwnd;
        private HwndSource? m_source;
        private readonly Dictionary<int, Action> m_hotkeyActions = new();
        private int m_currentId = 9000;
        private bool m_disposed;
        
        // 热键 ID
        public const int HOTKEY_SHOW_ALL = 9000;
        public const int HOTKEY_HIDE_ALL = 9001;
        public const int HOTKEY_LOCK = 9002;
        public const int HOTKEY_PEEK = 9003;
        
        public void Initialize(Window window)
        {
            m_hwnd = new WindowInteropHelper(window).Handle;
            m_source = HwndSource.FromHwnd(m_hwnd);
            m_source?.AddHook(HwndHook);
            
            // 注册默认热键
            RegisterDefaultHotkeys();
        }
        
        private void RegisterDefaultHotkeys()
        {
            // Win + D: 显示所有分组
            Register(HOTKEY_SHOW_ALL, MOD_WIN | MOD_SHIFT, VK_D, () =>
            {
                // 显示所有分组
                EventBroker.Instance.Publish("Hotkey:ShowAll");
            });
            
            // Win + F: 切换显示
            Register(HOTKEY_SHOW_ALL, MOD_WIN | MOD_NOREPEAT, VK_F, () =>
            {
                EventBroker.Instance.Publish("Hotkey:Toggle");
            });
            
            // Ctrl + Win + L: 锁定布局
            Register(HOTKEY_LOCK, MOD_CONTROL | MOD_WIN, VK_L, () =>
            {
                EventBroker.Instance.Publish("Hotkey:Lock");
            });
        }
        
        public bool Register(int id, uint modifiers, uint key, Action action)
        {
            if (m_hwnd == IntPtr.Zero) return false;
            
            // 取消已注册的热键
            if (m_hotkeyActions.ContainsKey(id))
            {
                UnregisterHotKey(m_hwnd, id);
            }
            
            bool result = RegisterHotKey(m_hwnd, id, modifiers, key);
            if (result)
            {
                m_hotkeyActions[id] = action;
            }
            return result;
        }
        
        public void Unregister(int id)
        {
            if (m_hwnd == IntPtr.Zero) return;
            
            UnregisterHotKey(m_hwnd, id);
            m_hotkeyActions.Remove(id);
        }
        
        private IntPtr HwndHook(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            if (msg == WM_HOTKEY)
            {
                int id = wParam.ToInt32();
                if (m_hotkeyActions.TryGetValue(id, out var action))
                {
                    action.Invoke();
                    handled = true;
                }
            }
            return IntPtr.Zero;
        }
        
        public void Dispose()
        {
            if (!m_disposed)
            {
                m_source?.RemoveHook(HwndHook);
                
                foreach (var id in m_hotkeyActions.Keys)
                {
                    UnregisterHotKey(m_hwnd, id);
                }
                m_hotkeyActions.Clear();
                
                m_disposed = true;
            }
        }
    }
}
