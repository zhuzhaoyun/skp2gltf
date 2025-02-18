/*
 * @Author: yaol 
 * @Date: 2025-02-18 17:30:14 
 * @Last Modified by:   yaol 
 * @Last Modified time: 2025-02-18 17:30:14 
 */

#include "parsegltfcommon.h"
#include "datatransform.hpp"
namespace gltf
{
/**
 * @description: 只解析gltf数据
 * @param :
 * @return {*}
 */
ParseGltfCommon::ParseGltfCommon(std::string _fileName) : fileName(_fileName)
{
    identityMatrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    initModel();
    getAllBufferViewers();
    getAllMeshes();
    getAllNodes();
    getMappingRelation();
    getGlobalBox();
}
ParseGltfCommon::ParseGltfCommon(std::string _fileName, bool parData) : fileName(_fileName)
{
    if (parData)
    {
        identityMatrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        initModel();
        getAllBufferViewers();
        getAllMeshes();
        getAllNodes();
        getMappingRelation();
        getGlobalBox();
    }
    else
    {
        identityMatrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        initModel();
    }
}
void ParseGltfCommon::parseBasePoint()
{
    basePointVec = {(maxPoint[0] + minPoint[0]) / 2, (maxPoint[1] + minPoint[1]) / 2, (maxPoint[2] + minPoint[2]) / 2};
}
std::string ParseGltfCommon::getFilePathExtension(const std::string &fileName)
{
    if (fileName.find_last_of(".") != std::string::npos)
        return fileName.substr(fileName.find_last_of(".") + 1);
    return "";
}
/**
 * @description: 初始化model
 * @param :
 * @return {*}
 */
int ParseGltfCommon::initModel(bool useGZFile)
{
    std::string err;
    std::string warn;
    std::string ext                                    = getFilePathExtension(fileName);
    bool store_original_json_for_extras_and_extensions = false;
    gltfCtx.SetStoreOriginalJSONForExtrasAndExtensions(store_original_json_for_extras_and_extensions);

    bool ret = false;

    if (ext.compare("glb") == 0)
    {
        std::cout << "Reading binary glb" << std::endl;
        ret = gltfCtx.LoadBinaryFromFile(&model, &err, &warn, fileName.c_str());
    }
    else
    {
        std::cout << "Reading ASCII glTF" << std::endl;
        ret = gltfCtx.LoadASCIIFromFile(&model, &err, &warn, fileName.c_str());
    }
    
    if (!warn.empty())
    {
        std::cout << "[WARN] Warn is " << warn << std::endl;
    }

    if (!err.empty())
    {
        std::cout << "Err is " << err << std::endl;
    }

    if (!ret)
    {
        std::cout << "Failed to parse glTF ,Application has stopped !!!" << std::endl;
        return -1;
    }
}
/**
 * @description:获取BufferViewer数据
 * @param :
 * @return {type}
 */
void ParseGltfCommon::getAllBufferViewers()
{
    allBufferViewers.clear();
    for (auto iter = model.bufferViews.begin(); iter != model.bufferViews.end(); ++iter)
    {
        int start = iter->byteOffset;
        int count = iter->byteLength;
        std::vector<uchar> bufferViewer(model.buffers[iter->buffer].data.begin() + start, model.buffers[iter->buffer].data.begin() + start + count);
        allBufferViewers.emplace_back(bufferViewer);
    }
}
void ParseGltfCommon::getInstance()
{
    instanceMeshVec.clear();
    mergeMeshVec.clear();
    for (auto iter = mappingRelationVec.begin(); iter != mappingRelationVec.end(); ++iter)
    {
        if (iter->second.size() >= instanceThreshold)  //如果大于等于阈值，加入instance集合，否则加入merge集合
        {
            instanceMeshVec.insert(iter->first);
        }
        else
        {
            mergeMeshVec.insert(iter->first);
        }
    }
}
void ParseGltfCommon::getAllNodes()
{
    for (auto iter = model.nodes.begin(); iter != model.nodes.end(); ++iter)
    {
        if (iter->mesh != -1)
        {
            Node node(iter->name, iter->matrix);
            node.meshIndex = iter->mesh;
            nodeVec.emplace_back(node);
        }
    }
}
/**
 * @description: 获取mesh数据
 * @param :
 * @return {*}
 */
void ParseGltfCommon::getAllMeshes()
{
    meshes.clear();
    for (auto iter = model.meshes.begin(); iter != model.meshes.end(); ++iter)
    {
        struct MeshData meshData;
        for (auto iter_primitives = iter->primitives.begin(); iter_primitives != iter->primitives.end(); ++iter_primitives)
        {
            Primitive primitive;
            std::vector<double> positionfloat;
            std::vector<double> normalfloat;
            std::vector<double> uvfloat;
            std::vector<size_t> indecShort;
            primitive.mode                        = iter_primitives->mode;
            primitive.material                    = model.materials[iter_primitives->material];
            int positionSize                      = 0;
            std::map<std::string, int> attributes = iter_primitives->attributes;
            if (attributes.find("POSITION") != attributes.end())
            {
                int position          = attributes["POSITION"];
                int bufferViewerIndex = model.accessors[position].bufferView;
                int bufferIndex       = model.bufferViews[bufferViewerIndex].buffer;
                size_t componentType  = model.accessors[position].componentType;
                int byteLength        = tinygltf::GetComponentSizeInBytes(componentType);
                size_t type           = model.accessors[position].type;
                size_t typeSize       = tinygltf::GetNumComponentsInType(type);
                int byteStride =
                    model.bufferViews[bufferViewerIndex].byteStride == 0 ? typeSize * byteLength : model.bufferViews[bufferViewerIndex].byteStride;
                int start                         = model.accessors[position].byteOffset;
                int end                           = start + model.accessors[position].count * byteStride;
                std::vector<uchar> &bufferViewers = allBufferViewers[bufferViewerIndex];
                std::vector<uchar> bufferViewer(bufferViewers.begin() + start, bufferViewers.begin() + end);
                for (int i = 0; i < bufferViewer.size(); i += byteStride)
                {
                    for (int j = 0; j < typeSize; ++j)
                    {
                        positionSize++;
                        if (byteLength == 4)
                        {
                            float f;
                            memcpy(&f, &bufferViewer[0] + i + j * byteLength, byteLength);
                            positionfloat.emplace_back(static_cast<double>(f));
                        }
                        else if (byteLength == 8)
                        {
                            double f;
                            memcpy(&f, &bufferViewer[0] + i + j * byteLength, byteLength);
                            positionfloat.emplace_back(f);
                        }
                    }
                }
            }
            if (attributes.find("NORMAL") != attributes.end())
            {
                int normal            = attributes["NORMAL"];
                int bufferViewerIndex = model.accessors[normal].bufferView;
                int bufferIndex       = model.bufferViews[bufferViewerIndex].buffer;
                size_t componentType  = model.accessors[normal].componentType;
                int byteLength        = tinygltf::GetComponentSizeInBytes(componentType);
                size_t type           = model.accessors[normal].type;
                size_t typeSize       = tinygltf::GetNumComponentsInType(type);
                int byteStride =
                    model.bufferViews[bufferViewerIndex].byteStride == 0 ? typeSize * byteLength : model.bufferViews[bufferViewerIndex].byteStride;
                int start                         = model.accessors[normal].byteOffset;
                int end                           = start + model.accessors[normal].count * byteStride;
                std::vector<uchar> &bufferViewers = allBufferViewers[bufferViewerIndex];
                std::vector<uchar> bufferViewer(bufferViewers.begin() + start, bufferViewers.begin() + end);
                for (int i = 0; i < bufferViewer.size();)
                {
                    for (int j = 0; j < typeSize; ++j)
                    {
                        if (byteLength == 4)
                        {
                            float f;
                            memcpy(&f, &bufferViewer[0] + i + j * byteLength, byteLength);
                            normalfloat.emplace_back(static_cast<double>(f));
                        }
                        else if (byteLength == 8)
                        {
                            double f;
                            memcpy(&f, &bufferViewer[0] + i + j * byteLength, byteLength);
                            normalfloat.emplace_back(f);
                        }
                    }
                    i += byteStride;
                }
            }
            if (attributes.find("TEXCOORD_0") != attributes.end())
            {
                int normal            = attributes["TEXCOORD_0"];
                int bufferViewerIndex = model.accessors[normal].bufferView;
                int bufferIndex       = model.bufferViews[bufferViewerIndex].buffer;
                uint componentType    = model.accessors[normal].componentType;
                int byteLength        = tinygltf::GetComponentSizeInBytes(componentType);
                uint type             = model.accessors[normal].type;
                uint typeSize         = tinygltf::GetNumComponentsInType(type);
                int byteStride =
                    model.bufferViews[bufferViewerIndex].byteStride == 0 ? typeSize * byteLength : model.bufferViews[bufferViewerIndex].byteStride;
                int start                         = model.accessors[normal].byteOffset;
                int end                           = start + model.accessors[normal].count * byteStride;
                std::vector<uchar> &bufferViewers = allBufferViewers[bufferViewerIndex];
                std::vector<uchar> bufferViewer(bufferViewers.begin() + start, bufferViewers.begin() + end);
                for (int i = 0; i < bufferViewer.size();)
                {
                    for (int j = 0; j < typeSize; ++j)
                    {
                        if (byteLength == 4)
                        {
                            float f;
                            memcpy(&f, &bufferViewer[0] + i + j * byteLength, byteLength);
                            uvfloat.emplace_back(static_cast<double>(f));
                        }
                        else if (byteLength == 8)
                        {
                            double f;
                            memcpy(&f, &bufferViewer[0] + i + j * byteLength, byteLength);
                            uvfloat.emplace_back(f);
                        }
                    }
                    i += byteStride;
                }
            }
            int indecIndex        = iter_primitives->indices;
            int bufferViewerIndex = model.accessors[indecIndex].bufferView;
            int bufferIndex       = model.bufferViews[bufferViewerIndex].buffer;
            int byteStride        = model.bufferViews[bufferViewerIndex].byteStride == 0 ? 1 : model.bufferViews[bufferViewerIndex].byteStride;
            int start             = model.accessors[indecIndex].byteOffset;
            size_t componentType  = model.accessors[indecIndex].componentType;
            int byteLength        = tinygltf::GetComponentSizeInBytes(componentType);
            int end               = start + model.accessors[indecIndex].count * byteLength;
            std::vector<uchar> &bufferViewers = allBufferViewers[bufferViewerIndex];
            std::vector<uchar> bufferViewer(bufferViewers.begin() + start, bufferViewers.begin() + end);
            if (byteLength == 2)
            {
                for (int i = 0; i < bufferViewer.size(); i += 2)
                {
                    unsigned int f;
                    uchar temp[4];
                    temp[0] = bufferViewer[i];
                    temp[1] = bufferViewer[i + 1];
                    temp[2] = 0;
                    temp[3] = 0;
                    memcpy(&f, temp, 4);
                    indecShort.emplace_back(f);
                }
            }
            else if (byteLength == 4)
            {
                for (int i = 0; i < bufferViewer.size(); i += 4)
                {
                    unsigned int f;
                    uchar temp[4];
                    temp[0] = bufferViewer[i];
                    temp[1] = bufferViewer[i + 1];
                    temp[2] = bufferViewer[i + 2];
                    temp[3] = bufferViewer[i + 3];
                    memcpy(&f, temp, 4);
                    indecShort.emplace_back(f);
                }
            }
            if (iter_primitives->material != -1)
            {
                int textureIndex = model.materials[iter_primitives->material].pbrMetallicRoughness.baseColorTexture.index;
                if (textureIndex != -1)
                {
                    int sourceIndex = model.textures[textureIndex].source;
                    if (sourceIndex != -1)
                    {
                        primitive.imageUri = model.images[sourceIndex].uri;
                    }
                }
            }
            primitive.position      = positionfloat;
            primitive.uv            = uvfloat;
            primitive.normal        = normalfloat;
            primitive.indec         = indecShort;
            primitive.materialIndex = iter_primitives->material;
            if (iter_primitives->material != -1)
            {
                primitive.materialName = model.materials[iter_primitives->material].name;
            }
            meshData.primitives.emplace_back(primitive);
        }
        meshes.emplace_back(meshData);
    }
}
/**
 * @description: 获取所有映射关系
 * @param :
 * @return {type}
 */
void ParseGltfCommon::getMappingRelation()
{
    mappingRelationVec.clear();
    int nodeIndex = 0;
    for (auto iter = model.nodes.begin(); iter != model.nodes.end(); ++iter, nodeIndex++)
    {
        if (iter->mesh != -1)
        {
            mappingRelationVec[iter->mesh].emplace_back(nodeIndex);
        }
    }
}
void ParseGltfCommon::findMinAndMaxVec(std::vector<double> &min, std::vector<double> &max, std::vector<double> &vec)
{
    double minX = vec[0];
    double minY = vec[1];
    double minZ = vec[2];
    double maxX = vec[0];
    double maxY = vec[1];
    double maxZ = vec[2];
    for (int i = 0; i < vec.size(); i += 3)
    {
        if (vec[i] < minX)
            minX = vec[i];
        if (vec[i + 1] < minY)
            minY = vec[i + 1];
        if (vec[i + 2] < minZ)
            minZ = vec[i + 2];
        if (vec[i] > maxX)
            maxX = vec[i];
        if (vec[i + 1] > maxY)
            maxY = vec[i + 1];
        if (vec[i + 2] > maxZ)
            maxZ = vec[i + 2];
    }
    min = {minX, minY, minZ};
    max = {maxX, maxY, maxZ};
}
/**
 * @description:求模型的包围盒
 * @param :
 * @return {*}
 */
void ParseGltfCommon::getGlobalBox()
{
    if (minPoint.empty())
    {
        minPoint = {DBL_MAX, DBL_MAX, DBL_MAX};
    }
    if (maxPoint.empty())
    {
        maxPoint = {-DBL_MAX, -DBL_MAX, -DBL_MAX};
    }
    for (auto iter = model.nodes.begin(); iter != model.nodes.end(); ++iter)
    {
        if (iter->mesh != -1)
        {
            if (iter->matrix.empty())
            {
                iter->matrix = identityMatrix;
            }
            std::vector<double> &matrix = iter->matrix;
            for (int j = 0; j < meshes[iter->mesh].primitives.size(); ++j)
            {
                std::vector<double> &oldPosition = meshes[iter->mesh].primitives[j].position;
                vertexNum += oldPosition.size() / 3;
                faceNum += meshes[iter->mesh].primitives[j].indec.size() / 3;
                for (int i = 0; i < oldPosition.size(); i += 3)
                {
                    double oldX = oldPosition[i];
                    double oldY = oldPosition[i + 1];
                    double oldZ = oldPosition[i + 2];
                    double newX = oldX * matrix[0] + oldY * matrix[4] + oldZ * matrix[8] + matrix[12];
                    double newY = oldX * matrix[1] + oldY * matrix[5] + oldZ * matrix[9] + matrix[13];
                    double newZ = oldX * matrix[2] + oldY * matrix[6] + oldZ * matrix[10] + matrix[14];
                    if (newX < minPoint[0])
                        minPoint[0] = newX;
                    if (newY < minPoint[1])
                        minPoint[1] = newY;
                    if (newZ < minPoint[2])
                        minPoint[2] = newZ;
                    if (newX > maxPoint[0])
                        maxPoint[0] = newX;
                    if (newY > maxPoint[1])
                        maxPoint[1] = newY;
                    if (newZ > maxPoint[2])
                        maxPoint[2] = newZ;
                }
            }
        }
    }
}
/**
 * @description: 从文件路中获取路径名
 * @param :
 * @return {*}
 */
std::string ParseGltfCommon::getFileName(const std::string &fileName)
{
    int start = 0;
    int end   = 0;
    if (fileName.find_last_of(".") != std::string::npos)
    {
        end = fileName.find_last_of(".");
    }
    if (fileName.find_last_of("/") != std::string::npos)
    {
        start = fileName.find_last_of("/");
        start += 1;
    }
    if (end <= start)
        return "";
    return fileName.substr(start, end - start);
}
// std::string ParseGltfCommon::generateUuid()
// {
//     sole::uuid u4 = sole::uuid4();
//     return u4.str();
// }
std::shared_ptr<std::vector<GeoData>> ParseGltfCommon::getGeoFromGltf()
{
    std::shared_ptr<std::vector<GeoData>> geoDatas = std::make_shared<std::vector<GeoData>>();
    for (auto iter = mappingRelationVec.begin(); iter != mappingRelationVec.end(); ++iter)
    {
        GeoData geoData;
        for (auto iter_primitive = meshes[iter->first].primitives.begin(); iter_primitive != meshes[iter->first].primitives.end(); ++iter_primitive)
        {
            std::vector<size_t> indecVec_temp = iter_primitive->indec;
            for (auto iter_indec = indecVec_temp.begin(); iter_indec != indecVec_temp.end(); ++iter_indec)
            {
                *iter_indec += geoData.position.size() / 3;
            }
            geoData.position.insert(geoData.position.end(), iter_primitive->position.begin(), iter_primitive->position.end());
            geoData.normal.insert(geoData.normal.end(), iter_primitive->normal.begin(), iter_primitive->normal.end());
            geoData.indec.insert(geoData.indec.end(), indecVec_temp.begin(), indecVec_temp.end());
        }
        std::vector<Node> mappingVec;
        for (auto iter_s = iter->second.begin(); iter_s != iter->second.end(); ++iter_s)
        {
            Node mapping;
            mapping.guid   = model.nodes[*iter_s].name;
            mapping.matrix = model.nodes[*iter_s].matrix;
            mappingVec.emplace_back(mapping);
        }
        geoData.mapping = mappingVec;
        geoDatas->emplace_back(geoData);
    }
    return geoDatas;
}
}  // namespace gltf