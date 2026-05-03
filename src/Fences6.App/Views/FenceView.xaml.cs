using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using Fences6.App.ViewModels;

namespace Fences6.App.Views
{
    public partial class FenceView : UserControl
    {
        #region 拖拽状态

        private bool _isDragging;
        private Point _dragStartPoint;
        private Point _initialPosition;
        private double _initialLeft;
        private double _initialTop;

        #endregion

        #region 大小调整状态

        private bool _isResizing;
        private Point _resizeStartPoint;
        private double _initialWidth;
        private double _initialHeight;
        private const double MinWidth = 150;
        private const double MinHeight = 100;

        #endregion

        public FenceView()
        {
            InitializeComponent();
        }

        #region 标题栏拖拽

        private void Header_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (DataContext is FenceViewModel fenceVm)
            {
                // 如果正在编辑名称，不处理拖拽
                if (fenceVm.IsEditing)
                {
                    return;
                }

                // 开始拖拽
                _isDragging = true;
                _dragStartPoint = e.GetPosition(Parent as UIElement);
                _initialLeft = Canvas.GetLeft(this);
                _initialTop = Canvas.GetTop(this);

                // 如果初始位置无效，设置默认位置
                if (double.IsNaN(_initialLeft)) _initialLeft = 0;
                if (double.IsNaN(_initialTop)) _initialTop = 0;

                _initialPosition = new Point(_initialLeft, _initialTop);

                // 标记选中
                fenceVm.IsSelected = true;

                // 捕获鼠标
                (sender as UIElement).CaptureMouse();
                e.Handled = true;
            }
        }

        private void Header_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (_isDragging)
            {
                _isDragging = false;
                (sender as UIElement).ReleaseMouseCapture();

                // 保存位置
                if (DataContext is FenceViewModel fenceVm)
                {
                    fenceVm.Bounds = new FenceRect
                    {
                        X = Canvas.GetLeft(this),
                        Y = Canvas.GetTop(this),
                        Width = ActualWidth,
                        Height = ActualHeight
                    };
                }
            }
        }

        private void Header_MouseMove(object sender, MouseEventArgs e)
        {
            if (_isDragging && e.LeftButton == MouseButtonState.Pressed)
            {
                var currentPoint = e.GetPosition(Parent as UIElement);
                var deltaX = currentPoint.X - _dragStartPoint.X;
                var deltaY = currentPoint.Y - _dragStartPoint.Y;

                var newLeft = Math.Max(0, _initialPosition.X + deltaX);
                var newTop = Math.Max(0, _initialPosition.Y + deltaY);

                Canvas.SetLeft(this, newLeft);
                Canvas.SetTop(this, newTop);
            }
        }

        #endregion

        #region 调整大小

        private void ResizeThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            var newWidth = Math.Max(MinWidth, ActualWidth + e.HorizontalChange);
            var newHeight = Math.Max(MinHeight, ActualHeight + e.VerticalChange);

            // 检查边界
            var parent = Parent as Canvas;
            if (parent != null)
            {
                newWidth = Math.Min(newWidth, parent.ActualWidth - Canvas.GetLeft(this));
                newHeight = Math.Min(newHeight, parent.ActualHeight - Canvas.GetTop(this));
            }

            Width = newWidth;
            Height = newHeight;

            // 更新 ViewModel
            if (DataContext is FenceViewModel fenceVm)
            {
                fenceVm.Bounds = new FenceRect
                {
                    X = Canvas.GetLeft(this),
                    Y = Canvas.GetTop(this),
                    Width = newWidth,
                    Height = newHeight
                };
            }
        }

        #endregion

        #region 标题编辑

        private void TitleTextBox_LostFocus(object sender, RoutedEventArgs e)
        {
            FinishEditing();
        }

        private void TitleTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter || e.Key == Key.Escape)
            {
                FinishEditing();
                e.Handled = true;
            }
        }

        private void FinishEditing()
        {
            if (DataContext is FenceViewModel fenceVm && fenceVm.IsEditing)
            {
                fenceVm.IsEditing = false;
                TitleText.Visibility = Visibility.Visible;
                TitleTextBox.Visibility = Visibility.Collapsed;
            }
        }

        #endregion

        #region 菜单操作

        private void PinButton_Click(object sender, RoutedEventArgs e)
        {
            if (DataContext is FenceViewModel fenceVm)
            {
                fenceVm.IsPinned = !fenceVm.IsPinned;
                PinButton.Content = fenceVm.IsPinned ? "📍" : "📌";
            }
        }

        private void MenuButton_Click(object sender, RoutedEventArgs e)
        {
            if (sender is Button button && button.ContextMenu != null)
            {
                button.ContextMenu.PlacementTarget = button;
                button.ContextMenu.IsOpen = true;
            }
        }

        private void ChangeIconSize_Click(object sender, RoutedEventArgs e)
        {
            // 打开图标大小选择对话框
            var sizes = new[] { "大图标 (48px)", "中图标 (32px)", "小图标 (16px)" };
            // TODO: 实现图标大小选择
        }

        private void ChangeBackground_Click(object sender, RoutedEventArgs e)
        {
            // 打开背景颜色选择器
            // TODO: 实现颜色选择器
        }

        private void LockFence_Click(object sender, RoutedEventArgs e)
        {
            if (DataContext is FenceViewModel fenceVm)
            {
                fenceVm.IsLocked = !fenceVm.IsLocked;
            }
        }

        #endregion

        #region 公共方法

        public void StartEditing()
        {
            if (DataContext is FenceViewModel fenceVm)
            {
                fenceVm.IsEditing = true;
                TitleText.Visibility = Visibility.Collapsed;
                TitleTextBox.Visibility = Visibility.Visible;
                TitleTextBox.Focus();
                TitleTextBox.SelectAll();
            }
        }

        #endregion
    }
}
