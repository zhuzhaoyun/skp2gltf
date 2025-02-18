/*
 * @Author: yaol
 * @Date: 2025-02-18 17:31:30
 * @Last Modified by: yaol
 * @Last Modified time: 2025-02-18 17:33:16
 */

#ifndef GLTFINSTANCE_GLTFDRACO_H
#define GLTFINSTANCE_GLTFDRACO_H
#include <vector>
#include <string>
#include <unordered_map>
// #include "parsegltf.h"
#include <draco/compression/encode.h>
#include <draco/io/mesh_io.h>
#include <draco/io/point_cloud_io.h>
#include "parsemeshdata.h"
namespace gltf
{
  class GltfDraco
  {
  public:
    GltfDraco(tinygltf::Model *_model);
    GltfDraco(std::string);
    void encode(int _speed, int _positionBit, int _texBit, int _normalBit, int _colorBit, int _genericBit);
    void saveGltf(std::string outputFile, std::string fileType);
    void deleteModel();

  private:
    int initModel(bool useGZFile = false);
    void dealAccessors();
    void dealMesh();
    std::string getFilePathExtension(const std::string &);

  private:
    tinygltf::Model *model;
    tinygltf::TinyGLTF gltfCtx;
    std::string inputFile;
    int speed;
    int positionBit;
    int texBit;
    int normalBit;
    int colorBit;
    int genericBit;
  };
} // namespace gltf
#endif // GLTFINSTANCE_GLTFDRACO_H
