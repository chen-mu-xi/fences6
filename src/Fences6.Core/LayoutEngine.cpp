#include "pch.h"
#include "LayoutEngine.h"
#include <algorithm>
#include <msclr/marshal_cppstd.h>

namespace Fences6
{
    LayoutEngine::LayoutEngine()
    {
        m_gridSize = 10;
        m_minGroupWidth = 200;
        m_minGroupHeight = 150;
        m_groupPadding = 10;
        m_occupiedAreas = gcnew System::Collections::Generic::List<FenceRect>();
    }

    LayoutEngine::~LayoutEngine()
    {
        m_occupiedAreas->Clear();
    }

    bool LayoutEngine::DetectOverlap(FenceRect a, FenceRect b)
    {
        // 使用边距避免过于紧密
        const double MARGIN = static_cast<double>(m_groupPadding);

        return !(a.Right + MARGIN <= b.Left ||
                 a.Left >= b.Right + MARGIN ||
                 a.Bottom + MARGIN <= b.Top ||
                 a.Top >= b.Bottom + MARGIN);
    }

    void LayoutEngine::CalculateLayout(System::Collections::Generic::List<FenceGroup^>^ groups)
    {
        if (groups == nullptr || groups->Count == 0)
            return;

        // 清空占用区域
        m_occupiedAreas->Clear();

        // 按优先级排序（高优先级在前）
        auto sortedGroups = gcnew System::Collections::Generic::List<FenceGroup^>(groups);
        sortedGroups->Sort([](FenceGroup^ a, FenceGroup^ b) {
            return b->Priority - a->Priority;
        });

        // 首先设置有效位置
        for each (FenceGroup^ group in sortedGroups)
        {
            if (!group->Bounds.IsValid)
            {
                group->Bounds = FenceRect(100, 100, m_minGroupWidth, m_minGroupHeight);
            }
            m_occupiedAreas->Add(group->Bounds);
        }

        // 自动排列
        AutoArrange(sortedGroups);

        // 解决碰撞
        ResolveCollisions(sortedGroups);

        // 网格对齐
        for each (FenceGroup^ group in sortedGroups)
        {
            group->Bounds = SnapToGrid(group->Bounds);
        }
    }

    void LayoutEngine::AutoArrange(System::Collections::Generic::List<FenceGroup^>^ groups)
    {
        if (groups == nullptr)
            return;

        // 收集启用了自动排列的分组
        auto autoArrangeGroups = gcnew System::Collections::Generic::List<FenceGroup^>();
        for each (FenceGroup^ group in groups)
        {
            if (group->AutoArrange)
            {
                autoArrangeGroups->Add(group);
            }
        }

        // 使用智能排列算法
        SmartArrange(autoArrangeGroups);
    }

    void LayoutEngine::SmartArrange(System::Collections::Generic::List<FenceGroup^>^ groups)
    {
        if (groups == nullptr || groups->Count == 0)
            return;

        // 使用简单的网格排列策略
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int columns = Math::Max(1, screenWidth / (m_minGroupWidth + m_groupPadding));

        int x = m_groupPadding;
        int y = m_groupPadding;
        int rowMaxY = 0;
        int col = 0;

        for each (FenceGroup^ group in groups)
        {
            // 检查是否需要换行
            if (col >= columns)
            {
                col = 0;
                x = m_groupPadding;
                y = rowMaxY + m_groupPadding;
            }

            // 设置分组位置
            group->Bounds = FenceRect(
                static_cast<double>(x),
                static_cast<double>(y),
                Math::Max(static_cast<double>(m_minGroupWidth), group->Bounds.Width),
                Math::Max(static_cast<double>(m_minGroupHeight), group->Bounds.Height)
            );

            // 更新位置
            rowMaxY = Math::Max(rowMaxY, static_cast<int>(group->Bounds.Bottom));
            x += static_cast<int>(group->Bounds.Width) + m_groupPadding;
            col++;
        }
    }

    FenceRect LayoutEngine::FindBestPosition(FenceGroup^ group, System::Collections::Generic::List<FenceGroup^>^ existing)
    {
        FenceRect bestPos = group->Bounds;
        bool found = false;

        // 在网格中寻找最优位置
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        for (int y = m_groupPadding; y < screenHeight - group->Bounds.Height && !found; y += m_gridSize)
        {
            for (int x = m_groupPadding; x < screenWidth - group->Bounds.Width && !found; x += m_gridSize)
            {
                FenceRect testRect(static_cast<double>(x), static_cast<double>(y),
                                  group->Bounds.Width, group->Bounds.Height);

                bool overlap = false;
                for each (FenceGroup^ existingGroup in existing)
                {
                    if (DetectOverlap(testRect, existingGroup->Bounds))
                    {
                        overlap = true;
                        break;
                    }
                }

                if (!overlap)
                {
                    bestPos = testRect;
                    found = true;
                }
            }
        }

        return bestPos;
    }

