// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

/*
 * @Author: yaol 
 * @Date: 2025-02-18 17:27:31 
 * @Last Modified by:   yaol 
 * @Last Modified time: 2025-02-18 17:27:31 
 */

#include <string>
#include <sstream>
#include <vector>
#include <cassert>
#include <stdint.h>  // For int32_t type
#include <cstdint>  // 添加这一行

#include "./xmlexporter.h"
#include "./xmltexturehelper.h"
#include "xmlgeomutils.h"
#include "utils.h"


#include <SketchUpAPI/import_export/pluginprogresscallback.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/model/component_definition.h>
#include <SketchUpAPI/model/component_instance.h>
#include <SketchUpAPI/model/drawing_element.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/entity.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/group.h>
#include <SketchUpAPI/model/layer.h>
#include <SketchUpAPI/model/loop.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/mesh_helper.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/texture.h>
#include <SketchUpAPI/model/texture_writer.h>
#include <SketchUpAPI/model/uv_helper.h>
#include <SketchUpAPI/model/vertex.h>

using namespace XmlGeomUtils;
#define pos(a, b) ((a) + ((b)*4))
// A simple SUStringRef wrapper class which makes usage simpler from C++.
class CSUString
{
  public:
    CSUString()
    {
        SUSetInvalid(su_str_);
        SUStringCreate(&su_str_);
    }

    ~CSUString() { SUStringRelease(&su_str_); }

    operator SUStringRef *() { return &su_str_; }

    std::string utf8()
    {
        size_t length;
        SUStringGetUTF8Length(su_str_, &length);
        std::string string;
        string.resize(length + 1);
        size_t returned_length;
        SUStringGetUTF8(su_str_, length, &string[0], &returned_length);
        return string;
    }

  private:
    SUStringRef su_str_;
};

// Utility function to get a material's name
static std::string GetMaterialName(SUMaterialRef material)
{
    CSUString name;
    SU_CALL(SUMaterialGetNameLegacyBehavior(material, name));
    return name.utf8();
}

// Utility function to get a layer's name
static std::string GetLayerName(SULayerRef layer)
{
    CSUString name;
    SU_CALL(SULayerGetName(layer, name));
    return name.utf8();
}


CXmlExporter::CXmlExporter()
{
    SUSetInvalid(model_);
    SUSetInvalid(texture_writer_);
}

CXmlExporter::~CXmlExporter() {}

void CXmlExporter::ReleaseModelObjects()
{
    if (!SUIsInvalid(texture_writer_))
    {
        SUTextureWriterRelease(&texture_writer_);
        SUSetInvalid(texture_writer_);
    }

    if (!SUIsInvalid(model_))
    {
        SUModelRelease(&model_);
        SUSetInvalid(model_);
    }

    // Terminate the SDK
    SUTerminate();
}

bool CXmlExporter::Convert(const std::string &src_file,
                           const std::string &file_name,
                           const std::string &file_path,
                           SketchUpPluginProgressCallback *progress_callback)
{
    bool exported = false;
    outPath       = file_path;
    try
    {
        // Initialize the SDK
        SUInitialize();

        // Create the model from the src_file
        SUSetInvalid(model_);
        SUModelLoadStatus status;
        SU_CALL(SUModelCreateFromFileWithStatus(&model_, src_file.c_str(), &status));
        // Create a texture writer
        SUSetInvalid(texture_writer_);
        SU_CALL(SUTextureWriterCreate(&texture_writer_));
        // Open the xml file for creation
        if (!file_.Open("123.xml", true))
        {
            ReleaseModelObjects();
            return exported;
        }
        // Materials
        std::cout << "WriteMaterials" << std::endl;
        WriteMaterials();
        // Geometry
        std::cout << "WriteGeometry" << std::endl;
        WriteGeometry();
        file_.Close(IsCancelled(progress_callback));
        
        // 在导出到GLTF之前压缩纹理
        CompressAndResizeTextures();
        
        exportToGltfImpl(file_path + file_name);
        exported = true;
    }
    catch (...)
    {
        exported = false;
        file_.Close(true);
    }
    ReleaseModelObjects();
    return exported;
}

static void WriteMaterialsTextureImage(SUMaterialRef material, const std::string &texture_image_file)
{
    assert(SUIsValid(material));
    // Only write the material's texture if a non-empty name was provided
    if (texture_image_file.empty())
        return;
    SUTextureRef texture = SU_INVALID;
    if (SUMaterialGetTexture(material, &texture) != SU_ERROR_NONE)
        return;
    // Write the texture using the provided file name
    SU_CALL(SUTextureWriteToFile(texture, texture_image_file.c_str()));
}

