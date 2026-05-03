using CommunityToolkit.Mvvm.ComponentModel;

namespace Fences6.App.ViewModels
{
    public partial class SettingsViewModel : ObservableObject
    {
        [ObservableProperty]
        private bool _startWithWindows = true;
        
        [ObservableProperty]
        private bool _minimizeToTray = true;
        
        [ObservableProperty]
        private bool _showNotifications = true;
        
        [ObservableProperty]
        private bool _autoArrange = false;
        
        [ObservableProperty]
        private bool _snapToEdges = true;
        
        [ObservableProperty]
        private int _gridSize = 25;
        
        [ObservableProperty]
        private string _hotkey = "Win + Shift + F";
        
        [ObservableProperty]
        private int _opacity = 100;
        
        [ObservableProperty]
        private string _theme = "Light";
        
        public SettingsViewModel()
        {
            LoadSettings();
        }
        
        public void LoadSettings()
        {
            // 从配置管理器加载设置
            var configManager = App.Services.GetService(typeof(Fences6.Core.ConfigManager)) as Fences6.Core.ConfigManager;
            if (configManager != null)
            {
                var settings = configManager.LoadSettings();
                StartWithWindows = settings.StartWithWindows;
                MinimizeToTray = settings.MinimizeToTray;
                ShowNotifications = settings.ShowNotifications;
                AutoArrange = settings.AutoArrange;
                SnapToEdges = settings.SnapToEdges;
                GridSize = settings.GridSize;
                Hotkey = settings.Hotkey;
                Opacity = settings.Opacity;
                Theme = settings.Theme;
            }
        }
        
        public void SaveSettings()
        {
            var configManager = App.Services.GetService(typeof(Fences6.Core.ConfigManager)) as Fences6.Core.ConfigManager;
            if (configManager != null)
            {
                var settings = new Fences6.Core.AppSettings
                {
                    StartWithWindows = StartWithWindows,
                    MinimizeToTray = MinimizeToTray,
                    ShowNotifications = ShowNotifications,
                    AutoArrange = AutoArrange,
                    SnapToEdges = SnapToEdges,
                    GridSize = GridSize,
                    Hotkey = Hotkey,
                    Opacity = Opacity,
                    Theme = Theme
                };
                configManager.SaveSettings(settings);
            }
        }
    }
}