    void LayoutEngine::ResolveCollisions(System::Collections::Generic::List<FenceGroup^>^ groups)
    {
        if (groups == nullptr)
            return;

        bool hasCollision = true;
        int iterations = 0;
        const int MAX_ITERATIONS = 100;
        const int PUSH_DISTANCE = 10;

        while (hasCollision && iterations < MAX_ITERATIONS)
        {
            hasCollision = false;
            iterations++;

            for (int i = 0; i < groups->Count; i++)
            {
                for (int j = i + 1; j < groups->Count; j++)
                {
                    if (DetectOverlap(groups[i]->Bounds, groups[j]->Bounds))
                    {
                        hasCollision = true;

                        // 计算碰撞方向，将后面的分组推开
                        double dx = groups[j]->Bounds.X - groups[i]->Bounds.X;
                        double dy = groups[j]->Bounds.Y - groups[i]->Bounds.Y;

                        if (Math::Abs(dx) > Math::Abs(dy))
                        {
                            // 水平方向推开
                            if (dx > 0)
                            {
                                groups[j]->Bounds = FenceRect(
                                    groups[i]->Bounds.Right + m_groupPadding,
                                    groups[j]->Bounds.Y,
                                    groups[j]->Bounds.Width,
                                    groups[j]->Bounds.Height
                                );
                            }
                            else
                            {
                                groups[i]->Bounds = FenceRect(
                                    groups[j]->Bounds.Right + m_groupPadding,
                                    groups[i]->Bounds.Y,
                                    groups[i]->Bounds.Width,
                                    groups[i]->Bounds.Height
                                );
                            }
                        }
                        else
                        {
                            // 垂直方向推开
                            if (dy > 0)
                            {
                                groups[j]->Bounds = FenceRect(
                                    groups[j]->Bounds.X,
                                    groups[i]->Bounds.Bottom + m_groupPadding,
                                    groups[j]->Bounds.Width,
                                    groups[j]->Bounds.Height
                                );
                            }
                            else
                            {
                                groups[i]->Bounds = FenceRect(
                                    groups[i]->Bounds.X,
                                    groups[j]->Bounds.Bottom + m_groupPadding,
                                    groups[i]->Bounds.Width,
                                    groups[i]->Bounds.Height
                                );
                            }
                        }
                    }
                }
            }
        }
    }

    FenceRect LayoutEngine::SnapToGrid(FenceRect rect)
    {
        int gridX = static_cast<int>(Math::Round(static_cast<double>(rect.X) / m_gridSize)) * m_gridSize;
        int gridY = static_cast<int>(Math::Round(static_cast<double>(rect.Y) / m_gridSize)) * m_gridSize;
        int gridW = static_cast<int>(Math::Round(static_cast<double>(rect.Width) / m_gridSize)) * m_gridSize;
        int gridH = static_cast<int>(Math::Round(static_cast<double>(rect.Height) / m_gridSize)) * m_gridSize;

        // 确保最小尺寸
        gridW = Math::Max(gridW, m_minGroupWidth);
        gridH = Math::Max(gridH, m_minGroupHeight);

        return FenceRect(static_cast<double>(gridX), static_cast<double>(gridY),
                        static_cast<double>(gridW), static_cast<double>(gridH));
    }

    void LayoutEngine::SaveLayout(System::String^ filePath)
    {
        try
        {
            // 使用 JSON 序列化保存布局
            auto serializer = gcnew System::Runtime.Serialization::Json::DataContractJsonSerializer(
                m_occupiedAreas->GetType());

            auto stream = gcnew System::IO::FileStream(filePath, System::IO::FileMode::Create);
            serializer->WriteObject(stream, m_occupiedAreas);
            stream->Close();
        }
        catch (System::Exception^ ex)
        {
            System::Diagnostics::Debug::WriteLine("SaveLayout failed: " + ex->Message);
        }
    }

    void LayoutEngine::LoadLayout(System::String^ filePath)
    {
        try
        {
            if (!System::IO::File::Exists(filePath))
                return;

            // 从 JSON 文件加载布局
            auto serializer = gcnew System::Runtime::Serialization::Json::DataContractJsonSerializer(
                m_occupiedAreas->GetType());

            auto stream = gcnew System::IO::FileStream(filePath, System::IO::FileMode::Open);
            auto data = safe_cast<System::Collections::Generic::List<FenceRect>^>(serializer->ReadObject(stream));
            stream->Close();

            if (data != nullptr)
            {
                m_occupiedAreas = data;
            }
        }
        catch (System::Exception^ ex)
        {
            System::Diagnostics::Debug::WriteLine("LoadLayout failed: " + ex->Message);
        }
    }
}