static XmlMaterialInfo GetMaterialInfo(SUMaterialRef material, const std::string &texture_directory)
{
    assert(SUIsValid(material));

    XmlMaterialInfo info;

    // Name
    info.name_ = GetMaterialName(material);

    // Color
    info.has_color_ = false;
    info.has_alpha_ = false;
    SUMaterialType type;
    SU_CALL(SUMaterialGetType(material, &type));
    // Color
    if ((type == SUMaterialType_Colored) || (type == SUMaterialType_ColorizedTexture))
    {
        SUColor color;
        if (SUMaterialGetColor(material, &color) == SU_ERROR_NONE)
        {
            info.has_color_ = true;
            info.color_     = color;
        }
    }

    // Alpha
    bool has_alpha = false;
    SU_CALL(SUMaterialGetUseOpacity(material, &has_alpha));
    if (has_alpha)
    {
        double alpha = 0;
        SU_CALL(SUMaterialGetOpacity(material, &alpha));
        info.has_alpha_ = true;
        info.alpha_     = alpha;
    }

    // Texture
    info.has_texture_ = false;
    if ((type == SUMaterialType_Textured) || (type == SUMaterialType_ColorizedTexture))
    {
        SUTextureRef texture = SU_INVALID;
        if (SUMaterialGetTexture(material, &texture) == SU_ERROR_NONE)
        {
            info.has_texture_ = true;
            // Get the PID from the texture to generate a unique output file name
            int32_t tex_id = 0;
            SU_CALL(SUEntityGetID(SUTextureToEntity(texture), &tex_id));
            // Generate a unique name for this material's texture
            std::stringstream sstream;
            sstream << texture_directory << "Texture" << tex_id << ".png";
            std::stringstream sstream_pic;
            sstream_pic << "Texture" << tex_id << ".png";
            info.texture_path_ = sstream.str();
            info.picture_name_ = sstream_pic.str();
            // Texture scale
            size_t width   = 0;
            size_t height  = 0;
            double s_scale = 0.0;
            double t_scale = 0.0;
            SU_CALL(SUTextureGetDimensions(texture, &width, &height, &s_scale, &t_scale));
            info.texture_sscale_ = s_scale;
            info.texture_tscale_ = t_scale;
        }
    }

    return info;
}

void CXmlExporter::WriteMaterials()
{
    if (options_.export_materials())
    {
        if (options_.export_materials_by_layer())
        {
            size_t num_layers;
            SU_CALL(SUModelGetNumLayers(model_, &num_layers));
            if (num_layers > 0)
            {
                std::vector<SULayerRef> layers(num_layers);
                SU_CALL(SUModelGetLayers(model_, num_layers, &layers[0], &num_layers));
                for (size_t i = 0; i < num_layers; i++)
                {
                    SULayerRef layer       = layers[i];
                    SUMaterialRef material = SU_INVALID;
                    if (SULayerGetMaterial(layer, &material) == SU_ERROR_NONE)
                    {
                        WriteMaterial(material);
                    }
                }
            }
        }
        else
        {
            size_t count = 0;
            SU_CALL(SUModelGetNumMaterials(model_, &count));
            if (count > 0)
            {
                std::vector<SUMaterialRef> materials(count);
                SU_CALL(SUModelGetMaterials(model_, count, &materials[0], &count));
                for (size_t i = 0; i < count; i++)
                {
                    WriteMaterial(materials[i]);
                }
            }
        }
    }
}

void CXmlExporter::WriteMaterial(SUMaterialRef material)
{
    if (SUIsInvalid(material))
        return;

    XmlMaterialInfo info    = GetMaterialInfo(material, outPath);
    materialMap[info.name_] = info;
    if (!info.texture_path_.empty())
    {
        std::cout << info.texture_path_ << std::endl;
    }
    WriteMaterialsTextureImage(material, info.texture_path_);
    file_.WriteMaterialInfo(info);
}

void CXmlExporter::WriteGeometry()
{
    SUTransformation transformation = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    if (options_.export_faces() || options_.export_edges())
    {
        // Write entities
        SUEntitiesRef model_entities;
        SU_CALL(SUModelGetEntities(model_, &model_entities));
        file_.StartGeometry();
        
        // 使用批处理替代直接处理
        ProcessGeometryBatch(model_entities, transformation, DEFAULT_BATCH_SIZE);
        
        // 确保处理完所有剩余数据
        faceBuffer.clear();
        faceBuffer.shrink_to_fit();
        
        file_.PopParentNode();
    }
}

