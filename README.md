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
    -   ```GameApp.h``` 、```GameApp.cpp``` ：定义了 ```GameApp``` 类，集成自 ```d3dApp``` 类，实现项目整体的功能。
    -   ``` Vertex.h``` 、 ```Vertex.cpp``` ：定义了与着色器交互需要用到的一些顶点结构体和输入布局数据。
    -   ```Geometry.h``` ：生成项目中用到的几种几何体网格模型。
    -   ``` LightHelper.h``` ：定义光照相关结构体数据。
    -   ```Camera.h``` 、``` Camera.cpp``` ：摄像机类相关代码，其中包括第一人称和第三人称两种相机类。
    -   ```RenderStates.h``` 、 ```RenderStates.cpp``` ：提供一些渲染状态：光栅化器状态、采样器状态、混合状态、深度/模板状态。
    -   ```Main.cpp``` ：程序入口。
-   ```HLSH``` 目录：着色器
-   ```Texture``` 目录：项目中用到的纹理文件（*.dds）

### 整体思路

小车由一个长方体和两个圆柱体组成，分别对三个物体进行平移和旋转的矩阵变换，即可组合得到一个小车模型。

用一个浮点型变量 **theta** 记录小车累计绕Y轴转动角度值，绘制小车前，进行一次旋转变换，使得小车转动到正确方向。当小车正在进行转向时，对前轮增加一个转动方向相应的角度值，即可实现前轮的转动效果。

用一个浮点型变量 **length** 记录小车累计前进距离，绘制轮胎前，对其进行一个绕自身转动轴转动相应角度的矩阵变换，即可实现车轮自转效果。

用一个浮点型变量 **velocity** 记录小车的车速，每次按 **W** 键时，速度加1.5，最大为3.0；每次按 **S** 键进行时，速度减1.5，最小为-3.0。

每次绘制前，根据角度值 **theta** 和速度 **velocity** 计算出沿 **x轴** 和 **z轴** 方向分别移动的距离，并根据当前位置计算出新的位置，对小车进行平移变换。

经过以上几次变换，最终可以实现基本正确的效果。