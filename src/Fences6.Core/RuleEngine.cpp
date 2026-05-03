#include "pch.h"
#include "RuleEngine.h"
#include <ShlObj.h>
#include <sys/stat.h>

namespace Fences6
{
    RuleEngine::RuleEngine()
    {
        m_rules = gcnew System::Collections::Generic::List<FenceRule^>();
        m_syncRoot = gcnew System::Object();
    }

    void RuleEngine::AddRule(FenceRule^ rule)
    {
        if (rule == nullptr)
            throw gcnew System::ArgumentNullException("rule");

        if (System::String::IsNullOrEmpty(rule->Id))
        {
            rule->Id = System::Guid::NewGuid().ToString();
        }

        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            m_rules->Add(rule);
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    void RuleEngine::RemoveRule(System::String^ ruleId)
    {
        if (System::String::IsNullOrEmpty(ruleId))
            return;

        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            FenceRule^ toRemove = nullptr;
            for each (FenceRule^ rule in m_rules)
            {
                if (rule->Id == ruleId)
                {
                    toRemove = rule;
                    break;
                }
            }
            if (toRemove != nullptr)
            {
                m_rules->Remove(toRemove);
            }
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    void RuleEngine::UpdateRule(FenceRule^ rule)
    {
        if (rule == nullptr || System::String::IsNullOrEmpty(rule->Id))
            return;

        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            for (int i = 0; i < m_rules->Count; i++)
            {
                if (m_rules[i]->Id == rule->Id)
                {
                    rule->ModifiedTime = System::DateTime::Now;
                    m_rules[i] = rule;
                    break;
                }
            }
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }

    FenceRule^ RuleEngine::GetRule(System::String^ ruleId)
    {
        if (System::String::IsNullOrEmpty(ruleId))
            return nullptr;

        for each (FenceRule^ rule in m_rules)
        {
            if (rule->Id == ruleId)
                return rule;
        }
        return nullptr;
    }

    System::String^ RuleEngine::MatchRules(System::String^ filePath)
    {
        if (System::String::IsNullOrEmpty(filePath))
            return nullptr;

        // 检查 OneShot 规则是否已处理过
        for each (FenceRule^ rule in m_rules)
        {
            if (rule->OneShot && rule->ProcessedFiles->Contains(filePath))
                return nullptr;
        }

        // 按优先级排序
        auto sortedRules = gcnew System::Collections::Generic::List<FenceRule^>(m_rules);
        sortedRules->Sort([](FenceRule^ a, FenceRule^ b) {
            return b->Priority - a->Priority;
        });

        for each (FenceRule^ rule in sortedRules)
        {
            if (!rule->Enabled) continue;

            bool allRequiredMatch = true;
            bool anyOptionalMatch = false;

            for each (RuleCondition^ cond in rule->Conditions)
            {
                if (!cond->Enabled) continue;

                bool condMatch = EvaluateCondition(cond, filePath);

                if (cond->IsRequired)
                {
                    if (!condMatch)
                    {
                        allRequiredMatch = false;
                        break;
                    }
                }
                else
                {
                    if (condMatch)
                        anyOptionalMatch = true;
                }
            }

            if (allRequiredMatch)
            {
                // 触发匹配事件
                RuleMatched(this, gcnew RuleMatchEventArgs(filePath, rule));

                // 如果是 OneShot 模式，记录处理过的文件
                if (rule->OneShot)
                {
                    rule->ProcessedFiles->Add(filePath);
                }

                return rule->TargetFenceId;
            }
        }

        return nullptr;
    }

    System::Collections::Generic::List<FenceRule^>^ RuleEngine::GetMatchingRules(System::String^ filePath)
    {
        auto matches = gcnew System::Collections::Generic::List<FenceRule^>();

        if (System::String::IsNullOrEmpty(filePath))
            return matches;

        for each (FenceRule^ rule in m_rules)
        {
            if (!rule->Enabled) continue;

            bool allMatch = true;
            for each (RuleCondition^ cond in rule->Conditions)
            {
                if (!cond->Enabled) continue;
                if (!EvaluateCondition(cond, filePath))
                {
                    allMatch = false;
                    break;
                }
            }

            if (allMatch)
                matches->Add(rule);
        }

        return matches;
    }

    bool RuleEngine::EvaluateCondition(RuleCondition^ condition, System::String^ filePath)
    {
        if (condition == nullptr || System::String::IsNullOrEmpty(filePath))
            return false;

        if (!EvaluateFileExists(filePath))
            return false;

        try
        {
            switch (condition->Type)
            {
            case RuleConditionType::FileExtension:
                {
                    auto ext = System::IO::Path::GetExtension(filePath)->ToLower();
                    auto target = condition->Value->ToLower();
                    if (!target->StartsWith("."))
                        target = "." + target;
                    
                    if (condition->Operator == ComparisonOperator::Equals)
                        return ext == target;
                    if (condition->Operator == ComparisonOperator::Contains)
                        return ext->Contains(target);
                    return false;
                }

            case RuleConditionType::FileNamePattern:
                {
                    auto fileName = System::IO::Path::GetFileName(filePath);
                    if (condition->Operator == ComparisonOperator::WildcardMatch)
                    {
                        return MatchesWildcard(fileName, condition->Value);
                    }
                    if (condition->Operator == ComparisonOperator::MatchesRegex)
                    {
                        return System::Text::RegularExpressions::Regex::IsMatch(
                            fileName, condition->Value,
                            System::Text::RegularExpressions::RegexOptions::IgnoreCase);
                    }
                    if (condition->Operator == ComparisonOperator::Contains)
                    {
                        return fileName->Contains(condition->Value);
                    }
                    return fileName->Equals(condition->Value, System::StringComparison::IgnoreCase);
                }

            case RuleConditionType::FileSize:
                {
                    auto size = GetFileSize(filePath);
                    auto target = System::Convert::ToInt64(condition->Value);
                    
                    switch (condition->Operator)
                    {
                    case ComparisonOperator::Equals: return size == target;
                    case ComparisonOperator::GreaterThan: return size > target;
                    case ComparisonOperator::LessThan: return size < target;
                    case ComparisonOperator::GreaterThanOrEqual: return size >= target;
                    case ComparisonOperator::LessThanOrEqual: return size <= target;
                    default: return false;
                    }
                }

            case RuleConditionType::FileCreatedTime:
                {
                    auto created = GetFileCreatedTime(filePath);
                    auto target = System::Convert::ToDateTime(condition->Value);
                    return CompareDateTime(created, target, condition->Operator);
                }

            case RuleConditionType::FileModifiedTime:
                {
                    auto modified = GetFileModifiedTime(filePath);
                    auto target = System::Convert::ToDateTime(condition->Value);
                    return CompareDateTime(modified, target, condition->Operator);
                }

            case RuleConditionType::FolderPath:
                {
                    auto folder = System::IO::Path::GetDirectoryName(filePath);
                    auto target = condition->Value->ToLower();
                    folder = folder->ToLower();
                    
                    if (condition->Operator == ComparisonOperator::Equals)
                        return folder == target;
                    if (condition->Operator == ComparisonOperator::Contains)
                        return folder->Contains(target);
                    if (condition->Operator == ComparisonOperator::WildcardMatch)
                        return MatchesWildcard(folder, condition->Value);
                    return false;
                }

            case RuleConditionType::FileAttributes:
                {
                    auto attrs = GetFileAttributes(filePath);
                    auto targetAttrs = System::Convert::ToInt32(condition->Value);
                    return (attrs & targetAttrs) != 0;
                }
            }
        }
        catch (System::Exception^)
        {
            return false;
        }

        return false;
    }

    bool RuleEngine::EvaluateFileExists(System::String^ filePath)
    {
        return System::IO::File::Exists(filePath) || System::IO::Directory::Exists(filePath);
    }

    System::DateTime RuleEngine::GetFileCreatedTime(System::String^ filePath)
    {
        return System::IO::File::GetCreationTime(filePath);
    }

    System::DateTime RuleEngine::GetFileModifiedTime(System::String^ filePath)
    {
        return System::IO::File::GetLastWriteTime(filePath);
    }

    System::Int64 RuleEngine::GetFileSize(System::String^ filePath)
    {
        auto info = gcnew System::IO::FileInfo(filePath);
        return info->Length;
    }

    DWORD RuleEngine::GetFileAttributes(System::String^ filePath)
    {
        pin_ptr<const wchar_t> str = PtrToStringChars(filePath);
        return GetFileAttributesW(str);
    }

    void RuleEngine::ApplyRule(FenceRule^ rule, System::String^ filePath)
    {
        if (rule == nullptr || System::String::IsNullOrEmpty(filePath))
            return;

        try
        {
            switch (rule->Action)
            {
            case RuleActionType::MoveToFence:
                {
                    if (!System::String::IsNullOrEmpty(rule->TargetFenceId))
                    {
                        // 获取目标分组路径
                        auto targetDir = GetTargetDirectory(rule->TargetFenceId);
                        if (!System::String::IsNullOrEmpty(targetDir))
                        {
                            auto fileName = System::IO::Path::GetFileName(filePath);
                            auto destPath = System::IO::Path::Combine(targetDir, fileName);
                            System::IO::File::Move(filePath, destPath);
                        }
                    }
                    break;
                }

            case RuleActionType::CopyToFence:
                {
                    if (!System::String::IsNullOrEmpty(rule->TargetFenceId))
                    {
                        auto targetDir = GetTargetDirectory(rule->TargetFenceId);
                        if (!System::String::IsNullOrEmpty(targetDir))
                        {
                            auto fileName = System::IO::Path::GetFileName(filePath);
                            auto destPath = System::IO::Path::Combine(targetDir, fileName);
                            System::IO::File::Copy(filePath, destPath, true);
                        }
                    }
                    break;
                }

            case RuleActionType::Highlight:
                {
                    // 高亮显示效果由UI层处理
                    break;
                }

            case RuleActionType::Execute:
                {
                    if (!System::String::IsNullOrEmpty(rule->ActionArgument))
                    {
                        System::Diagnostics::Process::Start(rule->ActionArgument, filePath);
                    }
                    break;
                }

            case RuleActionType::ShowNotification:
                {
                    if (!System::String::IsNullOrEmpty(rule->ActionArgument))
                    {
                        // 使用系统通知
                        ShowSystemNotification(rule->Name, rule->ActionArgument);
                    }
                    break;
                }

            case RuleActionType::Delete:
                {
                    System::IO::File::Delete(filePath);
                    break;
                }
            }

            // 触发执行事件
            RuleApplied(this, gcnew RuleMatchEventArgs(filePath, rule));
        }
        catch (System::Exception^ ex)
        {
            System::Diagnostics::Debug::WriteLine("ApplyRule failed: " + ex->Message);
        }
    }

    void RuleEngine::ImportRules(System::String^ filePath)
    {
        try
        {
            if (!System::IO::File::Exists(filePath))
                return;

            auto json = System::IO::File::ReadAllText(filePath);
            auto serializer = gcnew System::Runtime::Serialization::Json::DataContractJsonSerializer(
                m_rules->GetType());
            auto stream = gcnew System::IO::MemoryStream(
                System::Text::Encoding::UTF8->GetBytes(json));
            auto imported = safe_cast<System::Collections::Generic::List<FenceRule^>^>(
                serializer->ReadObject(stream));
            stream->Close();

            if (imported != nullptr)
            {
                for each (FenceRule^ rule in imported)
                {
                    AddRule(rule);
                }
            }
        }
        catch (System::Exception^ ex)
        {
            System::Diagnostics::Debug::WriteLine("ImportRules failed: " + ex->Message);
        }
    }

    void RuleEngine::ExportRules(System::String^ filePath)
    {
        try
        {
            auto serializer = gcnew System::Runtime::Serialization::Json::DataContractJsonSerializer(
                m_rules->GetType());
            auto stream = gcnew System::IO::MemoryStream();
            serializer->WriteObject(stream, m_rules);
            
            auto json = System::Text::Encoding::UTF8->GetString(stream->ToArray());
            System::IO::File::WriteAllText(filePath, json);
            stream->Close();
        }
        catch (System::Exception^ ex)
        {
            System::Diagnostics::Debug::WriteLine("ExportRules failed: " + ex->Message);
        }
    }

    void RuleEngine::ClearRules()
    {
        System::Threading::Monitor::Enter(m_syncRoot);
        try
        {
            m_rules->Clear();
        }
        finally
        {
            System::Threading::Monitor::Exit(m_syncRoot);
        }
    }
}