void CXmlExporter::ProcessGeometryBatch(SUEntitiesRef entities, 
                                      const SUTransformation& transformation,
                                      size_t batchSize) {
    size_t num_faces = 0;
    size_t num_groups = 0;
    size_t num_instances = 0;
    
    SU_CALL(SUEntitiesGetNumFaces(entities, &num_faces));
    SU_CALL(SUEntitiesGetNumGroups(entities, &num_groups));
    SU_CALL(SUEntitiesGetNumInstances(entities, &num_instances));
    
    std::cout << "Processing entity with:" << std::endl;
    std::cout << "- Faces: " << num_faces << std::endl;
    std::cout << "- Groups: " << num_groups << std::endl; 
    std::cout << "- Instances: " << num_instances << std::endl;
    
    // 处理直接的面
    if (num_faces > 0) {
        std::vector<SUFaceRef> faces(std::min<size_t>(batchSize, num_faces));
        
        for (size_t offset = 0; offset < num_faces; offset += batchSize) {
            size_t currentBatchSize = std::min<size_t>(batchSize, num_faces - offset);
            faces.resize(currentBatchSize);
            
            SU_CALL(SUEntitiesGetFaces(entities, currentBatchSize, &faces[0], &currentBatchSize));
            
            for (size_t i = 0; i < currentBatchSize; i++) {
                inheritance_manager_.PushElement(faces[i]);
                WriteFace(faces[i], transformation);
                inheritance_manager_.PopElement();
            }
        }
        faces.clear();
        faces.shrink_to_fit();
    } else {
        // 处理组和组件
        if (num_groups > 0) {
            traversalGroupEntity(entities, transformation);
        }
        
        if (num_instances > 0) {
            getComponentEntity(entities, transformation);
        }
    }
}

void CXmlExporter::WriteEntities(SUEntitiesRef entities, SUTransformation &transformation)
{
    if (SUIsInvalid(entities)) {
        return;
    }
    
    // 使用 ProcessGeometryBatch 统一处理所有实体
    ProcessGeometryBatch(entities, transformation, DEFAULT_BATCH_SIZE);
}
void CXmlExporter::traversalGroupEntity(SUEntitiesRef entities, const SUTransformation &transformation)
{
    // Groups
    size_t num_groups = 0;
    SU_CALL(SUEntitiesGetNumGroups(entities, &num_groups));
    if (num_groups > 0)
    {
        std::vector<SUGroupRef> groups(num_groups);
        SU_CALL(SUEntitiesGetGroups(entities, num_groups, &groups[0], &num_groups));
        for (size_t g = 0; g < num_groups; g++)
        {
            SUGroupRef group                         = groups[g];
            SUComponentDefinitionRef group_component = SU_INVALID;
            SUEntitiesRef group_entities             = SU_INVALID;
            SU_CALL(SUGroupGetEntities(group, &group_entities));
            SUTransformation transforMation2   = {0};
            SUTransformation resTransforMation = {0};
            memset(resTransforMation.values, 0, sizeof(resTransforMation.values));
            SU_CALL(SUGroupGetTransform(group, &transforMation2));

            SUDrawingElementRef drawing_element = SUGroupToDrawingElement(group);
            SULayerRef layer;
            SUSetInvalid(layer);
            SUDrawingElementGetLayer(drawing_element, &layer);
            bool is_visible = true;
            SU_CALL(SULayerGetVisibility(layer, &is_visible));
            if (is_visible)
            {
                inheritance_manager_.PushElement(group);
                for (int kk = 0; kk < 4; kk++)
                    for (int ii = 0; ii < 4; ii++)
                        for (int jj = 0; jj < 4; jj++)
                            resTransforMation.values[pos(ii, jj)] += transformation.values[pos(ii, kk)] * transforMation2.values[pos(kk, jj)];
                WriteEntities(group_entities, resTransforMation);
                inheritance_manager_.PopElement();
            }
        }
    }
}
void CXmlExporter::getComponentEntity(SUEntitiesRef entities, const SUTransformation &transformation)
{
    size_t componentInstanceLen;
    SUEntitiesGetNumInstances(entities, &componentInstanceLen);
    if (componentInstanceLen > 0)
    {
        std::vector<SUComponentInstanceRef> componentInstanceArr(componentInstanceLen);
        SUEntitiesGetInstances(entities, componentInstanceLen, &componentInstanceArr[0], &componentInstanceLen);
        for (size_t i = 0; i < componentInstanceLen; i++)
        {
            /*
                Get definition from instance of component
            */
            SUComponentDefinitionRef definitionOfInstance;
            SUComponentInstanceGetDefinition(componentInstanceArr[i], &definitionOfInstance);
            /*
                Get entities from component definition
            */
            SUEntitiesRef entitiesInDefinition;
            SUComponentDefinitionGetEntities(definitionOfInstance, &entitiesInDefinition);

            SUTransformation transforMation2   = {0};
            SUTransformation resTransforMation = {0};
            memset(resTransforMation.values, 0, sizeof(resTransforMation.values));

            SUComponentInstanceGetTransform(componentInstanceArr[i], &transforMation2);
            transforMation2.values[12] = transforMation2.values[12] * ratio;
            transforMation2.values[13] = transforMation2.values[13] * ratio;
            transforMation2.values[14] = transforMation2.values[14] * ratio;

            SUDrawingElementRef drawing_element = SUComponentInstanceToDrawingElement(componentInstanceArr[i]);
            SULayerRef layer;
            SUSetInvalid(layer);
            SUDrawingElementGetLayer(drawing_element, &layer);
            bool is_visible = true;
            SU_CALL(SULayerGetVisibility(layer, &is_visible));
            if (is_visible)
            {
                inheritance_manager_.PushElement(componentInstanceArr[i]);
                for (int kk = 0; kk < 4; kk++)
                    for (int ii = 0; ii < 4; ii++)
                        for (int jj = 0; jj < 4; jj++)
                            resTransforMation.values[pos(ii, jj)] += transformation.values[pos(ii, kk)] * transforMation2.values[pos(kk, jj)];

                WriteEntities(entitiesInDefinition, resTransforMation);
                inheritance_manager_.PopElement();
            }
        }
    }
}

