#pragma once
#include <Windows.h>

namespace Fences6
{
    /// <summary>
    /// 规则条件类型
    /// </summary>
    public enum class RuleConditionType
    {
        /// <summary>文件扩展名</summary>
        FileExtension,
        
        /// <summary>文件名模式（支持通配符）</summary>
        FileNamePattern,
        
        /// <summary>文件大小</summary>
        FileSize,
        
        /// <summary>文件创建时间</summary>
        FileCreatedTime,
        
        /// <summary>所在文件夹路径</summary>
        FolderPath,
        
        /// <summary>文件修改时间</summary>
        FileModifiedTime,
        
        /// <summary>文件属性</summary>
        FileAttributes,
    };

    /// <summary>
    /// 规则动作类型
    /// </summary>
    public enum class RuleActionType
    {
        /// <summary>移动文件到分组</summary>
        MoveToFence,
        
        /// <summary>复制文件到分组</summary>
        CopyToFence,
        
        /// <summary>高亮显示文件</summary>
        Highlight,
        
        /// <summary>执行程序</summary>
        Execute,
        
        /// <summary>显示通知</summary>
        ShowNotification,
        
        /// <summary>删除文件</summary>
        Delete,
    };

    /// <summary>
    /// 比较运算符
    /// </summary>
    public enum class ComparisonOperator
    {
        /// <summary>等于</summary>
        Equals,
        
        /// <summary>包含</summary>
        Contains,
        
        /// <summary>大于</summary>
        GreaterThan,
        
        /// <summary>小于</summary>
        LessThan,
        
        /// <summary>大于等于</summary>
        GreaterThanOrEqual,
        
        /// <summary>小于等于</summary>
        LessThanOrEqual,
        
        /// <summary>匹配正则表达式</summary>
        MatchesRegex,
        
        /// <summary>通配符匹配</summary>
        WildcardMatch,
    };

    /// <summary>
    /// 规则条件
    /// 定义规则匹配的条件
    /// </summary>
    [System::Serializable]
    public ref class RuleCondition
    {
    public:
        /// <summary>
        /// 条件类型
        /// </summary>
        property RuleConditionType Type;

        /// <summary>
        /// 条件值
        /// </summary>
        property System::String^ Value;

        /// <summary>
        /// 比较运算符
        /// </summary>
        property ComparisonOperator Operator;

        /// <summary>
        /// 是否启用此条件
        /// </summary>
        property bool Enabled;

        /// <summary>
        /// 逻辑组合类型（AND/OR）
        /// </summary>
        property bool IsRequired;

        RuleCondition()
        {
            Enabled = true;
            IsRequired = true;
            Operator = ComparisonOperator::Equals;
        }

        RuleCondition(RuleConditionType type, System::String^ value)
        {
            Type = type;
            Value = value;
            Enabled = true;
            IsRequired = true;
            Operator = ComparisonOperator::Equals;
        }
    };

    /// <summary>
    /// 规则
    /// 定义文件自动分类的行为规则
    /// </summary>
    [System::Serializable]
    public ref class FenceRule
    {
    public:
        /// <summary>
        /// 规则唯一标识符
        /// </summary>
        property System::String^ Id;

        /// <summary>
        /// 规则名称
        /// </summary>
        property System::String^ Name;

        /// <summary>
        /// 规则描述
        /// </summary>
        property System::String^ Description;

        /// <summary>
        /// 是否启用规则
        /// </summary>
        property bool Enabled;

        /// <summary>
        /// 规则条件列表
        /// </summary>
        property System::Collections::Generic::List<RuleCondition^>^ Conditions;

        /// <summary>
        /// 满足条件时执行的动作
        /// </summary>
        property RuleActionType Action;

        /// <summary>
        /// 目标分组ID（用于MoveToFence/CopyToFence动作）
        /// </summary>
        property System::String^ TargetFenceId;

        /// <summary>
        /// 规则优先级（数值越大优先级越高）
        /// </summary>
        property int Priority;

        /// <summary>
        /// 动作参数（如程序路径、通知内容等）
        /// </summary>
        property System::String^ ActionArgument;

        /// <summary>
        /// 规则创建时间
        /// </summary>
        property System::DateTime CreatedTime;

        /// <summary>
        /// 规则最后修改时间
        /// </summary>
        property System::DateTime ModifiedTime;

        /// <summary>
        /// 是否只在有新文件时执行一次
        /// </summary>
        property bool OneShot;

        /// <summary>
        /// 已处理的文件列表（用于OneShot模式）
        /// </summary>
        property System::Collections::Generic::HashSet<System::String^>^ ProcessedFiles;

        /// <summary>
        /// 构造函数
        /// </summary>
        FenceRule()
        {
            Conditions = gcnew System::Collections::Generic::List<RuleCondition^>();
            Enabled = true;
            Priority = 0;
            OneShot = false;
            ProcessedFiles = gcnew System::Collections::Generic::HashSet<System::String^>();
            CreatedTime = System::DateTime::Now;
            ModifiedTime = System::DateTime::Now;
        }

        /// <summary>
        /// 添加条件
        /// </summary>
        void AddCondition(RuleCondition^ condition)
        {
            Conditions->Add(condition);
            ModifiedTime = System::DateTime::Now;
        }

        /// <summary>
        /// 清除所有条件
        /// </summary>
        void ClearConditions()
        {
            Conditions->Clear();
            ModifiedTime = System::DateTime::Now;
        }
    };

