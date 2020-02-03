### 概况

-   API：DirectX 11
-   开发工具：Visual Studio 2017

-   开发平台：Windows 10 
-   作业难度：进阶版
-   运行说明：点击 ```bin``` 目录下的 ```3DHomework.exe``` 文件即可执行程序

### 源码结构说明

-   根目录：C++ 代码
    -   ```DDSTextureLoader.h``` 、 ```DDSTextureLoader.cpp``` ：从微软官方项目中提取，用于纹理文件的读取。
    -   ``` Mouse.h``` 、 ```Mouse.cpp``` 、 ```Keyboard.h``` 、 ```Keyboard.cpp``` ：从微软官方项目中提取，用于鼠标键盘输入的处理。
    -   ```d3dUtil.h``` 、 ```d3dUtil.cpp``` ：负责着色器编译相关操作。
    -   ```d3dApp.h``` 、```d3dApp.cpp``` ：程序的整体框架，包含了需要的静态库，负责Window窗口的初始化、Direct2D的初始化、Direct3D的初始化等操作，定义了 ```D3DAPP``` 类，其中比较重要的几个成员函数：
        -   **virtual bool Init();** ：初始化窗口、Direct2D和Direct3D部分。
        -   **virtual void OnResize();** ：窗口大小变动的时候调用，重新设置一些资源信息。
        -   **virtual void UpdateScene() = 0;** ：进行每一帧的更新，需要在子类中实现。
        -   **virtual void DrawScene() = 0;** ：进行每一帧的绘制，需要在子类中实现。
        -   **virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);**  ：窗口的消息回调函数。
    -   ```GameApp.h``` 、```GameApp.cpp``` ：定义了 ```D3DAPP``` 的子类 ```GameApp``` ，实现项目整体的功能。
    -   ``` Vertex.h``` 、 ```Vertex.cpp``` ：定义了与着色器交互需要用到的一些顶点结构体和输入布局数据。
    -   ```Geometry.h``` ：生成项目中用到的集中几何体网格模型。
    -   ``` LightHelper.h``` ：定义光照相关结构体数据。
    -   ```Camera.h``` 、``` Camera.cpp``` ：摄像机类相关代码。
    -   ```RenderStates.h``` 、 ```RenderStates.cpp``` ：提供一些渲染状态：光栅化器状态、采样器状态、混合状态、深度/模板状态。
    -   ```Main.cpp``` ：程序入口。
-   ```HLSH``` 目录：着色器
-   ```Texture``` 目录：项目中用到的纹理文件（*.dds）