int CXmlExporter::exportToGltfImpl(const std::string &gltfName)
{
    gltf::CreateGltfCommon createGltf;
    std::vector<gltf::Node> nodeVec;
    std::vector<gltf::MeshData> meshDataVec;
    int meshIndex = 0;
    for (auto &item : facetMap)
    {
        gltf::Node node;
        node.meshIndex = meshIndex++;
        node.guid      = "node_" + std::to_string(meshIndex);
        nodeVec.emplace_back(node);
        size_t faceIndex = 0;
        gltf::MeshData meshData;
        gltf::Primitive pri;
        std::vector<cFacet> facetVec = item.second;
        for (int i = 0; i < facetVec.size(); i++)
        {
            tinygltf::Material material;
            material.pbrMetallicRoughness.baseColorFactor = {item.first.r, item.first.g, item.first.b, item.first.a};
            material.pbrMetallicRoughness.metallicFactor  = 0.0;
            material.name                                 = item.first.name;
            pri.imageUri                                  = item.first.imageUri;
            pri.material                                  = material;
            for (int j = 0; j < 3; j++)
            {
                pri.position.emplace_back(facetVec[i].vertex[j].x);
                pri.position.emplace_back(facetVec[i].vertex[j].y);
                pri.position.emplace_back(facetVec[i].vertex[j].z);
                pri.indec.emplace_back(faceIndex++);
                if (!facetVec.empty())
                {
                    pri.uv.emplace_back(facetVec[i].uv[j].x);
                    pri.uv.emplace_back(-facetVec[i].uv[j].y);
                }
            }
        }
        meshData.primitives.emplace_back(pri);
        meshDataVec.emplace_back(meshData);
    }
    createGltf.setMeshData(nodeVec, meshDataVec);
    createGltf.createGltf("gltf", gltfName);
    return 0;
}

