# Fences 6

> 一款类似 Stardock Fences 的 Windows 桌面整理工具。

## 自动构建

**构建状态**:[![Build](https://github.com/chen-mu-xi/fences6/actions/workflows/build.yml/badge.svg)](https://github.com/chen-mu-xi/fences6/actions)
[![Test Build](https://github.com/chen-mu-xi/fences6/actions/workflows/pr-test.yml/badge.svg)](https://github.com/chen-mu-xi/fences6/actions)

**下载最新版本**: [Releases](https://github.com/chen-mu-xi/fences6/releases)

---

## 如何使用

### 方式一：下载编译好的版本
1. 点击上方徽章进入 [Actions](https://github.com/chen-mu-xi/fences6/actions)
2. 选择最新的 workflow run
3. 下载 `Fences6-win-x64-xxx` artifact

### 方式二：从源码编译
```bash
# 克隆仓库
git clone https://github.com/chen-mu-xi/fences6.git
cd fences6

# 安装 .NET 8 SDK
# https://dotnet.microsoft.com/download/dotnet/8.0

# 还原依赖
dotnet restore

# 编译
dotnet build -c Release

# 发布单文件可执行文件
dotnet publish src/Fences6.App/Fences6.App.csproj `
  -c Release `
  -r win-x64 `
  --self-contained `
  -p:PublishSingleFile=true `
  -p:IncludeNativeLibrariesForSelfExtract=true `
  -o ./publish

# 运行
./publish/Fences6.App.exe
```

---

## 技术栈

- **前端框架**: WPF (.NET 6+)
- **核心逻辑**: C++/CLI 混合架构
- **原生组件**: C++ 本地库
- **MVVM 框架**: CommunityToolkit.Mvvm
- **依赖注入**: Microsoft.Extensions.DependencyInjection
- **数据库**: SQLite

## 项目结构

```
Fences6/
├── Fences6.sln                          # 解决方案文件
├── src/
│   ├── Fences6.App/                     # WPF 主应用
│   │   ├── Fences6.App.csproj           # 项目文件
│   │   ├── App.xaml / App.xaml.cs       # 应用入口
│   │   ├── MainWindow.xaml / .cs        # 主窗口
│   │   ├── ViewModels/                  # MVVM ViewModels
│   │   ├── Views/                       # 视图
│   │   ├── Services/                     # 应用服务
│   │   ├── Resources/                   # 资源文件
│   │   └── Converters/                   # 值转换器
│   ├── Fences6.Core/                    # C++/CLI 核心库
│   │   ├── Fences6.Core.vcxproj
│   │   ├── pch.h / pch.cpp             # 预编译头
│   │   ├── FenceCore.h/.cpp             # 核心入口
│   │   ├── LayoutEngine.h/.cpp          # 布局引擎
│   │   ├── RuleEngine.h/.cpp            # 规则引擎
│   │   ├── ConfigManager.h/.cpp         # 配置管理
│   │   └── HotkeyManager.h/.cpp         # 热键管理
│   └── Fences6.Native/                  # C++ 本地库
│       ├── Fences6.Native.vcxproj
│       ├── FileWatcher.h/.cpp           # 文件监控
│       └── DesktopIntegration.h/.cpp    # 桌面集成
├── resources/
│   └── icons/                           # 图标资源
├── Directory.Build.props                # 构建配置
├── Directory.Packages.props             # NuGet 包管理
└── README.md
```

## 主要功能

### 1. 桌面分组管理
- 创建、编辑、删除桌面分组
- 拖拽调整分组大小和位置
- 分组透明度和背景颜色设置

### 2. 自动布局
- 网格对齐
- 自动排列算法
- 碰撞检测与避免

### 3. 图标规则
- 按扩展名自动分类
- 按文件夹路径分类
- 按名称模式匹配
- 手动分配图标

### 4. 热键支持
- 全局快捷键切换显示/隐藏
- 可配置的快捷键绑定

### 5. 系统托盘
- 最小化到托盘
- 托盘菜单快捷操作
- 后台运行

### 6. 文件监控
- 实时监控桌面变化
- 自动更新图标列表

## 构建要求

- Visual Studio 2022 17.0+
- .NET 6.0 SDK
- Windows SDK 10.0.19041.0+
- C++ Build Tools (用于编译 C++/CLI 和本地库)

## 构建步骤

1. 克隆仓库
2. 使用 Visual Studio 打开 `Fences6.sln`
3. 还原 NuGet 包
4. 选择配置 (Debug/Release) 和平台 (x64/x86)
5. 构建解决方案

```bash
# 命令行构建
dotnet build Fences6.sln -c Release -r win-x64
```

## NuGet 包依赖

### Fences6.App (WPF 应用)
- CommunityToolkit.Mvvm 8.2.2
- Microsoft.Extensions.DependencyInjection 8.0.0
- Microsoft.Extensions.Hosting 8.0.0
- Hardcodet.NotifyIcon.Wpf 1.1.0
- System.Data.SQLite.Core 1.0.118
- Microsoft.Xaml.Behaviors.Wpf 1.1.77

### Fences6.Core (C++/CLI 核心库)
- Microsoft.Windows.CppWinRT 2.0.240111.0
- Microsoft.WindowsAppSDK 1.4.231115000
- Microsoft.Windows.SDK.BuildTools 10.0.22621.756

## 配置存储

- 位置: `%LOCALAPPDATA%\Fences6\`
- 格式: XML (config.xml)
- 内容: 应用设置、分组配置、图标规则

## 开发指南

### 添加新的核心服务
1. 在 `Fences6.Core` 中创建新的 .h/.cpp 文件
2. 定义 public ref class
3. 在 `Fences6.App` 的 `App.xaml.cs` 中注册服务

### 添加新的视图
1. 在 `Views/` 目录创建 XAML 文件
2. 创建对应的 ViewModel
3. 使用 MVVM Toolkit 绑定

### 调试技巧
- 使用 `System::Diagnostics::Debug::WriteLine` 输出调试信息
- 查看 `%LOCALAPPDATA%\Fences6\fences6.log` 日志
- 使用 Attach to Process 附加到运行中的进程

## 许可证

MIT License

## 贡献

欢迎提交 Issue 和 Pull Request！
