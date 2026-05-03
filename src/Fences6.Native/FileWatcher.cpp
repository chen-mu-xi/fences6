#include "pch.h"
#include "FileWatcher.h"
#include <cstring>
#include <algorithm>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

FileWatcher::FileWatcher(const std::wstring& path, bool subdirectories)
    : m_path(path)
    , m_subdirectories(subdirectories)
    , m_hDir(INVALID_HANDLE_VALUE)
    , m_hThread(NULL)
    , m_running(false)
    , m_stopped(false)
    , m_notifyFilter(0)
    , m_totalEvents(0)
    , m_droppedEvents(0)
    , m_buffer(BUFFER_SIZE)
{
    // 默认监控所有变化
    m_notifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME |
                     FILE_NOTIFY_CHANGE_DIR_NAME |
                     FILE_NOTIFY_CHANGE_LAST_WRITE |
                     FILE_NOTIFY_CHANGE_CREATION |
                     FILE_NOTIFY_CHANGE_SIZE;
}

FileWatcher::~FileWatcher()
{
    Stop();
}

bool FileWatcher::Start()
{
    if (m_running.load())
        return true;

    // 打开目录
    m_hDir = CreateFileW(
        m_path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (m_hDir == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        wprintf(L"CreateFile failed with error: %lu\n", error);
        return false;
    }

    m_stopped.store(false);
    m_running.store(true);
    ResetStats();

    // 启动监控线程
    m_hThread = CreateThread(
        NULL,
        0,
        WatchThreadProc,
        this,
        0,
        NULL
    );

    if (m_hThread == NULL)
    {
        CloseHandle(m_hDir);
        m_hDir = INVALID_HANDLE_VALUE;
        m_running.store(false);
        return false;
    }

    return true;
}

void FileWatcher::Stop()
{
    if (!m_running.load())
        return;

    m_stopped.store(true);
    m_running.store(false);

    if (m_hThread)
    {
        // 等待线程结束
        WaitForSingleObject(m_hThread, 5000);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    if (m_hDir != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hDir);
        m_hDir = INVALID_HANDLE_VALUE;
    }
}

void FileWatcher::SetCallback(FileChangeCallback callback)
{
    m_callback = callback;
}

void FileWatcher::SetNotifyFilter(DWORD notifyFilter)
{
    m_notifyFilter = notifyFilter;
}

void FileWatcher::AddFilter(const std::wstring& filter)
{
    m_filters.push_back(filter);
}

void FileWatcher::ClearFilters()
{
    m_filters.clear();
}

void FileWatcher::GetStats(DWORD* pTotalEvents, DWORD* pDroppedEvents)
{
    if (pTotalEvents)
        *pTotalEvents = m_totalEvents.load();
    if (pDroppedEvents)
        *pDroppedEvents = m_droppedEvents.load();
}

void FileWatcher::ResetStats()
{
    m_totalEvents.store(0);
    m_droppedEvents.store(0);
}

DWORD WINAPI FileWatcher::WatchThreadProc(LPVOID param)
{
    auto watcher = static_cast<FileWatcher*>(param);
    watcher->ProcessChanges();
    return 0;
}

void FileWatcher::ProcessChanges()
{
    DWORD bytesReturned = 0;
    BOOL result = FALSE;

    while (!m_stopped.load() && m_hDir != INVALID_HANDLE_VALUE)
    {
        // 清空缓冲区
        std::fill(m_buffer.begin(), m_buffer.end(), 0);

        // 使用 ReadDirectoryChangesW 监控变化
        result = ReadDirectoryChangesW(
            m_hDir,
            m_buffer.data(),
            static_cast<DWORD>(m_buffer.size()),
            m_subdirectories ? TRUE : FALSE,
            m_notifyFilter,
            &bytesReturned,
            NULL,
            NULL
        );

        if (!result)
        {
            DWORD error = GetLastError();
            if (error != ERROR_INVALID_PARAMETER && error != ERROR_NOACCESS)
            {
                wprintf(L"ReadDirectoryChangesW failed with error: %lu\n", error);
            }
            Sleep(100);
            continue;
        }

        if (bytesReturned > 0)
        {
            // 解析通知
            FILE_NOTIFY_INFORMATION* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(m_buffer.data());
            
            while (info && !m_stopped.load())
            {
                FileChangeInfo changeInfo;
                if (ParseNotification(info, changeInfo))
                {
                    m_totalEvents++;

                    // 检查过滤器
                    if (MatchesFilter(changeInfo.fileName))
                    {
                        // 调用回调
                        if (m_callback)
                        {
                            m_callback(changeInfo);
                        }
                    }
                }

                if (info->NextEntryOffset == 0)
                    break;

                info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                    reinterpret_cast<char*>(info) + info->NextEntryOffset);
            }
        }

        // 避免过于频繁的轮询
        Sleep(10);
    }
}

bool FileWatcher::ParseNotification(FILE_NOTIFY_INFORMATION* info, FileChangeInfo& changeInfo)
{
    if (!info)
        return false;

    // 解析文件名
    changeInfo.fileName = WideCharToWString(info->FileName, info->FileNameLength / sizeof(WCHAR));
    changeInfo.timestamp = GetTickCount();
    changeInfo.oldFileName.clear();

    // 解析动作类型
    switch (info->Action)
    {
    case FILE_ACTION_ADDED:
        changeInfo.action = FileAction::Created;
        break;
    case FILE_ACTION_REMOVED:
        changeInfo.action = FileAction::Deleted;
        break;
    case FILE_ACTION_MODIFIED:
        changeInfo.action = FileAction::Modified;
        break;
    case FILE_ACTION_RENAMED_OLD_NAME:
        changeInfo.action = FileAction::Renamed;
        // 下一个通知会包含新名称
        break;
    case FILE_ACTION_RENAMED_NEW_NAME:
        changeInfo.action = FileAction::Renamed;
        break;
    default:
        changeInfo.action = FileAction::Modified;
        break;
    }

    return !changeInfo.fileName.empty();
}

bool FileWatcher::MatchesFilter(const std::wstring& fileName)
{
    // 如果没有过滤器，接受所有文件
    if (m_filters.empty())
        return true;

    // 检查是否匹配任何过滤器
    for (const auto& filter : m_filters)
    {
        if (PathMatchSpecW(fileName.c_str(), filter.c_str()))
        {
            return true;
        }
    }

    return false;
}

std::wstring FileWatcher::WideCharToWString(WCHAR* buffer, size_t length)
{
    if (!buffer || length == 0)
        return std::wstring();

    return std::wstring(buffer, length);
}

// FileWatcherEx 实现

FileWatcherEx::FileWatcherEx(const std::wstring& path, bool subdirectories)
    : FileWatcher(path, subdirectories)
    , m_callbackInterface(nullptr)
{
}

FileWatcherEx::~FileWatcherEx()
{
}

void FileWatcherEx::SetCallbackInterface(IFileWatcherCallback* callback)
{
    m_callbackInterface = callback;
    
    if (callback)
    {
        SetCallback([this](const FileChangeInfo& info) {
            StaticCallback(info, this);
        });
    }
}

void FileWatcherEx::StaticCallback(const FileChangeInfo& info, void* userData)
{
    auto* self = static_cast<FileWatcherEx*>(userData);
    if (self && self->m_callbackInterface)
    {
        self->m_callbackInterface->OnFileChanged(info);
    }
}