void CXmlExporter::WriteFace(SUFaceRef face, const SUTransformation &transformation)
{
    if (SUIsInvalid(face))
        return;

    XmlFaceInfo info;

    // Get Current layer off of our stack and then get the id from it
    SULayerRef layer = inheritance_manager_.GetCurrentLayer();
    if (!SUIsInvalid(layer))
    {
        info.layer_name_ = GetLayerName(layer);
    }

    // Get the current front and back materials off of our stack
    SUMaterialRef front_material = inheritance_manager_.GetCurrentFrontMaterial();
    if (!SUIsInvalid(front_material))
    {
        // Material name
        info.front_mat_name_ = GetMaterialName(front_material);
        // std::cout << "info.front_mat_name_=" << info.front_mat_name_ << std::endl;

        // Has texture ?
        SUTextureRef texture_ref = SU_INVALID;
        info.has_front_texture_  = SUMaterialGetTexture(front_material, &texture_ref) == SU_ERROR_NONE;
    }
    SUMaterialRef back_material = inheritance_manager_.GetCurrentBackMaterial();
    if (!SUIsInvalid(back_material))
    {
        // Material name
        info.back_mat_name_ = GetMaterialName(back_material);

        // Has texture ?
        SUTextureRef texture_ref = SU_INVALID;
        info.has_back_texture_   = SUMaterialGetTexture(back_material, &texture_ref) == SU_ERROR_NONE;
    }

    // Get a uv helper
    SUUVHelperRef uv_helper = SU_INVALID;
    SUFaceGetUVHelper(face, info.has_front_texture_, info.has_back_texture_, texture_writer_, &uv_helper);

    // If this is a complex face with one or more holes in it
    // we tessellate it into triangles using the polygon mesh class, then
    // export each triangle as a face.
    info.has_single_loop_ = false;

    // Create and process mesh
    SUMeshHelperRef mesh_ref = SU_INVALID;
    SU_CALL(SUMeshHelperCreateWithTextureWriter(&mesh_ref, face, texture_writer_));

    // Get the vertices
    size_t num_vertices = 0;
    SU_CALL(SUMeshHelperGetNumVertices(mesh_ref, &num_vertices));
    if (num_vertices == 0)
        return;
    std::vector<SUPoint3D> vertices(num_vertices);
    SU_CALL(SUMeshHelperGetVertices(mesh_ref, num_vertices, &vertices[0], &num_vertices));

    // Get triangle indices.
    size_t num_triangles = 0;
    SU_CALL(SUMeshHelperGetNumTriangles(mesh_ref, &num_triangles));

    std::vector<SUVector3D> normalArr(num_triangles);
    size_t num_normals = 0;
    SUMeshHelperGetNormals(mesh_ref, num_triangles, &normalArr[0], &num_normals);

    const size_t num_indices = 3 * num_triangles;
    size_t num_retrieved     = 0;
    std::vector<size_t> indices(num_indices);
    SU_CALL(SUMeshHelperGetVertexIndices(mesh_ref, num_indices, &indices[0], &num_retrieved));

    // Get UV coords.
    std::vector<SUPoint3D> front_stq(num_vertices);
    std::vector<SUPoint3D> back_stq(num_vertices);
    size_t count;
    if (info.has_front_texture_)
    {
        SU_CALL(SUMeshHelperGetFrontSTQCoords(mesh_ref, num_vertices, &front_stq[0], &count));
    }
    if (info.has_back_texture_)
    {
        SU_CALL(SUMeshHelperGetBackSTQCoords(mesh_ref, num_vertices, &back_stq[0], &count));
    }
    XmlMaterialInfo &materialInfo     = materialMap[info.front_mat_name_];
    XmlMaterialInfo &materialInfoBack = materialMap[info.back_mat_name_];
    Color color;
    Color colorBack;
    if (info.front_mat_name_ != "")
    {
        if (materialInfo.has_color_)
        {
            color.r = (double)(materialInfo.color_.red) / 255;
            color.g = (double)(materialInfo.color_.green) / 255;
            color.b = (double)(materialInfo.color_.blue) / 255;
        }
        else
        {
            color.r = 1.0;
            color.g = 1.0;
            color.b = 1.0;
        }
        if (materialInfo.has_alpha_)
        {
            color.a = (double)(materialInfo.color_.alpha) / 255;
        }
        else
        {
            color.a = 1.0;
        }
        color.name = info.front_mat_name_;
        if (info.has_front_texture_)
        {
            color.imageUri = materialInfo.picture_name_;
        }
    }
    else
    {
        color.r        = 1.0;
        color.g        = 1.0;
        color.b        = 1.0;
        color.a        = 1.0;
        color.name     = "default";
        color.imageUri = materialInfo.picture_name_;
    }
    if (info.back_mat_name_ != "")
    {
        if (materialInfoBack.has_color_)
        {
            colorBack.r = (double)(materialInfoBack.color_.red) / 255;
            colorBack.g = (double)(materialInfoBack.color_.green) / 255;
            colorBack.b = (double)(materialInfoBack.color_.blue) / 255;
        }
        else
        {
            colorBack.r = 1.0;
            colorBack.g = 1.0;
            colorBack.b = 1.0;
        }
        if (materialInfoBack.has_alpha_)
        {
            colorBack.a = (double)(materialInfoBack.color_.alpha) / 255;
        }
        else
        {
            colorBack.a = 1.0;
        }
        colorBack.name = info.front_mat_name_;
        if (info.has_back_texture_)
        {
            colorBack.imageUri = materialInfoBack.picture_name_;
        }
    }
    
    std::vector<size_t> optimizedIndices;
    std::vector<Vector3> uniqueVerticesVec;
    std::vector<Vector3> uniqueUVsVec;

    for (size_t i = 0; i < num_triangles; i++) {
        for (size_t j = 0; j < 3; j++) {
            size_t index = indices[i * 3 + j];
            
            VertexKey key;
            // 转换顶点坐标
            double vertex[3] = {vertices[index].x, vertices[index].y, vertices[index].z};
            double transformed[3] = {0, 0, 0};
            
            for (int ii = 0; ii < 3; ii++) {
                for (int jj = 0; jj < 3; jj++) {
                    transformed[ii] += vertex[jj] * transformation.values[pos(ii, jj)];
                }
                transformed[ii] += transformation.values[pos(ii, 3)];
            }
            
            key.x = transformed[0];
            key.y = transformed[1];
            key.z = transformed[2];
            
            // 设置UV坐标
            if (info.has_front_texture_) {
                SUPoint3D stq = front_stq[index];
                key.u = stq.x * materialInfo.texture_sscale_;
                key.v = stq.y * materialInfo.texture_tscale_;
            } else {
                key.u = key.v = 0.0;
            }
            
            // 获取或创建新的顶点索引
            size_t optimizedIndex = GetOrCreateVertexIndex(key);
            optimizedIndices.push_back(optimizedIndex);
            
            if (optimizedIndex == uniqueVerticesVec.size()) {
                // 这是一个新顶点
                uniqueVerticesVec.push_back(Vector3(transformed[0], transformed[1], transformed[2]));
                if (info.has_front_texture_) {
                    uniqueUVsVec.push_back(Vector3(key.u, key.v, 0));
                }
            }
        }

        // 创建面片并添加到facetMap
        cFacet aFacet;
        // ... 设置法线和颜色 ...
        
        for (int j = 0; j < 3; j++) {
            size_t idx = optimizedIndices[i * 3 + j];
            aFacet.vertex[j] = uniqueVerticesVec[idx];
            if (info.has_front_texture_) {
                aFacet.uv[j] = uniqueUVsVec[idx];
            }
        }
        
        // 添加到facetMap
        Color color1((double)color.r, (double)color.g, (double)color.b, 
                    (double)color.a, color.imageUri, color.name);
        facetMap[color1].push_back(aFacet);
    }

    // 清理本次处理的顶点缓存
    ClearVertexCache();
    
    SU_CALL(SUMeshHelperRelease(&mesh_ref));  // 及时释放mesh资源
    SU_CALL(SUUVHelperRelease(&uv_helper));
}

