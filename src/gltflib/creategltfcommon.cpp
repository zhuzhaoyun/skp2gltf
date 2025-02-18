/*
 * @Author: yaol 
 * @Date: 2025-02-18 17:28:38 
 * @Last Modified by:   yaol 
 * @Last Modified time: 2025-02-18 17:28:38 
 */

#include "creategltfcommon.h"
#include "datatransform.hpp"
namespace gltf
{
void CreateGltfCommon::setMeshData(std::vector<Node> &_nodeVec, std::vector<MeshData> &_meshDataVec)
{
    nodeVec     = _nodeVec;
    meshDataVec = _meshDataVec;
}
CreateGltfCommon::CreateGltfCommon()
{
    maxByteLength = 4;
    if (basePointVec.empty())
    {
        basePointVec = {0.0, 0.0, 0.0};
    }
    if (axisVec.empty())
    {
        axisVec = {0.0, 0.0, 1.0};
    }
    if (refDirectionVec.empty())
    {
        refDirectionVec = {1.0, 0.0, 0.0};
    }
}
void CreateGltfCommon::createGltf(const std::string &_fileType, const std::string &_gltfName)
{
    gltfName = _gltfName;
    fileType = _fileType;
    createAssets();
    createScenes();
    createNodes();
    createMeshes();
    createAccessors();
    createImages();
    createTextures();
    saveGltf(gltfName);
}
void CreateGltfCommon::createAssets()
{
    tinygltf::Asset asset;
    asset.version   = "2.0";
    asset.generator = "BDR";
    model.asset     = asset;
}
void CreateGltfCommon::createScenes()
{
    tinygltf::Scene scene;
    scene.nodes.emplace_back(0);
    model.scenes.emplace_back(scene);
}
/**
 * @description: 创建Nodes(如果是instance则只需要处理mesh,node不需要处理)
 * @param :
 * @return:
 */
void CreateGltfCommon::createNodes()
{
    tinygltf::Node node;
    for (int i = 0; i < nodeVec.size(); ++i)
    {
        node.children.emplace_back(i + 1);
        node.matrix = {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0};
    }
    model.nodes.emplace_back(node);
    for (auto iter = nodeVec.begin(); iter != nodeVec.end(); ++iter)
    {
        tinygltf::ExtensionMap extensionsMap;
        tinygltf::ExtensionMap bcoreMap;
        {
            tinygltf::Node node;
            node.mesh   = iter->meshIndex;
            node.name   = iter->guid;
            node.matrix = iter->matrix;
            model.nodes.emplace_back(node);
        }
    }
}
/**
 * @description: 创建Meshes
 * @param :
 * @return:
 */
void CreateGltfCommon::createMeshes()
{
    int i              = 0;
    int materialCount  = -1;
    int attributeIndex = -1;
    for (auto iter = meshDataVec.begin(); iter != meshDataVec.end(); ++iter)
    {
        tinygltf::Mesh newMesh;
        for (auto iter_pris = iter->primitives.begin(); iter_pris != iter->primitives.end(); ++iter_pris)
        {
            int attributesCount = 2;
            if (!iter_pris->uv.empty())
            {
                attributesCount++;
            }
            if (!iter_pris->normal.empty())
            {
                attributesCount++;
            }
            if (!iter_pris->color.empty())
            {
                attributesCount++;
            }
            tinygltf::Primitive newPrimitive;
            std::map<std::string, int> attributes;
            newPrimitive.indices = ++attributeIndex;
            newPrimitive.mode    = iter_pris->mode;
            if (iter_pris->materialIndex != -1)
            {
                newPrimitive.material    = ++materialCount;
                iter_pris->materialIndex = materialCount;
                model.materials.emplace_back(iter_pris->material);
            }
            if (!iter_pris->position.empty())
            {
                attributes.insert(std::make_pair("POSITION", ++attributeIndex));
            }
            if (!iter_pris->normal.empty())
            {
                attributes.insert(std::make_pair("NORMAL", ++attributeIndex));
            }
            if (!iter_pris->color.empty())
            {
                attributes.insert(std::make_pair("COLOR_0", ++attributeIndex));
            }
            if (!iter_pris->uv.empty())
            {
                attributes.insert(std::make_pair("TEXCOORD_0", ++attributeIndex));
            }
            newPrimitive.attributes = attributes;
            newMesh.primitives.emplace_back(newPrimitive);
        }
        model.meshes.emplace_back(newMesh);
    }
}
void CreateGltfCommon::createTextures()
{
    int n = 0;
    int i = 0;
    for (auto iter = meshDataVec.begin(); iter != meshDataVec.end(); ++iter)
    {
        for (auto iter_pri = iter->primitives.begin(); iter_pri != iter->primitives.end(); ++iter_pri, ++i)
        {
            if (iter_pri->imageUri != "")
            {
                model.materials[i].pbrMetallicRoughness.baseColorTexture.index = imageIndexMap[iter_pri->imageUri];
            }
            else
            {
                if (iter_pri->materialIndex != -1)
                {
                    model.materials[iter_pri->materialIndex].pbrMetallicRoughness.baseColorTexture.index = -1;
                }
            }
        }
    }
}
void CreateGltfCommon::createImages()
{
    int imageIndex = 0;
    for (auto iter = meshDataVec.begin(); iter != meshDataVec.end(); ++iter)
    {
        for (auto iter_pri = iter->primitives.begin(); iter_pri != iter->primitives.end(); ++iter_pri)
        {
            if (iter_pri->imageUri != "")
            {
                if (imageIndexMap.find(iter_pri->imageUri) == imageIndexMap.end())
                {
                    imageIndexMap[iter_pri->imageUri] = imageIndex;
                    std::string uri                   = iter_pri->imageUri;
                    tinygltf::Image image;
                    image.uri = uri;
                    model.images.emplace_back(image);

                    tinygltf::Texture texture;
                    texture.source = imageIndex;
                    model.textures.emplace_back(texture);
                    imageIndex++;
                }
            }
        }
    }
}
/**
 * @description: 创建Accessors
 * @param :
 * @return:
 */
void CreateGltfCommon::createAccessors()
{
    int indecOffeset = 0;
    int arrOffset    = 0;
    int uvOffset     = 0;
    std::vector<uchar> buffer;
    std::vector<uchar> indecBufferVec;
    std::vector<uchar> arrayBufferVec;
    std::vector<uchar> uvVec;
    for (auto iter = meshDataVec.begin(); iter != meshDataVec.end(); ++iter)
    {
        for (auto iter_pri = iter->primitives.begin(); iter_pri != iter->primitives.end(); ++iter_pri)
        {
            // indec
            if (!iter_pri->indec.empty())
            {
                tinygltf::Accessor indecAcc;
                indecAcc.bufferView    = 0;
                indecAcc.byteOffset    = indecOffeset;
                indecAcc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
                indecAcc.count         = iter_pri->indec.size();
                indecAcc.maxValues     = std::vector<double>{(double)*max_element(iter_pri->indec.begin(), iter_pri->indec.end())};
                indecAcc.minValues     = std::vector<double>{(double)*min_element(iter_pri->indec.begin(), iter_pri->indec.end())};
                indecAcc.type          = TINYGLTF_TYPE_SCALAR;
                model.accessors.emplace_back(indecAcc);
                indecOffeset += iter_pri->indec.size() * 4;
                std::vector<uchar> indecBuffer;
                convertTogltf::AccessorsDataS2Unint dataIndec;
                dataIndec.accessorsData = iter_pri->indec;
                dataIndec.getBufferData(indecBuffer);
                indecBufferVec.insert(indecBufferVec.end(), indecBuffer.begin(), indecBuffer.end());
            }
            // position
            if (!iter_pri->position.empty())
            {
                tinygltf::Accessor positionAcc;
                positionAcc.bufferView    = 1;
                positionAcc.byteOffset    = arrOffset;
                positionAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                positionAcc.count         = iter_pri->position.size() / 3;
                positionAcc.type          = TINYGLTF_TYPE_VEC3;
                findMinAndMaxVec(positionAcc.minValues, positionAcc.maxValues, iter_pri->position);
                model.accessors.emplace_back(positionAcc);
                arrOffset += iter_pri->position.size() * 4;
                std::vector<uchar> positionBuffer;
                convertTogltf::AccessorsDataD2F dataPosition;
                dataPosition.accessorsData = iter_pri->position;
                dataPosition.getBufferData(positionBuffer);
                arrayBufferVec.insert(arrayBufferVec.end(), positionBuffer.begin(), positionBuffer.end());
            }
            // normal
            if (!iter_pri->normal.empty())
            {
                tinygltf::Accessor normalAcc;
                normalAcc.bufferView    = 1;
                normalAcc.byteOffset    = arrOffset;
                normalAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                normalAcc.type          = TINYGLTF_TYPE_VEC3;
                normalAcc.count         = iter_pri->normal.size() / 3;
                model.accessors.emplace_back(normalAcc);
                arrOffset += iter_pri->normal.size() * 4;
                std::vector<uchar> normalBuffer;
                convertTogltf::AccessorsDataD2F dataNormal;
                dataNormal.accessorsData = iter_pri->normal;
                dataNormal.getBufferData(normalBuffer);
                arrayBufferVec.insert(arrayBufferVec.end(), normalBuffer.begin(), normalBuffer.end());
            }
            // color
            if (!iter_pri->color.empty())
            {
                tinygltf::Accessor colorlAcc;
                colorlAcc.bufferView    = 1;
                colorlAcc.byteOffset    = arrOffset;
                colorlAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                colorlAcc.type          = TINYGLTF_TYPE_VEC3;
                colorlAcc.count         = iter_pri->color.size() / 3;
                model.accessors.emplace_back(colorlAcc);
                arrOffset += iter_pri->color.size() * 4;
                std::vector<uchar> colorlBuffer;
                convertTogltf::AccessorsDataD2F dataNormal;
                dataNormal.accessorsData = iter_pri->color;
                dataNormal.getBufferData(colorlBuffer);
                arrayBufferVec.insert(arrayBufferVec.end(), colorlBuffer.begin(), colorlBuffer.end());
            }
            // uv
            if (!iter_pri->uv.empty())
            {
                tinygltf::Accessor accessorUv;
                accessorUv.bufferView    = 2;
                accessorUv.byteOffset    = uvOffset;
                accessorUv.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                accessorUv.count         = iter_pri->uv.size() / 2;
                accessorUv.type          = TINYGLTF_TYPE_VEC2;
                model.accessors.emplace_back(accessorUv);
                uvOffset += iter_pri->uv.size() * 4;
                std::vector<uchar> uvBuffer;
                convertTogltf::AccessorsDataD2F dataUV;
                dataUV.accessorsData = iter_pri->uv;
                dataUV.getBufferData(uvBuffer);
                uvVec.insert(uvVec.end(), uvBuffer.begin(), uvBuffer.end());
            }
        }
    }
    tinygltf::BufferView bufferViewIndec;
    bufferViewIndec.buffer     = 0;
    bufferViewIndec.byteOffset = 0;
    bufferViewIndec.byteLength = indecOffeset;
    bufferViewIndec.target     = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    model.bufferViews.emplace_back(bufferViewIndec);

    tinygltf::BufferView bufferViewPos;
    bufferViewPos.buffer     = 0;
    bufferViewPos.byteOffset = indecOffeset;
    bufferViewPos.byteLength = arrOffset;
    bufferViewPos.target     = TINYGLTF_TARGET_ARRAY_BUFFER;
    bufferViewPos.byteStride = 12;
    model.bufferViews.emplace_back(bufferViewPos);

    if (uvOffset > 0)
    {
        tinygltf::BufferView bufferView2;  //纹理
        bufferView2.byteOffset = arrOffset + indecOffeset;
        bufferView2.byteLength = uvOffset;
        bufferView2.buffer     = 0;
        bufferView2.byteStride = 2 * 4;
        bufferView2.target     = TINYGLTF_TARGET_ARRAY_BUFFER;
        model.bufferViews.emplace_back(bufferView2);
    }

    buffer.insert(buffer.end(), indecBufferVec.begin(), indecBufferVec.end());
    buffer.insert(buffer.end(), arrayBufferVec.begin(), arrayBufferVec.end());
    buffer.insert(buffer.end(), uvVec.begin(), uvVec.end());
    tinygltf::Buffer tinyBuffer;
    tinyBuffer.data = buffer;
    model.buffers.emplace_back(tinyBuffer);
}
void CreateGltfCommon::findMinAndMaxVec(std::vector<double> &min, std::vector<double> &max, std::vector<double> &vec)
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
        min = {minX, minY, minZ};
        max = {maxX, maxY, maxZ};
    }
}
// std::string CreateGltfCommon::generateUuid()
// {
//     sole::uuid u4 = sole::uuid4();
//     return u4.str();
// }
void CreateGltfCommon::saveGltf(const std::string &outFile)
{
    bool writeBinary = false;  //写二进制文件
    if (fileType == "glb")
    {
        writeBinary = true;
    }
    else if (fileType == "gltf")
    {
        writeBinary = false;
    }
    std::string filePathStr = outFile + "." + fileType;
    gltfCtx.WriteGltfSceneToFile(&model, filePathStr, false, /*embedBuffers*/ true, true, writeBinary);
}
}  // namespace gltf
