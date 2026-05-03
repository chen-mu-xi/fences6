using System;
using System.Linq;
using System.Windows;
using Fences6.App.Services;
using Fences6.Core;
using Microsoft.Extensions.DependencyInjection;

namespace Fences6.App
{
    public partial class App : Application
    {
        public static IServiceProvider Services { get; private set; } = null!;
        
        // 服务实例
        private TrayService? _trayService;
        private HotkeyService? _hotkeyService;
        private ShellIntegrationService? _shellService;
        private AutoStartService? _autoStartService;
        
        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);
            
            // 配置服务
            ConfigureServices();
            
            // 初始化核心库
            FenceCore.Initialize();
            
            // 初始化服务
            InitializeServices();
            
            // 创建主窗口
            var mainWindow = new MainWindow();
            MainWindow = mainWindow;
            
            // 初始化热键
            _hotkeyService?.Initialize(mainWindow);
            
            // 如果不是最小化启动，显示窗口
            if (!e.Args.Contains("--minimized"))
            {
                mainWindow.Show();
            }
            
            // 初始化托盘
            _trayService?.Initialize();
        }
        
        private void ConfigureServices()
        {
            var services = new ServiceCollection();
            
            services.AddSingleton<LayoutEngine>();
            services.AddSingleton<RuleEngine>();
            services.AddSingleton<ConfigManager>();
            services.AddSingleton<HotkeyManager>();
            services.AddSingleton<TrayService>();
            services.AddSingleton<HotkeyService>();
            services.AddSingleton<ShellIntegrationService>();
            services.AddSingleton<AutoStartService>();
            
            Services = services.BuildServiceProvider();
        }
        
        private void InitializeServices()
        {
            _trayService = Services.GetService(typeof(TrayService)) as TrayService;
            _hotkeyService = Services.GetService(typeof(HotkeyService)) as HotkeyService;
            _shellService = Services.GetService(typeof(ShellIntegrationService)) as ShellIntegrationService;
            _autoStartService = Services.GetService(typeof(AutoStartService)) as AutoStartService;
            
            // 订阅托盘事件
            _trayService!.ShowWindowRequested += (s, e) => ShowMainWindow();
            _trayService!.ExitRequested += (s, e) => Shutdown();
            _trayService!.SettingsRequested += (s, e) => OpenSettings();
            
            // 初始化 Shell 服务
            _shellService!.Initialize();
        }
        
        private void ShowMainWindow()
        {
            var mainWindow = MainWindow;
            if (mainWindow == null)
            {
                mainWindow = new MainWindow();
                MainWindow = mainWindow;
            }
            
            mainWindow.Show();
            mainWindow.WindowState = WindowState.Normal;
            mainWindow.Activate();
        }
        
        private void OpenSettings()
        {
            var settings = new Views.SettingsWindow();
            settings.Owner = MainWindow;
            settings.ShowDialog();
        }
        
        protected override void OnExit(ExitEventArgs e)
        {
            _hotkeyService?.Dispose();
            _trayService?.Dispose();
            
            FenceCore.Shutdown();
            
            base.OnExit(e);
        }
    }
}
