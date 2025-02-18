/*
 * @Author: yaol 
 * @Date: 2025-02-18 17:31:17 
 * @Last Modified by:   yaol 
 * @Last Modified time: 2025-02-18 17:31:17 
 */

#ifndef CREATEMERGEGLTFCOMMON_H
#define CREATEMERGEGLTFCOMMON_H
#include <vector>
#include <string>
#include "tinygltf/gltfstruct.hpp"
namespace gltf
{
class CreateGltfCommon
{
  public:
    CreateGltfCommon();
    void setMeshData(std::vector<Node> &, std::vector<MeshData> &);
    void createGltf(const std::string &, const std::string &);
    void dealPosition(std::vector<double> &);
    void findMinAndMaxVec(std::vector<double> &, std::vector<double> &, std::vector<double> &);
    std::string generateUuid();

  private:
    void createScenes();
    void createNodes();
    void createAssets();
    void createMeshes();
    void createAccessors();
    void createTextures();
    void createImages();
    void saveGltf(const std::string &);

  public:
    tinygltf::Model model;

  private:
    tinygltf::TinyGLTF gltfCtx;
    int maxByteLength;
    std::vector<double> basePointVec;
    std::vector<double> axisVec;
    std::vector<double> refDirectionVec;
    std::string gltfName;
    std::string fileType;
    std::unordered_map<std::string, int> imageIndexMap;
    std::vector<Node> nodeVec;
    std::vector<MeshData> meshDataVec;
};
}  // namespace gltf
#endif  // CREATEMERGEGLTF_H