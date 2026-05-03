using System;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Fences6.Core;

namespace Fences6.App.ViewModels
{
    public partial class FenceViewModel : ObservableObject
    {
        [ObservableProperty]
        private string _id = string.Empty;
        
        [ObservableProperty]
        private string _name = string.Empty;
        
        [ObservableProperty]
        private FenceRect _bounds;
        
        [ObservableProperty]
        private ObservableCollection<IconViewModel> _icons = new();
        
        [ObservableProperty]
        private bool _isSelected;
        
        [ObservableProperty]
        private bool _isEditing;
        
        [ObservableProperty]
        private int _iconCount;
        
        [ObservableProperty]
        private double _opacity = 1.0;
        
        [ObservableProperty]
        private string _backgroundColor = "#FFFFFF";
        
        public FenceViewModel()
        {
            Id = Guid.NewGuid().ToString();
            Icons.CollectionChanged += Icons_CollectionChanged;
        }
        
        public FenceViewModel(FenceGroup fence) : this()
        {
            Id = fence.Id;
            Name = fence.Name;
            Bounds = fence.Bounds;
            
            if (fence.Icons != null)
            {
                foreach (var iconPath in fence.Icons)
                {
                    Icons.Add(new IconViewModel { Path = iconPath });
                }
            }
        }
        
        private void Icons_CollectionChanged(object? sender, NotifyCollectionChangedEventArgs e)
        {
            IconCount = Icons.Count;
        }
        
        public FenceGroup ToFenceGroup()
        {
            var fence = new FenceGroup
            {
                Id = Id,
                Name = Name,
                Bounds = Bounds,
                Icons = new System.Collections.Generic.List<string>()
            };
            
            foreach (var icon in Icons)
            {
                fence.Icons.Add(icon.Path);
            }
            
            return fence;
        }
        
        [RelayCommand]
        private void StartEditing()
        {
            IsEditing = true;
        }
        
        [RelayCommand]
        private void FinishEditing()
        {
            IsEditing = false;
        }
        
        [RelayCommand]
        private void Delete()
        {
            var configManager = App.Services.GetService(typeof(ConfigManager)) as ConfigManager;
            configManager?.DeleteFence(Id);
        }
        
        [RelayCommand]
        private void AddIcon()
        {
            // TODO: 打开图标选择器
        }
    }
    
    public partial class IconViewModel : ObservableObject
    {
        [ObservableProperty]
        private string _path = string.Empty;
        
        [ObservableProperty]
        private string _name = string.Empty;
        
        [ObservableProperty]
        private double _x;
        
        [ObservableProperty]
        private double _y;
        
        [ObservableProperty]
        private bool _isSelected;
    }
}
