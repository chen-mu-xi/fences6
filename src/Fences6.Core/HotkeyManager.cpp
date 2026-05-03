#include "pch.h"
#include "HotkeyManager.h"

namespace Fences6
{
    int HotkeyManager::s_nextId = 1;

    HotkeyManager::HotkeyManager()
    {
        m_hwnd = System::IntPtr::Zero;
        m_nativeHwnd = NULL;
        m_hotkeys = gcnew System::Collections::Generic::Dictionary<int, HotkeyInfo^>();
        m_syncRoot = gcnew System::Object();
    }

    HotkeyManager::~HotkeyManager()
    {
        UnregisterAll();
    }

    bool HotkeyManager::RegisterHotkey(HotkeyInfo^ hotkey)
    {
        if (hotkey == nullptr)
            return false;

        if (hotkey->Id == 0)
        {
            hotkey->Id = s_nextId++;
        }

        // 检查冲突
        if (HasConflict(hotkey->VirtualKey, hotkey->Modifiers))
        {
            System::Diagnostics::Debug::WriteLine("Hotkey conflict detected");
            return false;
        }

        return RegisterHotkeyInternal(hotkey);
    }

    bool HotkeyManager::RegisterHotkeyInternal(HotkeyInfo^ hotkey)
    {
        if (m_nativeHwnd == NULL)
        {
            System::Diagnostics::Debug::WriteLine("Window handle not set");
            return false;
        }

        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            // 注册全局热键
            BOOL result = RegisterHotKey(
                m_nativeHwnd,
                hotkey->Id,
                hotkey->Modifiers,
                hotkey->VirtualKey);

            if (result)
            {
                hotkey->Registered = true;
                m_hotkeys[hotkey->Id] = hotkey;
                return true;
            }
            else
            {
                DWORD error = GetLastError();
                System::Diagnostics::Debug::WriteLine(
                    "RegisterHotKey failed with error: " + error);
                return false;
            }
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    bool HotkeyManager::UnregisterHotkey(int hotkeyId)
    {
        if (m_nativeHwnd == NULL)
            return false;

        return UnregisterHotkeyInternal(hotkeyId, m_nativeHwnd);
    }

    bool HotkeyManager::UnregisterHotkeyInternal(int hotkeyId, HWND hwnd)
    {
        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            if (m_hotkeys->ContainsKey(hotkeyId))
            {
                HotkeyInfo^ hotkey = m_hotkeys[hotkeyId];

                if (hwnd != NULL && hotkey->Registered)
                {
                    UnregisterHotKey(hwnd, hotkeyId);
                    hotkey->Registered = false;
                }

                m_hotkeys->Remove(hotkeyId);
                return true;
            }
            return false;
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    void HotkeyManager::UnregisterAll()
    {
        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            auto ids = gcnew array<int>(m_hotkeys->Keys->Count);
            m_hotkeys->Keys->CopyTo(ids, 0);

            for each (int id in ids)
            {
                UnregisterHotkeyInternal(id, m_nativeHwnd);
            }
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    HotkeyInfo^ HotkeyManager::GetHotkey(int hotkeyId)
    {
        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            if (m_hotkeys->ContainsKey(hotkeyId))
            {
                return m_hotkeys[hotkeyId];
            }
            return nullptr;
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    property System::Collections::Generic::List<HotkeyInfo^>^ HotkeyManager::Hotkeys
    {
        System::Collections::Generic::List<HotkeyInfo^>^ get()
        {
            System::Threading::Monitor::Enter(m_syncRoot);
            try
            {
                auto list = gcnew System::Collections::Generic::List<HotkeyInfo^>();
                for each (HotkeyInfo^ hotkey in m_hotkeys->Values)
                {
                    list->Add(hotkey);
                }
                return list;
            }
            finally
            {
                System::Threading::Monitor::Exit(m_syncRoot);
            }
        }
    }

    bool HotkeyManager::HasConflict(int virtualKey, int modifiers)
    {
        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            for each (HotkeyInfo^ hotkey in m_hotkeys->Values)
            {
                if (hotkey->VirtualKey == virtualKey &&
                    hotkey->Modifiers == modifiers)
                {
                    return true;
                }
            }
            return false;
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    void HotkeyManager::SetNativeWindow(HWND hwnd)
    {
        m_nativeHwnd = hwnd;
        m_hwnd = System::IntPtr(hwnd);
    }

    void HotkeyManager::ProcessHotkeyMessage(int hotkeyId)
    {
        if (m_hotkeys->ContainsKey(hotkeyId))
        {
            HotkeyPressed(this, gcnew HotkeyEventArgs(hotkeyId));
        }
    }
}
