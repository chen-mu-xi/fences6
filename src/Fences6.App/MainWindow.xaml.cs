using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using Fences6.App.ViewModels;

namespace Fences6.App
{
    public partial class MainWindow : Window
    {
        private readonly MainViewModel _viewModel;
        
        public MainWindow()
        {
            InitializeComponent();
            
            _viewModel = App.Services.GetService(typeof(MainViewModel)) as MainViewModel ?? new MainViewModel();
            DataContext = _viewModel;
            
            // 加载配置
            Loaded += MainWindow_Loaded;
        }
        
        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            // 启用窗口拖动
            var helper = new WindowInteropHelper(this);
            var source = HwndSource.FromHwnd(helper.Handle);
            source?.AddHook(WndProc);
            
            // 加载布局
            _viewModel.LoadLayout();
            
            // 注册全局热键
            var hotkeyManager = App.Services.GetService(typeof(Fences6.Core.HotkeyManager)) as Fences6.Core.HotkeyManager;
            hotkeyManager?.RegisterHotkey();
        }
        
        private IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            const int WM_GETMINMAXINFO = 0x0024;
            
            if (msg == WM_GETMINMAXINFO)
            {
                // 最大化时恢复为窗口模式
                if (WindowState == WindowState.Maximized)
                {
                    MaximizeButton.Content = "\uE922";
                }
            }
            
            return IntPtr.Zero;
        }
        
        #region Window Control Events
        
        private void MinimizeButton_Click(object sender, RoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }
        
        private void MaximizeButton_Click(object sender, RoutedEventArgs e)
        {
            if (WindowState == WindowState.Maximized)
            {
                WindowState = WindowState.Normal;
                MaximizeButton.Content = "\uE922";
            }
            else
            {
                WindowState = WindowState.Maximized;
                MaximizeButton.Content = "\uE923";
            }
        }
        
        private void CloseButton_Click(object sender, RoutedEventArgs e)
        {
            // 隐藏到托盘而不是关闭
            Hide();
        }
        
        #endregion
        
        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
        {
            base.OnMouseLeftButtonDown(e);
            
            // 拖动窗口
            if (e.ChangedButton == MouseButton.Left)
            {
                try
                {
                    DragMove();
                }
                catch { }
            }
        }
        
        protected override void OnStateChanged(EventArgs e)
        {
            base.OnStateChanged(e);
            
            if (MaximizeButton != null)
            {
                MaximizeButton.Content = WindowState == WindowState.Maximized ? "\uE923" : "\uE922";
            }
        }
        
        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            // 阻止关闭，改为隐藏
            e.Cancel = true;
            Hide();
        }
    }
}
