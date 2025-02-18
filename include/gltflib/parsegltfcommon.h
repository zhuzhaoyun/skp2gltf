/*
 * @Author: yaol
 * @Date: 2025-02-18 17:31:38
 * @Last Modified by: yaol
 * @Last Modified time: 2025-02-18 17:32:52
 */

#ifndef PARSEGLTFCOMMON_H
#define PARSEGLTFCOMMON_H
#include "gltfstruct.hpp"
using uchar = unsigned char;
using uint = unsigned int;
namespace gltf
{
  class ParseGltfCommon
  {
  public:
    ParseGltfCommon(std::string, int, std::string, std::string, std::string, bool split = false);
    ParseGltfCommon(std::string);
    ParseGltfCommon(std::string, bool);
    ParseGltfCommon() = delete;
    int initModel(bool useGZFile = false);
    void getGlobalBox();
    std::string getFilePathExtension(const std::string &);
    void getAllMeshes();
    void getAllNodes();
    void getAllBufferViewers();
    void getInstance();
    void getMappingRelation();
    void getAllMeshByNode();
    void findMinAndMaxVec(std::vector<double> &min, std::vector<double> &max, std::vector<double> &vec);
    std::string generateUuid();
    std::string getFileName(const std::string &);
    void parseBasePoint();
    std::shared_ptr<std::vector<GeoData>> getGeoFromGltf();

  protected:
    tinygltf::Model model;
    tinygltf::Model newModel;
    tinygltf::TinyGLTF gltfCtx;

  public:
    std::vector<MeshData> meshes;
    std::vector<Node> nodeVec;

  protected:
    std::vector<std::vector<uchar>> allBufferViewers;
    std::map<int, std::vector<int>> mappingRelationVec;
    std::set<int> instanceMeshVec;
    std::set<int> mergeMeshVec;

    std::string fileName;
    std::vector<double> basePointVec;
    std::vector<std::string> newNodeUuidVec;
    std::vector<double> identityMatrix;
    std::vector<double> minPoint;
    std::vector<double> maxPoint;
    size_t faceNum;
    size_t vertexNum;

    int instanceThreshold;
    std::string filePath;
    std::string fileType;
    std::string locationParameter;

    bool bigValue;
    std::vector<std::string> attributesVec;
  };
} // namespace gltf

#endif // PARSEGLTFCOMMON_H