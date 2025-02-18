/*
 * @Author: yaol 
 * @Date: 2025-02-18 17:29:56 
 * @Last Modified by:   yaol 
 * @Last Modified time: 2025-02-18 17:29:56 
 */


#include "gltfdraco.h"
namespace gltf
{
using namespace draco;
GltfDraco::GltfDraco(tinygltf::Model *_model)
{
    model = _model;
}
GltfDraco::GltfDraco(std::string _inputFile)
{
    inputFile = _inputFile;
    model     = new tinygltf::Model;
    initModel();
}
std::string GltfDraco::getFilePathExtension(const std::string &fileName)
{
    if (fileName.find_last_of(".") != std::string::npos)
        return fileName.substr(fileName.find_last_of(".") + 1);
    return "";
}
int GltfDraco::initModel(bool useGZFile)
{
    std::string err;
    std::string warn;
    std::string ext                                    = getFilePathExtension(inputFile);
    bool store_original_json_for_extras_and_extensions = false;
    gltfCtx.SetStoreOriginalJSONForExtrasAndExtensions(store_original_json_for_extras_and_extensions);

    bool ret = false;
    if (useGZFile)
    {
        std::cout << "Reading gz-compressed glTF" << std::endl;
        ret = gltfCtx.LoadASCIIFromGZFile(model, &err, &warn, inputFile.c_str());
    }
    else
    {
        if (ext.compare("glb") == 0)
        {
            std::cout << "Reading binary glb" << std::endl;
            ret = gltfCtx.LoadBinaryFromFile(model, &err, &warn, inputFile.c_str());
        }
        else
        {
            std::cout << "Reading ASCII glTF" << std::endl;
            ret = gltfCtx.LoadASCIIFromFile(model, &err, &warn, inputFile.c_str());
        }
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
void GltfDraco::encode(int _speed, int _positionBit, int _texBit, int _normalBit, int _colorBit, int _genericBit)
{
    speed       = _speed;
    positionBit = _positionBit;
    texBit      = _texBit;
    normalBit   = _normalBit;
    colorBit    = _colorBit;
    genericBit  = _genericBit;
    dealMesh();
    dealAccessors();
}
void GltfDraco::saveGltf(std::string outputFile, std::string fileType)
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
    std::string filePathStr = outputFile + "." + fileType;
    gltfCtx.WriteGltfSceneToFile(model, filePathStr, false, /*embedBuffers*/ true, true, writeBinary);
}
void GltfDraco::deleteModel()
{
    if (model != nullptr)
    {
        delete model;
    }
}
void GltfDraco::dealAccessors()
{
    model->extensionsUsed.emplace_back("KHR_draco_mesh_compression");
    for (auto iter = model->accessors.begin(); iter != model->accessors.end(); ++iter)
    {
        tinygltf::Accessor accessor;
        accessor.type          = iter->type;
        accessor.count         = iter->count;
        accessor.componentType = iter->componentType;
        accessor.maxValues     = iter->maxValues;
        accessor.minValues     = iter->minValues;
        accessor.type          = iter->type;
        *iter                  = accessor;
    }
}
void GltfDraco::dealMesh()
{
    size_t bufferOffset = 0;
    int bufferViewIndex = 0;
    std::vector<unsigned char> allBuffer;
    int meshIndex = 0;
    ParseMeshdata parseMeshdata(model);
    model->bufferViews.clear();
    model->buffers.clear();
    for (auto iter = parseMeshdata.meshes.begin(); iter != parseMeshdata.meshes.end(); ++iter)
    {
        tinygltf::Mesh &mesh = model->meshes[meshIndex++];
        int primitiveIndex   = 0;
        for (auto iter_pris = iter->primitives.begin(); iter_pris != iter->primitives.end(); ++iter_pris)
        {
            tinygltf::Primitive &primitive = mesh.primitives[primitiveIndex++];
            std::map<std::string, int> attributes;
            std::pair<std::string, tinygltf::Value> extensionsPair;
            tinygltf::ExtensionMap bcoreMap;
            bcoreMap["bufferView"] = tinygltf::Value(bufferViewIndex++);
            tinygltf::ExtensionMap attributeMap;

            bool use_identity_mapping = false;
            std::unique_ptr<Mesh> out_mesh(new Mesh());
            out_mesh->SetNumFaces(iter_pris->indec.size() / 3);
            Mesh *mesh                  = out_mesh.get();
            PointCloud *out_point_cloud = static_cast<PointCloud *>(mesh);

            int posAttId   = -1;
            int texAttId   = -1;
            int normAttId  = -1;
            int colorAttId = -1;
            if (!iter_pris->position.empty())
            {
                out_point_cloud->set_num_points(iter_pris->indec.size());
                GeometryAttribute va;
                va.Init(GeometryAttribute::POSITION, nullptr, 3, DT_FLOAT32, false, sizeof(float) * 3, 0);
                posAttId                 = out_point_cloud->AddAttribute(va, use_identity_mapping, iter_pris->position.size() / 3);
                attributeMap["POSITION"] = tinygltf::Value(posAttId);
            }
            if (!iter_pris->normal.empty())
            {
                GeometryAttribute va;
                va.Init(GeometryAttribute::NORMAL, nullptr, 3, DT_FLOAT32, false, sizeof(float) * 3, 0);
                normAttId              = out_point_cloud->AddAttribute(va, use_identity_mapping, iter_pris->normal.size() / 3);
                attributeMap["NORMAL"] = tinygltf::Value(normAttId);
            }
            if (!iter_pris->color.empty())
            {
                GeometryAttribute va;
                va.Init(GeometryAttribute::COLOR, nullptr, 3, DT_FLOAT32, false, sizeof(float) * 3, 0);
                colorAttId              = out_point_cloud->AddAttribute(va, use_identity_mapping, iter_pris->color.size() / 3);
                attributeMap["COLOR_0"] = tinygltf::Value(colorAttId);
            }
            if (!iter_pris->uv.empty())
            {
                GeometryAttribute va;
                va.Init(GeometryAttribute::TEX_COORD, nullptr, 2, DT_FLOAT32, false, sizeof(float) * 2, 0);
                texAttId                   = out_point_cloud->AddAttribute(va, use_identity_mapping, iter_pris->uv.size() / 2);
                attributeMap["TEXCOORD_0"] = tinygltf::Value(texAttId);
            }

            /******************插入数据*****************/
            if (posAttId >= 0)
            {
                int numPosition = 0;
                for (int i = 0; i < iter_pris->position.size(); i += 3)
                {
                    float val[3] = {(float)iter_pris->position[i], (float)iter_pris->position[i + 1], (float)iter_pris->position[i + 2]};
                    out_point_cloud->attribute(posAttId)->SetAttributeValue(AttributeValueIndex(numPosition++), val);
                }
            }
            if (normAttId >= 0)
            {
                int numNormal = 0;
                for (int i = 0; i < iter_pris->normal.size(); i += 3)
                {
                    float val[3] = {(float)iter_pris->normal[i], (float)iter_pris->normal[i + 1], (float)iter_pris->normal[i + 2]};
                    out_point_cloud->attribute(normAttId)->SetAttributeValue(AttributeValueIndex(numNormal++), val);
                }
            }
            if (colorAttId >= 0)
            {
                int numColor = 0;
                for (int i = 0; i < iter_pris->color.size(); i += 3)
                {
                    float val[3] = {(float)iter_pris->color[i], (float)iter_pris->color[i + 1], (float)iter_pris->color[i + 2]};
                    out_point_cloud->attribute(colorAttId)->SetAttributeValue(AttributeValueIndex(numColor++), val);
                }
            }
            if (texAttId >= 0)
            {
                int numTex = 0;
                for (int i = 0; i < iter_pris->uv.size(); i += 2)
                {
                    float val[2] = {(float)iter_pris->uv[i], (float)iter_pris->uv[i + 1]};
                    out_point_cloud->attribute(texAttId)->SetAttributeValue(AttributeValueIndex(numTex++), val);
                }
            }

            for (int i = 0; i < iter_pris->indec.size(); ++i)
            {
                const PointIndex vertId(i);
                if (posAttId >= 0)
                {
                    out_point_cloud->attribute(posAttId)->SetPointMapEntry(vertId, AttributeValueIndex(iter_pris->indec[i]));
                }
                if (normAttId >= 0)
                {
                    out_point_cloud->attribute(normAttId)->SetPointMapEntry(vertId, AttributeValueIndex(iter_pris->indec[i]));
                }
                if (colorAttId >= 0)
                {
                    out_point_cloud->attribute(colorAttId)->SetPointMapEntry(vertId, AttributeValueIndex(iter_pris->indec[i]));
                }
                if (texAttId >= 0)
                {
                    out_point_cloud->attribute(texAttId)->SetPointMapEntry(vertId, AttributeValueIndex(iter_pris->indec[i]));
                }
            }
            /******************插入数据*****************/
            if (iter_pris->indec.size() > 0)
            {
                Mesh::Face face;
                for (FaceIndex i(0); i < iter_pris->indec.size() / 3; ++i)
                {
                    for (int c = 0; c < 3; ++c)
                    {
                        face[c] = 3 * i.value() + c;
                    }
                    out_mesh->SetFace(i, face);
                }
            }

            draco::Encoder encoder;
            encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, positionBit);
            encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, texBit);
            encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, normalBit);
            encoder.SetAttributeQuantization(draco::GeometryAttribute::COLOR, colorBit);
            encoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, genericBit);
            encoder.SetSpeedOptions(speed, speed);
            draco::EncoderBuffer buffer;
            const draco::Status status = encoder.EncodeMeshToBuffer(*out_mesh.get(), &buffer);

            bcoreMap["attributes"] = tinygltf::Value(attributeMap);
            extensionsPair         = std::make_pair("KHR_draco_mesh_compression", tinygltf::Value(bcoreMap));
            primitive.extensions.insert(extensionsPair);
            /************* create bufferview and buffer *****************/
            tinygltf::BufferView bufferViewIndec;
            bufferViewIndec.buffer     = 0;
            bufferViewIndec.byteOffset = bufferOffset;
            bufferViewIndec.byteLength = buffer.size();
            model->bufferViews.emplace_back(bufferViewIndec);
            bufferOffset += buffer.size();

            allBuffer.insert(allBuffer.end(), buffer.data(), buffer.data() + buffer.size());
            /************* create bufferview and buffer *****************/
        }
    }
    tinygltf::Buffer tinyBuffer;
    tinyBuffer.data = allBuffer;
    model->buffers.emplace_back(tinyBuffer);
}
}  // namespace gltf