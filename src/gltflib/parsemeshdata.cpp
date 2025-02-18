/*
 * @Author: yaol 
 * @Date: 2025-02-18 17:30:07 
 * @Last Modified by:   yaol 
 * @Last Modified time: 2025-02-18 17:30:07 
 */

#include "parsemeshdata.h"
namespace gltf
{
ParseMeshdata::ParseMeshdata(tinygltf::Model *_model)
{
    model = _model;
    getAllBufferViewers();
    getAllMeshes();
}
void ParseMeshdata::getAllBufferViewers()
{
    allBufferViewers.clear();
    for (auto iter = model->bufferViews.begin(); iter != model->bufferViews.end(); ++iter)
    {
        int start = iter->byteOffset;
        int count = iter->byteLength;
        std::vector<uchar> bufferViewer(model->buffers[iter->buffer].data.begin() + start, model->buffers[iter->buffer].data.begin() + start + count);
        allBufferViewers.emplace_back(bufferViewer);
    }
}
void ParseMeshdata::getAllMeshes()
{
    meshes.clear();
    for (auto iter = model->meshes.begin(); iter != model->meshes.end(); ++iter)
    {
        struct MeshData meshData;
        for (auto iter_primitives = iter->primitives.begin(); iter_primitives != iter->primitives.end(); ++iter_primitives)
        {
            Primitive primitive;
            std::vector<double> positionfloat;
            std::vector<double> colorfloat;
            std::vector<double> normalfloat;
            std::vector<double> uvfloat;
            std::vector<size_t> indecShort;
            primitive.mode                        = iter_primitives->mode;
            primitive.material                    = model->materials[iter_primitives->material];
            int positionSize                      = 0;
            std::map<std::string, int> attributes = iter_primitives->attributes;
            if (attributes.find("POSITION") != attributes.end())
            {
                int position          = attributes["POSITION"];
                int bufferViewerIndex = model->accessors[position].bufferView;
                size_t componentType  = model->accessors[position].componentType;
                int byteLength        = tinygltf::GetComponentSizeInBytes(componentType);
                size_t type           = model->accessors[position].type;
                size_t typeSize       = tinygltf::GetNumComponentsInType(type);
                int byteStride =
                    model->bufferViews[bufferViewerIndex].byteStride == 0 ? typeSize * byteLength : model->bufferViews[bufferViewerIndex].byteStride;
                int start                         = model->accessors[position].byteOffset;
                int end                           = start + model->accessors[position].count * byteStride;
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
            if (attributes.find("COLOR_0") != attributes.end())
            {
                int color          = attributes["COLOR_0"];
                int bufferViewerIndex = model->accessors[color].bufferView;
                size_t componentType  = model->accessors[color].componentType;
                int byteLength        = tinygltf::GetComponentSizeInBytes(componentType);
                size_t type           = model->accessors[color].type;
                size_t typeSize       = tinygltf::GetNumComponentsInType(type);
                int byteStride =
                    model->bufferViews[bufferViewerIndex].byteStride == 0 ? typeSize * byteLength : model->bufferViews[bufferViewerIndex].byteStride;
                int start                         = model->accessors[color].byteOffset;
                int end                           = start + model->accessors[color].count * byteStride;
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
                            colorfloat.emplace_back(static_cast<double>(f));
                        }
                        else if (byteLength == 8)
                        {
                            double f;
                            memcpy(&f, &bufferViewer[0] + i + j * byteLength, byteLength);
                            colorfloat.emplace_back(f);
                        }
                    }
                }
            }
            if (attributes.find("NORMAL") != attributes.end())
            {
                int normal            = attributes["NORMAL"];
                int bufferViewerIndex = model->accessors[normal].bufferView;
                size_t componentType  = model->accessors[normal].componentType;
                int byteLength        = tinygltf::GetComponentSizeInBytes(componentType);
                size_t type           = model->accessors[normal].type;
                size_t typeSize       = tinygltf::GetNumComponentsInType(type);
                int byteStride =
                    model->bufferViews[bufferViewerIndex].byteStride == 0 ? typeSize * byteLength : model->bufferViews[bufferViewerIndex].byteStride;
                int start                         = model->accessors[normal].byteOffset;
                int end                           = start + model->accessors[normal].count * byteStride;
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
                int bufferViewerIndex = model->accessors[normal].bufferView;
                uint componentType    = model->accessors[normal].componentType;
                int byteLength        = tinygltf::GetComponentSizeInBytes(componentType);
                uint type             = model->accessors[normal].type;
                uint typeSize         = tinygltf::GetNumComponentsInType(type);
                int byteStride =
                    model->bufferViews[bufferViewerIndex].byteStride == 0 ? typeSize * byteLength : model->bufferViews[bufferViewerIndex].byteStride;
                int start                         = model->accessors[normal].byteOffset;
                int end                           = start + model->accessors[normal].count * byteStride;
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
            int indecIndex                    = iter_primitives->indices;
            int bufferViewerIndex             = model->accessors[indecIndex].bufferView;
            int start                         = model->accessors[indecIndex].byteOffset;
            size_t componentType              = model->accessors[indecIndex].componentType;
            int byteLength                    = tinygltf::GetComponentSizeInBytes(componentType);
            int end                           = start + model->accessors[indecIndex].count * byteLength;
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
                int textureIndex = model->materials[iter_primitives->material].pbrMetallicRoughness.baseColorTexture.index;
                if (textureIndex != -1)
                {
                    int sourceIndex = model->textures[textureIndex].source;
                    if (sourceIndex != -1)
                    {
                        primitive.imageUri = model->images[sourceIndex].uri;
                    }
                }
            }
            primitive.position      = positionfloat;
            primitive.color         = colorfloat;
            primitive.uv            = uvfloat;
            primitive.normal        = normalfloat;
            primitive.indec         = indecShort;
            primitive.materialIndex = iter_primitives->material;
            if (iter_primitives->material != -1)
            {
                primitive.materialName = model->materials[iter_primitives->material].name;
            }
            meshData.primitives.emplace_back(primitive);
        }
        meshes.emplace_back(meshData);
    }
}
}  // namespace gltf