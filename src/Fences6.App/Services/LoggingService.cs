using System;
using System.IO;
using Serilog;
using Serilog.Events;

namespace Fences6.App.Services
{
    /// <summary>
    /// 日志服务
    /// </summary>
    public static class LoggingService
    {
        private static ILogger? _logger;
        private static readonly string LogPath;
        
        static LoggingService()
        {
            var appDataPath = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            var logDir = Path.Combine(appDataPath, "Fences6", "logs");
            
            if (!Directory.Exists(logDir))
                Directory.CreateDirectory(logDir);
            
            LogPath = Path.Combine(logDir, "fences6-.log");
        }
        
        public static void Initialize()
        {
            _logger = new LoggerConfiguration()
                .MinimumLevel.Debug()
                .WriteTo.File(
                    LogPath,
                    rollingInterval: RollingInterval.Day,
                    retainedFileCountLimit: 7,
                    outputTemplate: "{Timestamp:yyyy-MM-dd HH:mm:ss.fff} [{Level:u3}] {Message:lj}{NewLine}{Exception}")
                .CreateLogger();
            
            Log.Logger = _logger;
            
            Info("Fences6 日志系统已初始化");
        }
        
        public static void Info(string message)
        {
            _logger?.Information(message);
        }
        
        public static void Debug(string message)
        {
            _logger?.Debug(message);
        }
        
        public static void Warning(string message)
        {
            _logger?.Warning(message);
        }
        
        public static void Error(string message, Exception? ex = null)
        {
            if (ex != null)
                _logger?.Error(ex, message);
            else
                _logger?.Error(message);
        }
        
        public static void Fatal(string message, Exception? ex = null)
        {
            if (ex != null)
                _logger?.Fatal(ex, message);
            else
                _logger?.Fatal(message);
        }
        
        public static void Shutdown()
        {
            Info("Fences6 正在关闭");
            Log.CloseAndFlush();
        }
    }
}
