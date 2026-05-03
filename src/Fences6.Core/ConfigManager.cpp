#include "pch.h"
#include "ConfigManager.h"
#include <shlobj.h>

namespace Fences6
{
    ConfigManager::ConfigManager()
    {
        m_configItems = gcnew System::Collections::Generic::Dictionary<System::String^, ConfigItem^>();
        m_settings = gcnew AppSettings();
        m_syncRoot = gcnew System::Object();
        m_autoSave = false;
        m_autoSaveTimer = nullptr;

        // 获取配置目录
        m_configPath = GetDefaultConfigPath();

        // 确保目录存在
        try
        {
            if (!System::IO::Directory::Exists(m_configPath))
            {
                System::IO::Directory::CreateDirectory(m_configPath);
            }
        }
        catch (System::Exception^)
        {
            // 如果无法创建配置目录，使用临时目录
            m_configPath = System::IO::Path::GetTempPath();
        }
    }

    void ConfigManager::Load()
    {
        LoadFrom(System::IO::Path::Combine(m_configPath, "settings.json"));
    }

    void ConfigManager::Save()
    {
        SaveTo(System::IO::Path::Combine(m_configPath, "settings.json"));
    }

    void ConfigManager::SaveTo(System::String^ filePath)
    {
        if (System::String::IsNullOrEmpty(filePath))
            return;

        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            auto serializer = gcnew System::Runtime::Serialization::Json::DataContractJsonSerializer(
                AppSettings::typeid);

            auto stream = gcnew System::IO::FileStream(filePath, System::IO::FileMode::Create);
            serializer->WriteObject(stream, m_settings);
            stream->Close();

            // 同时保存配置项
            auto configPath = filePath->Replace("settings.json", "config.json");
            auto configSerializer = gcnew System::Runtime::Serialization::Json::DataContractJsonSerializer(
                m_configItems->GetType());

            auto configStream = gcnew System::IO::FileStream(configPath, System::IO::FileMode::Create);
            configSerializer->WriteObject(configStream, m_configItems);
            configStream->Close();
        }
        catch (System::Exception^ ex)
        {
            System::Diagnostics::Debug::WriteLine("SaveTo failed: " + ex->Message);
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    void ConfigManager::LoadFrom(System::String^ filePath)
    {
        if (System::String::IsNullOrEmpty(filePath) || !System::IO::File::Exists(filePath))
        {
            LoadDefaultConfig();
            return;
        }

        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            auto serializer = gcnew System::Runtime::Serialization::Json::DataContractJsonSerializer(
                AppSettings::typeid);

            auto stream = gcnew System::IO::FileStream(filePath, System::IO::FileMode::Open);
            auto settings = safe_cast<AppSettings^>(serializer->ReadObject(stream));
            stream->Close();

            if (settings != nullptr)
            {
                m_settings = settings;
            }

            // 加载配置项
            auto configPath = filePath->Replace("settings.json", "config.json");
            if (System::IO::File::Exists(configPath))
            {
                auto configSerializer = gcnew System::Runtime::Serialization::Json::DataContractJsonSerializer(
                    m_configItems->GetType());

                auto configStream = gcnew System::IO::FileStream(configPath, System::IO::FileMode::Open);
                auto items = safe_cast<System::Collections::Generic::Dictionary<System::String^, ConfigItem^>^>(
                    configSerializer->ReadObject(configStream));
                configStream->Close();

                if (items != nullptr)
                {
                    m_configItems = items;
                }
            }
        }
        catch (System::Exception^ ex)
        {
            System::Diagnostics::Debug::WriteLine("LoadFrom failed: " + ex->Message);
            LoadDefaultConfig();
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    ConfigItem^ ConfigManager::GetConfig(System::String^ key)
    {
        if (System::String::IsNullOrEmpty(key))
            return nullptr;

        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            if (m_configItems->ContainsKey(key))
            {
                return m_configItems[key];
            }
            return nullptr;
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    void ConfigManager::SetConfig(System::String^ key, System::String^ value)
    {
        if (System::String::IsNullOrEmpty(key))
            return;

        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            auto item = gcnew ConfigItem(key, value);
            m_configItems[key] = item;
            OnConfigChanged(key);
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    void ConfigManager::ResetToDefaults()
    {
        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            m_settings = gcnew AppSettings();
            m_configItems->Clear();
            OnConfigChanged("all");
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    void ConfigManager::Export(System::String^ filePath)
    {
        if (System::String::IsNullOrEmpty(filePath))
            return;

        SaveTo(filePath);
    }

    void ConfigManager::Import(System::String^ filePath)
    {
        if (System::String::IsNullOrEmpty(filePath) || !System::IO::File::Exists(filePath))
            return;

        LoadFrom(filePath);
    }

    void ConfigManager::OnConfigChanged(System::String^ key)
    {
        ConfigChanged(this, key);
    }

    void ConfigManager::LoadDefaultConfig()
    {
        m_settings = gcnew AppSettings();
    }

    System::String^ ConfigManager::GetDefaultConfigPath()
    {
        // 使用 %APPDATA%\Fences6 作为配置目录
        pin_ptr<const wchar_t> appData = nullptr;
        System::String^ result;

        try
        {
            wchar_t path[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path)))
            {
                result = gcnew System::String(path);
            }
            else
            {
                result = System::Environment::GetFolderPath(System::Environment::SpecialFolder::ApplicationData);
            }
        }
        catch (...)
        {
            result = System::IO::Path::GetTempPath();
        }

        return System::IO::Path::Combine(result, "Fences6");
    }

    void ConfigManager::StartAutoSave()
    {
        if (m_autoSaveTimer == nullptr)
        {
            // 每5分钟自动保存一次
            m_autoSaveTimer = gcnew System::Threading::Timer(
                gcnew System::Threading::TimerCallback(this, &ConfigManager::AutoSaveCallback),
                nullptr,
                TimeSpan::FromMinutes(5),
                TimeSpan::FromMinutes(5));
            m_autoSave = true;
        }
    }

    void ConfigManager::StopAutoSave()
    {
        if (m_autoSaveTimer != nullptr)
        {
            m_autoSaveTimer->Dispose();
            m_autoSaveTimer = nullptr;
            m_autoSave = false;
        }
    }

    void ConfigManager::AutoSaveCallback(System::Object^ state)
    {
        if (m_autoSave)
        {
            Save();
        }
    }
}
