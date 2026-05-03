using System;
using System.Collections.ObjectModel;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Fences6.Core;

namespace Fences6.App.ViewModels
{
    public partial class MainViewModel : ObservableObject
    {
        [ObservableProperty]
        private string _searchText = string.Empty;
        
        [ObservableProperty]
        private string _statusMessage = "就绪";
        
        [ObservableProperty]
        private int _fenceCount;
        
        [ObservableProperty]
        private int _totalIconCount;
        
        [ObservableProperty]
        private ObservableCollection<FenceViewModel> _fenceGroups = new();
        
        [ObservableProperty]
        private FenceViewModel? _selectedFence;
        
        private readonly LayoutEngine _layoutEngine;
        private readonly RuleEngine _ruleEngine;
        private readonly ConfigManager _configManager;
        private readonly HotkeyManager _hotkeyManager;
        
        public MainViewModel()
        {
            _layoutEngine = App.Services.GetService(typeof(LayoutEngine)) as LayoutEngine ?? new LayoutEngine();
            _ruleEngine = App.Services.GetService(typeof(RuleEngine)) as RuleEngine ?? new RuleEngine();
            _configManager = App.Services.GetService(typeof(ConfigManager)) as ConfigManager ?? new ConfigManager();
            _hotkeyManager = App.Services.GetService(typeof(HotkeyManager)) as HotkeyManager ?? new HotkeyManager();
        }
        
        public void LoadLayout()
        {
            try
            {
                var fences = _configManager.LoadFences();
                
                FenceGroups.Clear();
                foreach (var fence in fences)
                {
                    var vm = new FenceViewModel(fence);
                    FenceGroups.Add(vm);
                }
                
                UpdateCounts();
                StatusMessage = "布局已加载";
            }
            catch (Exception ex)
            {
                StatusMessage = $"加载失败: {ex.Message}";
            }
        }
        
        [RelayCommand]
        private void CreateNewFence()
        {
            var newFence = new FenceGroup
            {
                Id = Guid.NewGuid().ToString(),
                Name = $"新建分组 {FenceGroups.Count + 1}",
                Bounds = new FenceRect { X = 100, Y = 100, Width = 300, Height = 200 }
            };
            
            _configManager.SaveFence(newFence);
            
            var vm = new FenceViewModel(newFence);
            FenceGroups.Add(vm);
            
            UpdateCounts();
            StatusMessage = "已创建新分组";
        }
        
        [RelayCommand]
        private void AutoArrange()
        {
            var fences = new System.Collections.Generic.List<FenceGroup>();
            foreach (var vm in FenceGroups)
            {
                fences.Add(vm.ToFenceGroup());
            }
            
            _layoutEngine.AutoArrange(fences);
            
            for (int i = 0; i < fences.Count && i < FenceGroups.Count; i++)
            {
                FenceGroups[i].Bounds = fences[i].Bounds;
            }
            
            _configManager.SaveAllFences(fences);
            StatusMessage = "已完成自动排列";
        }
        
        [RelayCommand]
        private void OpenSettings()
        {
            StatusMessage = "打开设置...";
            // TODO: 打开设置窗口
        }
        
        [RelayCommand]
        private void ShowMainWindow()
        {
            // 由 App.xaml.cs 处理
        }
        
        [RelayCommand]
        private void Exit()
        {
            App.Current.Shutdown();
        }
        
        [RelayCommand]
        private void RefreshIcons()
        {
            try
            {
                _ruleEngine.ScanDesktop();
                LoadLayout();
                StatusMessage = "已刷新桌面图标";
            }
            catch (Exception ex)
            {
                StatusMessage = $"刷新失败: {ex.Message}";
            }
        }
        
        private void UpdateCounts()
        {
            FenceCount = FenceGroups.Count;
            TotalIconCount = 0;
            foreach (var fence in FenceGroups)
            {
                TotalIconCount += fence.IconCount;
            }
        }
    }
}