// 添加新的方法实现
void CXmlExporter::CompressAndResizeTextures() {
    for (auto& materialPair : materialMap) {
        const auto& materialInfo = materialPair.second;
        if (!materialInfo.texture_path_.empty()) {
            materialPair.second.picture_name_ = ProcessTexture(materialInfo.texture_path_);
        }
    }
}

std::string CXmlExporter::ProcessTexture(const std::string& texturePath) {
    // 检查缓存
    if (textureCache.find(texturePath) != textureCache.end()) {
        return textureCache[texturePath].compressedPath;
    }

    // 实现纹理压缩和缩放
    CompressedTexture compressedInfo;
    compressedInfo.originalPath = texturePath;
    
    // TODO: 使用图像处理库实现实际的压缩
    // 这里需要添加实际的图像压缩代码
    // 建议使用 stb_image 或 OpenCV 等库
    
    // 临时使用原始路径
    compressedInfo.compressedPath = texturePath;
    compressedInfo.isCompressed = false;
    
    textureCache[texturePath] = compressedInfo;
    return compressedInfo.compressedPath;
}

size_t CXmlExporter::GetOrCreateVertexIndex(const VertexKey& key) {
    auto it = vertexCache.find(key);
    if (it != vertexCache.end()) {
        return it->second;
    }
    
    size_t newIndex = uniqueVertices.size();
    vertexCache[key] = newIndex;
    uniqueVertices.push_back(key);
    return newIndex;
}

void CXmlExporter::ClearVertexCache() {
    vertexCache.clear();
    uniqueVertices.clear();
}
