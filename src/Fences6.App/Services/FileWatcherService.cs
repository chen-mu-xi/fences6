using System;
using System.Collections.Generic;
using System.IO;
using Fences6.Core;

namespace Fences6.App.Services
{
    /// <summary>
    /// 文件监控服务
    /// </summary>
    public class FileWatcherService : IDisposable
    {
        private Native.FileWatcher? _watcher;
        private readonly string _desktopPath;
        private bool _disposed;
        
        public event EventHandler<FileChangedEventArgs>? FileChanged;
        
        public FileWatcherService()
        {
            _desktopPath = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
        }
        
        public void Start()
        {
            if (_watcher != null)
                return;
            
            _watcher = new Native.FileWatcher(_desktopPath);
            _watcher.SetCallback(OnFileChanged);
            _watcher.Start();
        }
        
        public void Stop()
        {
            _watcher?.Stop();
        }
        
        private void OnFileChanged(string path, uint action)
        {
            var args = new FileChangedEventArgs
            {
                Path = path,
                ChangeType = (WatcherChangeTypes)action
            };
            
            FileChanged?.Invoke(this, args);
        }
        
        public void Dispose()
        {
            if (_disposed)
                return;
            
            Stop();
            _disposed = true;
        }
    }
    
    public class FileChangedEventArgs : EventArgs
    {
        public string Path { get; set; } = string.Empty;
        public WatcherChangeTypes ChangeType { get; set; }
    }
}

namespace Fences6.App.Native
{
    using Fences6.Native;
    
    public class FileWatcher
    {
        private IntPtr _handle;
        
        public FileWatcher(string path)
        {
            _handle = CreateWatcher(path);
        }
        
        public void Start()
        {
            StartWatcher(_handle);
        }
        
        public void Stop()
        {
            StopWatcher(_handle);
        }
        
        public void SetCallback(Action<string, uint> callback)
        {
            // 回调处理
        }
        
        private IntPtr CreateWatcher(string path) => IntPtr.Zero;
        private void StartWatcher(IntPtr handle) { }
        private void StopWatcher(IntPtr handle) { }
    }
}