    /// <summary>
    /// 规则匹配事件参数
    /// </summary>
    public ref class RuleMatchEventArgs : public System::EventArgs
    {
    public:
        property System::String^ FilePath;
        property FenceRule^ MatchedRule;
        property System::DateTime MatchTime;

        RuleMatchEventArgs(System::String^ filePath, FenceRule^ rule)
        {
            FilePath = filePath;
            MatchedRule = rule;
            MatchTime = System::DateTime::Now;
        }
    };

    /// <summary>
    /// 规则引擎
    /// 负责管理和执行文件分类规则
    /// </summary>
    public ref class RuleEngine
    {
    public:
        /// <summary>
        /// 构造函数
        /// </summary>
        RuleEngine();

        /// <summary>
        /// 添加规则
        /// </summary>
        /// <param name="rule">要添加的规则</param>
        void AddRule(FenceRule^ rule);

        /// <summary>
        /// 移除规则
        /// </summary>
        /// <param name="ruleId">规则ID</param>
        void RemoveRule(System::String^ ruleId);

        /// <summary>
        /// 更新规则
        /// </summary>
        /// <param name="rule">更新后的规则</param>
        void UpdateRule(FenceRule^ rule);

        /// <summary>
        /// 获取规则
        /// </summary>
        /// <param name="ruleId">规则ID</param>
        /// <returns>规则对象，如果不存在返回nullptr</returns>
        FenceRule^ GetRule(System::String^ ruleId);

        /// <summary>
        /// 获取所有规则
        /// </summary>
        property System::Collections::Generic::List<FenceRule^>^ Rules;

        /// <summary>
        /// 规则数量
        /// </summary>
        property int Count
        {
            int get() { return m_rules->Count; }
        }

        /// <summary>
        /// 检查文件是否匹配任何规则
        /// </summary>
        /// <param name="filePath">文件路径</param>
        /// <returns>匹配的目标分组ID，如果没有匹配返回nullptr</returns>
        System::String^ MatchRules(System::String^ filePath);

        /// <summary>
        /// 获取匹配的文件规则
        /// </summary>
        /// <param name="filePath">文件路径</param>
        /// <returns>匹配的规则列表</returns>
        System::Collections::Generic::List<FenceRule^>^ GetMatchingRules(System::String^ filePath);

        /// <summary>
        /// 执行规则动作
        /// </summary>
        /// <param name="rule">要执行的规则</param>
        /// <param name="filePath">目标文件路径</param>
        void ApplyRule(FenceRule^ rule, System::String^ filePath);

        /// <summary>
        /// 导入规则
        /// </summary>
        /// <param name="filePath">规则文件路径</param>
        void ImportRules(System::String^ filePath);

        /// <summary>
        /// 导出规则
        /// </summary>
        /// <param name="filePath">目标文件路径</param>
        void ExportRules(System::String^ filePath);

        /// <summary>
        /// 清除所有规则
        /// </summary>
        void ClearRules();

        /// <summary>
        /// 规则匹配事件
        /// </summary>
        event System::EventHandler<RuleMatchEventArgs^>^ RuleMatched;

        /// <summary>
        /// 规则执行事件
        /// </summary>
        event System::EventHandler<RuleMatchEventArgs^>^ RuleApplied;

    private:
        System::Collections::Generic::List<FenceRule^>^ m_rules;
        System::Object^ m_syncRoot;

        bool EvaluateCondition(RuleCondition^ condition, System::String^ filePath);
        bool EvaluateFileExists(System::String^ filePath);
        System::DateTime GetFileCreatedTime(System::String^ filePath);
        System::DateTime GetFileModifiedTime(System::String^ filePath);
        System::Int64 GetFileSize(System::String^ filePath);
        DWORD GetFileAttributes(System::String^ filePath);
    };
}
