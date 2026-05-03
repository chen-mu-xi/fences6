#pragma once

#include <vcclr.h>
#include <Windows.h>
#include <vector>

namespace Fences6
{
    /// <summary>
    /// 矩形结构，用于表示分组边界
    /// </summary>
    [System::Serializable]
    public value struct FenceRect
    {
    public:
        double X;
        double Y;
        double Width;
        double Height;

        FenceRect(double x, double y, double width, double height)
            : X(x), Y(y), Width(width), Height(height) {}

        FenceRect() : X(0), Y(0), Width(0), Height(0) {}

        /// <summary>
        /// 左边坐标
        /// </summary>
        property double Left 
        { 
            double get() { return X; } 
        }

        /// <summary>
        /// 顶部坐标
        /// </summary>
        property double Top 
        { 
            double get() { return Y; } 
        }

        /// <summary>
        /// 右边坐标
        /// </summary>
        property double Right 
        { 
            double get() { return X + Width; } 
        }

        /// <summary>
        /// 底部坐标
        /// </summary>
        property double Bottom 
        { 
            double get() { return Y + Height; } 
        }

        /// <summary>
        /// 是否有效（宽高都大于0）
        /// </summary>
        property bool IsValid
        {
            bool get() { return Width > 0 && Height > 0; }
        }

        /// <summary>
        /// 转换为字符串表示
        /// </summary>
        virtual System::String^ ToString() override
        {
            return System::String::Format("{{X={0}, Y={1}, Width={2}, Height={3}}}", X, Y, Width, Height);
        }
    };

    /// <summary>
    /// Fence 分组类
    /// 表示桌面上的一个图标分组区域
    /// </summary>
    public ref class FenceGroup
    {
    public:
        /// <summary>
        /// 分组唯一标识符
        /// </summary>
        property System::String^ Id;

        /// <summary>
        /// 分组显示名称
        /// </summary>
        property System::String^ Name;

        /// <summary>
        /// 分组边界矩形
        /// </summary>
        property FenceRect Bounds;

        /// <summary>
        /// 图标路径列表
        /// </summary>
        property System::Collections::Generic::List<System::String^>^ Icons;

        /// <summary>
        /// 分组优先级（用于排序）
        /// </summary>
        property int Priority;

        /// <summary>
        /// 是否启用自动排列
        /// </summary>
        property bool AutoArrange;

        /// <summary>
        /// 分组是否可见
        /// </summary>
        property bool Visible;

        /// <summary>
        /// 构造函数
        /// </summary>
        FenceGroup()
        {
            Icons = gcnew System::Collections::Generic::List<System::String^>();
            Priority = 0;
            AutoArrange = true;
            Visible = true;
            Bounds = FenceRect(100, 100, 300, 200);
        }

        /// <summary>
        /// 获取图标数量
        /// </summary>
        property int IconCount
        {
            int get() { return Icons != nullptr ? Icons->Count : 0; }
        }

        /// <summary>
        /// 添加图标
        /// </summary>
        void AddIcon(System::String^ iconPath)
        {
            if (Icons == nullptr)
                Icons = gcnew System::Collections::Generic::List<System::String^>();
            Icons->Add(iconPath);
        }

        /// <summary>
        /// 移除图标
        /// </summary>
        void RemoveIcon(System::String^ iconPath)
        {
            if (Icons != nullptr)
                Icons->Remove(iconPath);
        }
    };

    /// <summary>
    /// 布局引擎
    /// 负责计算和管理桌面分组的布局
    /// </summary>
    public ref class LayoutEngine
    {
    public:
        /// <summary>
        /// 构造函数
        /// </summary>
        LayoutEngine();

        /// <summary>
        /// 析构函数
        /// </summary>
        ~LayoutEngine();

        /// <summary>
        /// 计算所有分组的布局
        /// </summary>
        /// <param name="groups">分组列表</param>
        void CalculateLayout(System::Collections::Generic::List<FenceGroup^>^ groups);

        /// <summary>
        /// 检测两个矩形是否重叠
        /// </summary>
        /// <param name="a">矩形A</param>
        /// <param name="b">矩形B</param>
        /// <returns>是否重叠</returns>
        static bool DetectOverlap(FenceRect a, FenceRect b);

        /// <summary>
        /// 自动排列分组
        /// </summary>
        /// <param name="groups">分组列表</param>
        void AutoArrange(System::Collections::Generic::List<FenceGroup^>^ groups);

        /// <summary>
        /// 保存布局到文件
        /// </summary>
        /// <param name="filePath">文件路径</param>
        void SaveLayout(System::String^ filePath);

        /// <summary>
        /// 从文件加载布局
        /// </summary>
        /// <param name="filePath">文件路径</param>
        void LoadLayout(System::String^ filePath);

        /// <summary>
        /// 网格大小
        /// </summary>
        property int GridSize 
        { 
            int get() { return m_gridSize; } 
            void set(int value) { m_gridSize = Math::Max(1, value); } 
        }

        /// <summary>
        /// 最小分组宽度
        /// </summary>
        property int MinGroupWidth
        {
            int get() { return m_minGroupWidth; }
            void set(int value) { m_minGroupWidth = Math::Max(50, value); }
        }

        /// <summary>
        /// 最小分组高度
        /// </summary>
        property int MinGroupHeight
        {
            int get() { return m_minGroupHeight; }
            void set(int value) { m_minGroupHeight = Math::Max(50, value); }
        }

        /// <summary>
        /// 分组间距
        /// </summary>
        property int GroupPadding
        {
            int get() { return m_groupPadding; }
            void set(int value) { m_groupPadding = Math::Max(0, value); }
        }

    private:
        int m_gridSize;
        int m_minGroupWidth;
        int m_minGroupHeight;
        int m_groupPadding;
        System::Collections::Generic::List<FenceRect>^ m_occupiedAreas;

        /// <summary>
        /// 碰撞检测与解决
        /// </summary>
        void ResolveCollisions(System::Collections::Generic::List<FenceGroup^>^ groups);

        /// <summary>
        /// 智能排列算法
        /// </summary>
        void SmartArrange(System::Collections::Generic::List<FenceGroup^>^ groups);

        /// <summary>
        /// 计算最优位置
        /// </summary>
        FenceRect FindBestPosition(FenceGroup^ group, System::Collections::Generic::List<FenceGroup^>^ existing);

        /// <summary>
        /// 获取网格对齐的位置
        /// </summary>
        FenceRect SnapToGrid(FenceRect rect);
    };
}
