<p align="center">
  <a href="./README.md">简体中文</a>  | 
  <a href="./README_en.md">English</a>
</p>

# SKP2GLTF

一个用于将 SketchUp (.skp) 文件转换为 glTF/GLB 格式的工具。支持 Draco 网格压缩，可以显著减小输出文件的大小。

## 功能特性

- 支持 SketchUp (.skp) 文件转换为 glTF/GLB 格式
- 集成 Draco 压缩算法，可以有效减小输出文件体积
- 支持材质、纹理和几何数据的转换
- 支持自定义压缩参数配置
- 支持批量处理文件

## 截图展示

### 命令行执行界面
![命令行执行界面](./static/cli.png)

### 转换结果预览
![转换结果预览](./static/preview.png)

## 系统要求

- 操作系统：仅支持 Windows 和 Windows Server 平台
  - Windows 10/11 64位
  - Windows Server 2016/2019/2022
- 其他要求：
  - Visual Studio 2019 或更高版本（用于编译）
  - SketchUp 2019 或更高版本（用于运行时环境）

## 依赖项

- SketchUp SDK (2019+)
- Draco 压缩库
- TinyGLTF
- CMake (构建系统)

## 构建说明

1. 确保已安装 CMake 和支持的 C++ 编译器
2. 克隆仓库：
   ```bash
   git clone <repository-url>
   cd skp2gltf
   ```
3. 创建构建目录：
   ```bash
   mkdir build && cd build
   ```
4. 配置和构建项目：
   ```bash
   cmake ..
   cmake --build .
   ```

## 使用方法

### 基本用法

编译完成后，可执行文件 `skp2gltf.exe` 位于 `build/Debug` 或 `build/Release` 目录下（取决于编译配置）。

命令行格式：
```bash
skp2gltf.exe <input.skp> <output_dir> <output_name>
```

参数说明：
- `<input.skp>`: 输入的 SketchUp 文件路径
- `<output_dir>`: 输出文件夹路径
- `<output_name>`: 输出文件名（不需要包含扩展名）

使用示例：
```bash
# 将 model.skp 转换为 GLTF 文件，输出到 output 文件夹，文件名为 result
skp2gltf.exe "C:\models\model.skp" "C:\models\output" "result"
```

注意：
- 如果路径中包含空格，需要用引号括起来
- 输出文件夹必须已存在
- 程序会根据设置自动选择输出 .gltf 或 .glb 格式

## 贡献

欢迎提交 Issue 和 Pull Request！
如需进一步沟通，请通过以下邮箱联系我们：  
**Email:** dlutyaol@qq.com

![QR Code](./static/contact.jpg)

## 许可证

本项目采用 GNU General Public License v3.0 (GPLv3) 许可证。

### 主要条款

- 自由使用：您可以自由地使用、修改和分发本软件。
- 开源要求：任何基于本软件的衍生作品必须以相同的 GPLv3 许可证开源。
- 专利授权：贡献者明确授予专利权利。
- 声明要求：需要在显著位置说明对源代码的修改。
- 复制保护：禁止添加额外限制，不得限制他人的 GPLv3 权利。

完整许可证文本请参阅：[GNU GPLv3](https://www.gnu.org/licenses/gpl-3.0.html)

注意：本项目仅推荐个人使用。

## 致谢

- [SketchUp SDK](https://extensions.sketchup.com/developers)
- [Draco](https://github.com/google/draco)
- [TinyGLTF](https://github.com/syoyo/tinygltf)