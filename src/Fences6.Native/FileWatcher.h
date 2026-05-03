#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <functional>

/// <summary>
/// 文件监控类
/// 使用 Windows API 监控目录和文件的变化
/// </summary>
class FileWatcher
{
public:
    /// <summary>
    /// 文件变化动作类型
    /// </summary>
    enum class FileAction
    {
        Created = 1,    ///< 文件被创建
        Deleted = 2,   ///< 文件被删除
        Modified = 3,   ///< 文件被修改
        Renamed = 4,    ///< 文件被重命名
        Overflow = 5   ///< 缓冲区溢出
    };

    /// <summary>
    /// 文件变化信息
    /// </summary>
    struct FileChangeInfo
    {
        std::wstring fileName;    ///< 变化的文件名
        FileAction action;        ///< 动作类型
        std::wstring oldFileName; ///< 重命名前的文件名（仅重命名时）
        DWORD timestamp;         ///< 时间戳
    };

    /// <summary>
    /// 文件变化回调函数类型
    /// </summary>
    typedef std::function<void(const FileChangeInfo& info)> FileChangeCallback;

    /// <summary>
    /// 构造函数
    /// </summary>
    /// <param name="path">要监控的目录路径</param>
    /// <param name="subdirectories">是否监控子目录</param>
    FileWatcher(const std::wstring& path, bool subdirectories = true);

    /// <summary>
    /// 析构函数
    /// </summary>
    ~FileWatcher();

    /// <summary>
    /// 开始监控
    /// </summary>
    /// <returns>是否成功</returns>
    bool Start();

    /// <summary>
    /// 停止监控
    /// </summary>
    void Stop();

    /// <summary>
    /// 是否正在运行
    /// </summary>
    bool IsRunning() const { return m_running.load(); }

    /// <summary>
    /// 设置文件变化回调
    /// </summary>
    /// <param name="callback">回调函数</param>
    void SetCallback(FileChangeCallback callback);

    /// <summary>
    /// 设置监控标志
    /// </summary>
    /// <param name="notifyFilter">监控的文件变化类型</param>
    void SetNotifyFilter(DWORD notifyFilter);

    /// <summary>
    /// 添加要监控的文件过滤器
    /// </summary>
    /// <param name="filter">文件扩展名（如 L"*.txt"）</param>
    void AddFilter(const std::wstring& filter);

    /// <summary>
    /// 清除所有过滤器
    /// </summary>
    void ClearFilters();

    /// <summary>
    /// 获取被监控的路径
    /// </summary>
    const std::wstring& GetPath() const { return m_path; }

    /// <summary>
    /// 获取监控状态信息
    /// </summary>
    void GetStats(DWORD* pTotalEvents, DWORD* pDroppedEvents);

    /// <summary>
    /// 重置统计信息
    /// </summary>
    void ResetStats();

private:
    std::wstring m_path;
    bool m_subdirectories;
    HANDLE m_hDir;
    HANDLE m_hThread;
    std::atomic<bool> m_running;
    std::atomic<bool> m_stopped;
    FileChangeCallback m_callback;
    DWORD m_notifyFilter;
    std::vector<std::wstring> m_filters;

    // 统计
    std::atomic<DWORD> m_totalEvents;
    std::atomic<DWORD> m_droppedEvents;

    // 缓冲区
    static const DWORD BUFFER_SIZE = 65536;
    std::vector<char> m_buffer;

    // 监控线程
    static DWORD WINAPI WatchThreadProc(LPVOID param);
    void ProcessChanges();

    // 解析文件变化信息
    bool ParseNotification(FILE_NOTIFY_INFORMATION* info, FileChangeInfo& changeInfo);

    // 检查文件是否匹配过滤器
    bool MatchesFilter(const std::wstring& fileName);

    // 工具函数
    std::wstring WideCharToWString(WCHAR* buffer, size_t length);
};

// FileWatcher 回调接口类（可选的面向对象接口）
class IFileWatcherCallback
{
public:
    virtual ~IFileWatcherCallback() {}
    virtual void OnFileChanged(const FileWatcher::FileChangeInfo& info) = 0;
};

/// <summary>
/// 文件监控器包装类
/// 支持使用接口回调
/// </summary>
class FileWatcherEx : public FileWatcher
{
public:
    FileWatcherEx(const std::wstring& path, bool subdirectories = true);
    virtual ~FileWatcherEx();

    /// <summary>
    /// 设置回调接口
    /// </summary>
    void SetCallbackInterface(IFileWatcherCallback* callback);

protected:
    static void StaticCallback(const FileWatcher::FileChangeInfo& info, void* userData);

private:
    IFileWatcherCallback* m_callbackInterface;
};
