#pragma once
#include <Windows.h>

namespace Fences6
{
    /// <summary>
    /// 热键信息
    /// </summary>
    [System::Serializable]
    public ref class HotkeyInfo
    {
    public:
        /// <summary>
        /// 热键ID
        /// </summary>
        property int Id;

        /// <summary>
        /// 热键名称
        /// </summary>
        property System::String^ Name;

        /// <summary>
        /// 热键描述
        /// </summary>
        property System::String^ Description;

        /// <summary>
        /// 虚拟键码
        /// </summary>
        property int VirtualKey;

        /// <summary>
        /// 修饰键（Ctrl, Alt, Shift, Win）
        /// </summary>
        property int Modifiers;

        /// <summary>
        /// 是否启用
        /// </summary>
        property bool Enabled;

        /// <summary>
        /// 热键是否已注册
        /// </summary>
        property bool Registered;

        HotkeyInfo()
        {
            Modifiers = 0;
            Enabled = true;
            Registered = false;
        }

        HotkeyInfo(int id, System::String^ name, int virtualKey, int modifiers)
        {
            Id = id;
            Name = name;
            VirtualKey = virtualKey;
            Modifiers = modifiers;
            Enabled = true;
            Registered = false;
        }

        /// <summary>
        /// 获取热键的显示字符串
        /// </summary>
        property System::String^ DisplayString
        {
            System::String^ get()
            {
                System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder();

                if ((Modifiers & MOD_CONTROL) != 0)
                    sb->Append("Ctrl+");
                if ((Modifiers & MOD_ALT) != 0)
                    sb->Append("Alt+");
                if ((Modifiers & MOD_SHIFT) != 0)
                    sb->Append("Shift+");
                if ((Modifiers & MOD_WIN) != 0)
                    sb->Append("Win+");

                sb->Append(GetKeyName(VirtualKey));

                return sb->ToString();
            }
        }

    private:
        System::String^ GetKeyName(int vk)
        {
            if (vk >= 0x30 && vk <= 0x39)
                return vk.ToString();
            if (vk >= 0x41 && vk <= 0x5A)
                return ((char)vk).ToString();
            if (vk >= 0x70 && vk <= 0x87)
                return "F" + (vk - 0x6F).ToString();

            switch (vk)
            {
            case VK_SPACE: return "Space";
            case VK_RETURN: return "Enter";
            case VK_ESCAPE: return "Escape";
            case VK_TAB: return "Tab";
            case VK_BACK: return "Backspace";
            case VK_INSERT: return "Insert";
            case VK_DELETE: return "Delete";
            case VK_HOME: return "Home";
            case VK_END: return "End";
            case VK_PRIOR: return "PageUp";
            case VK_NEXT: return "PageDown";
            case VK_UP: return "Up";
            case VK_DOWN: return "Down";
            case VK_LEFT: return "Left";
            case VK_RIGHT: return "Right";
            case VK_MULTIPLY: return "Num*";
            case VK_ADD: return "Num+";
            case VK_SUBTRACT: return "Num-";
            case VK_DIVIDE: return "Num/";
            case VK_DECIMAL: return "Num.";
            default: return "Key" + vk.ToString();
            }
        }
    };

    /// <summary>
    /// 热键事件参数
    /// </summary>
    public ref class HotkeyEventArgs : public System::EventArgs
    {
    public:
        property int HotkeyId;
        property System::DateTime Time;

        HotkeyEventArgs(int id)
        {
            HotkeyId = id;
            Time = System::DateTime::Now;
        }
    };

    /// <summary>
    /// 热键管理器
    /// 负责全局热键的注册和响应
    /// </summary>
    public ref class HotkeyManager
    {
    public:
        /// <summary>
        /// 构造函数
        /// </summary>
        HotkeyManager();

        /// <summary>
        /// 析构函数
        /// </summary>
        ~HotkeyManager();

        /// <summary>
        /// 注册热键
        /// </summary>
        /// <param name="hotkey">热键信息</param>
        /// <returns>是否成功</returns>
        bool RegisterHotkey(HotkeyInfo^ hotkey);

        /// <summary>
        /// 注销热键
        /// </summary>
        /// <param name="hotkeyId">热键ID</param>
        /// <returns>是否成功</returns>
        bool UnregisterHotkey(int hotkeyId);

        /// <summary>
        /// 注销所有热键
        /// </summary>
        void UnregisterAll();

        /// <summary>
        /// 获取热键信息
        /// </summary>
        /// <param name="hotkeyId">热键ID</param>
        /// <returns>热键信息，如果不存在返回nullptr</returns>
        HotkeyInfo^ GetHotkey(int hotkeyId);

        /// <summary>
        /// 获取所有已注册的热键
        /// </summary>
        property System::Collections::Generic::List<HotkeyInfo^>^ Hotkeys;

        /// <summary>
        /// 是否有热键冲突
        /// </summary>
        bool HasConflict(int virtualKey, int modifiers);

        /// <summary>
        /// 设置窗口句柄（用于接收热键消息）
        /// </summary>
        property System::IntPtr WindowHandle
        {
            System::IntPtr get() { return m_hwnd; }
            void set(System::IntPtr value) { m_hwnd = value; }
        }

        /// <summary>
        /// 热键触发事件
        /// </summary>
        event System::EventHandler<HotkeyEventArgs^>^ HotkeyPressed;

        /// <summary>
        /// 处理热键消息
        /// </summary>
        void ProcessHotkeyMessage(int hotkeyId);

    internal:
        /// <summary>
        /// 设置原生窗口句柄
        /// </summary>
        void SetNativeWindow(HWND hwnd);

    private:
        System::IntPtr m_hwnd;
        HWND m_nativeHwnd;
        System::Collections::Generic::Dictionary<int, HotkeyInfo^>^ m_hotkeys;
        System::Object^ m_syncRoot;
        static int s_nextId;

        bool RegisterHotkeyInternal(HotkeyInfo^ hotkey);
        bool UnregisterHotkeyInternal(int hotkeyId, HWND hwnd);
    };
}
