using System;
using System.Windows;
using System.Windows.Controls;
using Hardcodet.Wpf.TaskbarNotification;
using System.Drawing;
using Fences6.App.Commands;

namespace Fences6.App.Services
{
    public class TrayService : IDisposable
    {
        private TaskbarIcon? m_trayIcon;
        private bool m_disposed;
        
        public event EventHandler? ShowWindowRequested;
        public event EventHandler? ExitRequested;
        public event EventHandler? SettingsRequested;
        
        public void Initialize()
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                m_trayIcon = new TaskbarIcon
                {
                    ToolTipText = "Fences6 - 桌面整理工具",
                    Visibility = Visibility.Visible
                };
                
                // 设置图标
                m_trayIcon.Icon = CreateDefaultIcon();
                
                // 创建右键菜单
                m_trayIcon.ContextMenu = CreateContextMenu();
                
                // 双击打开主窗口
                m_trayIcon.TrayMouseDoubleClick += (s, e) => ShowWindowRequested?.Invoke(this, EventArgs.Empty);
            });
        }
        
        private ContextMenu CreateContextMenu()
        {
            var menu = new ContextMenu();
            
            menu.Items.Add(new MenuItem
            {
                Header = "打开 Fences6",
                Command = new RelayCommand(() => ShowWindowRequested?.Invoke(this, EventArgs.Empty))
            });
            
            menu.Items.Add(new Separator());
            
            menu.Items.Add(new MenuItem { Header = "新建分组" });
            menu.Items.Add(new MenuItem { Header = "锁定布局" });
            menu.Items.Add(new MenuItem { Header = "自动排列" });
            
            menu.Items.Add(new Separator());
            
            menu.Items.Add(new MenuItem
            {
                Header = "设置...",
                Command = new RelayCommand(() => SettingsRequested?.Invoke(this, EventArgs.Empty))
            });
            
            menu.Items.Add(new Separator());
            
            menu.Items.Add(new MenuItem
            {
                Header = "退出",
                Command = new RelayCommand(() => ExitRequested?.Invoke(this, EventArgs.Empty))
            });
            
            return menu;
        }
        
        private Icon CreateDefaultIcon()
        {
            // 创建一个简单的默认图标
            using var bitmap = new Bitmap(32, 32);
            using var g = Graphics.FromImage(bitmap);
            
            g.Clear(Color.Transparent);
            using var brush = new SolidBrush(Color.FromArgb(0, 120, 215));
            g.FillRectangle(brush, 4, 4, 24, 24);
            using var pen = new Pen(Color.White, 2);
            g.DrawLine(pen, 8, 16, 24, 16);
            g.DrawLine(pen, 16, 8, 16, 24);
            
            return Icon.FromHandle(bitmap.GetHicon());
        }
        
        public void ShowBalloonTip(string title, string message, BalloonIcon icon = BalloonIcon.Info)
        {
            m_trayIcon?.ShowBalloonTip(title, message, icon);
        }
        
        public void Dispose()
        {
            if (!m_disposed)
            {
                m_trayIcon?.Dispose();
                m_disposed = true;
            }
        }
    }
}
