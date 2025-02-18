/*
 * @Author: yaol
 * @Date: 2025-02-18 17:31:48
 * @Last Modified by: yaol
 * @Last Modified time: 2025-02-18 17:32:30
 */

#ifndef GLTFINSTANCE_PARSEMESHDATA_H
#define GLTFINSTANCE_PARSEMESHDATA_H
#include "gltfstruct.hpp"
using uchar = unsigned char;
using uint = unsigned int;
namespace gltf
{
  class ParseMeshdata
  {
  public:
    ParseMeshdata(tinygltf::Model *);

  private:
    void getAllBufferViewers();
    void getAllMeshes();

  private:
    tinygltf::Model *model;
    std::vector<std::vector<uchar>> allBufferViewers; 
  public:
    std::vector<MeshData> meshes;
    std::vector<Node> nodeVec;
  };
}
#endif // GLTFINSTANCE_PARSEMESHDATA_H
