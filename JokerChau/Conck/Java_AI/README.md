# Conck - JavaFX 通讯录应用

这是一个基于 JavaFX 纯代码构建的桌面通讯录应用，数据通过本地 JSON 文件进行持久化。项目使用 Maven 进行依赖与构建管理。

## ✨ 功能特性
- **纯 Java 构建 UI**：不使用 FXML，界面完全由 Java 代码声明式编写。
- **联系人管理**：支持添加、修改、删除联系人。
- **实时搜索**：输入框内容改变时，自动过滤表格数据。
- **排序功能**：支持按姓名（A-Z）或电话号码进行排序。
- **本地持久化**：所有数据自动保存至本地的 `contacts.json` 文件中。

## 🛠️ 技术栈
- **JDK**：25
- **UI 框架**：JavaFX 21.0.2 (javafx-controls)
- **JSON 处理**：Gson 2.11.0
- **构建工具**：Maven

## 📂 项目结构
```text
jonck/
├── pom.xml                               # Maven 项目配置
└── src/
    └── main/
        ├── java/
        │   ├── module-info.java          # Java 模块化配置
        │   └── com/main/
        │       ├── App.java              # JavaFX 主入口与 UI 逻辑
        │       ├── Launcher.java         # 仅用于打包 JAR 时的启动类
        │       ├── model/
        │       │   └── Contact.java      # 联系人数据模型 (Record)
        │       ├── repository/
        │       │   ├── ContactRepository.java      # 数据仓储接口
        │       │   └── FileContactRepository.java  # JSON 文件读写实现
        │       └── service/
        │           └── ContactService.java         # 核心业务逻辑
        └── resources/
            └── style.css                 # (可选) JavaFX CSS 样式表
```

## 🚀 如何运行
由于网络或环境问题导致打包失败，本项目目前直接通过 Maven 插件运行：
在 IDE 左侧的 **Maven** 面板中，展开 `jonck` -> `Plugins` -> `javafx`，双击 **`javafx:run`** 即可启动程序。

## 🎨 样式说明
在 `App.java` 中通过 `scene.getStylesheets().add(...)` 引入了 `style.css`（如果存在）。你可以通过修改 CSS 文件来调整按钮颜色、表格圆角等界面效果，而无需改动 Java 代码。