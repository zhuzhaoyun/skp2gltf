# 项目目录结构优化方案

## 新的目录结构

```
/
├── cmake/                    # CMake配置文件
│   └── sketchup-sdk.cmake    # SketchUp SDK配置
├── include/                  # 公共头文件
│   ├── exportgltf/          # GLTF导出相关头文件
│   ├── gltflib/             # GLTF库头文件
│   └── skp2xml/             # SKP到XML转换头文件
├── src/                     # 源代码
│   ├── exportgltf/          # GLTF导出实现
│   ├── gltflib/             # GLTF库实现
│   ├── skp2xml/             # SKP到XML转换实现
│   └── main.cpp             # 主程序入口
├── thirdparty/             # 第三方依赖
│   ├── SketchUpAPI/         # SketchUp SDK
│   ├── draco/               # Draco压缩库
│   ├── nlohmann/            # JSON库
│   ├── pugixml/             # XML解析库
│   ├── rapidjson/           # JSON解析库
│   ├── rapidxml/            # XML解析库
│   ├── tinygltf/            # GLTF解析库
│   └── tinyxml2/            # XML解析库
├── lib/                    # 库文件
└── CMakeLists.txt           # 主CMake配置文件
```

## 调整说明

1. 将所有源代码统一移至 src 目录
2. 将所有头文件统一移至 include 目录
3. 将第三方库统一移至 thirdparty 目录
4. 将CMake配置文件移至 cmake 目录
5. 删除冗余的目录和文件
6. 优化CMakeLists.txt的配置