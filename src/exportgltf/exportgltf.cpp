/**
 * @file	stl_exporter.cc
 * @author	Joseph Lee <joseph@jc-lab.net>
 * @date	2021/05/29
 * @copyright Copyright (C) 2021 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#include "exportgltf.h"
#include <memory>
#include <vector>
#include <cstdarg>
#include <string>
#include <unordered_map>

#define pos(a, b) ((a) + ((b)*4))

namespace skp2gltf
{
class Color
{
  public:
    Color(double _r, double _g, double _b, double _a, std::string _imageUri) : r(_r), g(_g), b(_b), a(_a), imageUri(_imageUri) {}
    double r;
    double g;
    double b;
    double a;
    std::string imageUri = "";
    bool operator==(const Color &t) const { return r == t.r && g == t.g && b == t.b && a == t.a && imageUri == t.imageUri; }
};

struct colorHashFuc
{
    size_t operator()(const Color &key) const
    {
        return ((std::hash<int>()((int)key.r * 10000) ^ (std::hash<int>()((int)key.g * 10000) << 1)) >> 1) ^
               (std::hash<int>()((int)key.b * 10000) << 1);
    }
};

class Vector3
{
  public:
    Vector3() {}
    Vector3(double _x, double _y, double _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
    double x, y, z;
    Vector3 Cross(const Vector3 &V)
    {
        Vector3 v;
        v.x = y * V.z - z * V.y;
        v.y = z * V.x - x * V.z;
        v.z = x * V.y - y * V.x;
        return v;
    }
    double Dot(const Vector3 &V)
    {
        double res;
        res = x * V.x + y * V.y + z * V.z;
        return res;
    }
    Vector3 operator-(const Vector3 V)
    {
        Vector3 v;
        v.x = x - V.x;
        v.y = y - V.y;
        v.z = z - V.z;
        return v;
    }
};

class cFacet
{
  public:
    Vector3 normal, vertex[3], uv[3];
    double color[4];
    std::string imageUri = "";

    Vector3 Middle()
    {
        Vector3 v;
        v.x = v.y = v.z = 0;
        for (int i = 0; i < 3; i++)
        {
            v.x += vertex[i].x;
            v.y += vertex[i].y;
            v.z += vertex[i].z;
        }
        v.x /= 3;
        v.y /= 3;
        v.z /= 3;
        return v;
    }
};
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
    // Disallow copying for simplicity
    CSUString(const CSUString &copy);
    CSUString &operator=(const CSUString &copy);

    SUStringRef su_str_;
};
class FormattedWriter
{
  protected:
    virtual int write(const char *data, size_t length) = 0;

  public:
    void outf(const char *format, ...)
    {
        char buf[256];
        size_t len;

        std::va_list vl;
        va_start(vl, format);
        len = vsnprintf(buf, sizeof(buf), format, vl);
        va_end(vl);

        write(buf, len);
    }
};

class FstreamWriter : public FormattedWriter
{
  private:
    std::fstream &stream_;

  public:
    FstreamWriter(std::fstream &stream) : stream_(stream) {}

  protected:
    int write(const char *data, size_t length) override
    {
        stream_.write(data, length);
        return 0;
    }
};

class GltfExporterImpl : public GltfExporter
{
  private:
    int triangle_count_;
    int face_count_;
    int entities_count_;
    double ratio;
    SUTransformation matrix_i_;
    SUTextureWriterRef texture_writer;
    std::vector<cFacet> facet_arr_;
    std::unordered_map<Color, std::vector<cFacet>, colorHashFuc> facetMap;
    std::unordered_map<std::string, SUMaterialRef> materialMap;
    std::unordered_map<std::string, std::string> materialNameMap;

  public:
    GltfExporterImpl() {}

    int loadFromEntities(SUEntitiesRef entities, std::string filePath, double _ratio) override
    {
        ratio = _ratio;
        init();
        traversalEntities(entities, 0, matrix_i_);
        textureWriteToFile(filePath);
        return 0;
    }
    int exportToGltf(const std::string gltfName) const override { return exportToGltfImpl(gltfName); }

  private:
    void textureWriteToFile(std::string path)
    {
        int textureIndex = 0;
        for (auto &item : materialMap)
        {
            SUTextureRef texture = SU_INVALID;
            SUMaterialGetTexture(item.second, &texture);
            size_t imsize, bits_per_pixel;
            SUTextureGetImageDataSize(texture, &imsize, &bits_per_pixel);
            size_t width, height;
            double s, t;
            SUByte *data = new SUByte[imsize];
            SUTextureGetImageData(texture, imsize, data);
            SUTextureGetDimensions(texture, &width, &height, &s, &t);
            SUTextureCreateFromImageData(&texture, width, height, bits_per_pixel, data);
            std::string fileName = "texture_" + std::to_string(textureIndex++) + ".png";
            SUTextureWriteToFile(texture, (path + fileName).c_str());
            materialNameMap[item.first] = fileName;
        }
    }
    int exportToGltfImpl(const std::string &gltfName) const
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
                material.pbrMetallicRoughness.baseColorFactor = {facetVec[i].color[0], facetVec[i].color[1], facetVec[i].color[2],
                                                                 facetVec[i].color[3]};
                material.pbrMetallicRoughness.metallicFactor  = 0.0;
                if (facetVec[i].imageUri != "")
                {
                    auto filePath = materialNameMap.find(facetVec[i].imageUri);
                    pri.imageUri  = filePath->second;
                }
                pri.material = material;
                for (int j = 0; j < 3; j++)
                {
                    pri.normal.emplace_back(facetVec[i].normal.x);
                    pri.normal.emplace_back(facetVec[i].normal.y);
                    pri.normal.emplace_back(facetVec[i].normal.z);
                    pri.position.emplace_back(facetVec[i].vertex[j].x);
                    pri.position.emplace_back(facetVec[i].vertex[j].y);
                    pri.position.emplace_back(facetVec[i].vertex[j].z);
                    pri.uv.emplace_back(facetVec[i].uv[j].x);
                    pri.uv.emplace_back(-facetVec[i].uv[j].y);
                    pri.indec.emplace_back(faceIndex++);
                }
            }
            meshData.primitives.emplace_back(pri);
            meshDataVec.emplace_back(meshData);
        }
        createGltf.setMeshData(nodeVec, meshDataVec);
        createGltf.createGltf("gltf", gltfName);
        return 0;
    }

    void init()
    {
        triangle_count_ = 0;
        face_count_     = 0;
        entities_count_ = 0;
        for (size_t i = 0; i < 16; i++)
            matrix_i_.values[i] = 0;
        for (size_t i = 0; i < 4; i++)
            matrix_i_.values[pos(i, i)] = 1;
    }

    void getComponentEntity(SUEntitiesRef entities, int recursive_depth, const SUTransformation &transformation)
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

                for (int kk = 0; kk < 4; kk++)
                    for (int ii = 0; ii < 4; ii++)
                        for (int jj = 0; jj < 4; jj++)
                            resTransforMation.values[pos(ii, jj)] += transformation.values[pos(ii, kk)] * transforMation2.values[pos(kk, jj)];

                traversalEntities(entitiesInDefinition, recursive_depth, resTransforMation);
            }
        }
    }

    void traversalGroupEntity(SUEntitiesRef entities, int recursive_depth, const SUTransformation &transformation)
    {
        size_t groupLen;
        SUEntitiesGetNumGroups(entities, &groupLen);
        if (groupLen > 0)
        {
            std::vector<SUGroupRef> groupArr(groupLen);
            SUEntitiesGetGroups(entities, groupLen, &groupArr[0], &groupLen);
            std::cout << "groupLen=" << groupLen << std::endl;
            std::cout << "entities=" << &entities << std::endl;
            for (size_t i = 0; i < groupLen; i++)
            {
                SUEntitiesRef entitiesInGroup;
                SUGroupGetEntities(groupArr[i], &entitiesInGroup);

                SUTransformation transforMation2   = {0};
                SUTransformation resTransforMation = {0};
                memset(resTransforMation.values, 0, sizeof(resTransforMation.values));

                SUGroupGetTransform(groupArr[i], &transforMation2);
                transforMation2.values[12] = transforMation2.values[12] * ratio;
                transforMation2.values[13] = transforMation2.values[13] * ratio;
                transforMation2.values[14] = transforMation2.values[14] * ratio;
                for (int kk = 0; kk < 4; kk++)
                    for (int ii = 0; ii < 4; ii++)
                        for (int jj = 0; jj < 4; jj++)
                            resTransforMation.values[pos(ii, jj)] += transformation.values[pos(ii, kk)] * transforMation2.values[pos(kk, jj)];

                traversalEntities(entitiesInGroup, recursive_depth, resTransforMation);
            }
        }
    }
    void ErrorHandler(SUResult res)
    {
        if (res == SU_ERROR_NONE)
            return;

        std::cout << "ERROR DETECTED \n";
        switch (res)
        {
            case SU_ERROR_NULL_POINTER_INPUT:
                std::cout << "SU_ERROR_NULL_POINTER_INPUT \n";
                break;
            case SU_ERROR_INVALID_INPUT:
                std::cout << "SU_ERROR_INVALID_INPUT \n";
                break;
            case SU_ERROR_NULL_POINTER_OUTPUT:
                std::cout << "Error: Null Pointer Output \n";
                break;
            case SU_ERROR_INVALID_OUTPUT:
                std::cout << "SU_ERROR_INVALID_OUTPUT \n";
                break;
            case SU_ERROR_OVERWRITE_VALID:
                std::cout << "SU_ERROR_OVERWRITE_VALID \n";
                break;
            case SU_ERROR_GENERIC:
                std::cout << "SU_ERROR_GENERIC \n";
                break;
            case SU_ERROR_SERIALIZATION:
                std::cout << "SU_ERROR_SERIALIZATION \n";
                break;
            case SU_ERROR_OUT_OF_RANGE:
                std::cout << "SU_ERROR_OUT_OF_RANGE \n";
                break;
            case SU_ERROR_NO_DATA:
                std::cout << "SU_ERROR_NO_DATA \n";
                break;
            case SU_ERROR_INSUFFICIENT_SIZE:
                std::cout << "SU_ERROR_INSUFFICIENT_SIZE \n";
                break;
            case SU_ERROR_UNKNOWN_EXCEPTION:
                std::cout << "SU_ERROR_UNKNOWN_EXCEPTION \n";
                break;
            case SU_ERROR_MODEL_INVALID:
                std::cout << "SU_ERROR_MODEL_INVALID \n";
                break;
            case SU_ERROR_MODEL_VERSION:
                std::cout << "SU_ERROR_MODEL_INVALID \n";
                break;
            default:
                std::cout << "Unlisted Error \n";
        }
        exit(EXIT_FAILURE);
    }
    std::string convertUtf8(SUStringRef str)
    {
        size_t length;
        SUStringGetUTF8Length(str, &length);
        std::string string;
        string.resize(length + 1);
        size_t returned_length;
        SUStringGetUTF8(str, length, &string[0], &returned_length);
        return string;
    }
    std::string StringReplace(std::string in, std::string find, std::string replace)
    {
        while (true)
        {
            size_t pos = in.find(find);
            if (pos == -1)
                break;

            in = in.substr(0, pos) + replace + in.substr(pos + find.length());
        }
        return in;
    }
    std::string CleanString(std::string s)
    {
        std::string allow = "_|.-";
        bool keepFlag     = false;
        if (s[0] == '[')
            s = s.substr(1, s.length() - 1);

        for (int i = 0; i < s.length(); i++)
        {
            if (s[i] >= 'a' && s[i] <= 'z')
                continue;
            if (s[i] >= 'A' && s[i] <= 'Z')
                continue;
            if (s[i] >= '0' && s[i] <= '9')
                continue;

            keepFlag = false;
            for (int j = 0; j < allow.length(); j++)
            {
                keepFlag = keepFlag || allow[j] == s[i];
            }
            if (!keepFlag)
                s[i] = '_';
        }
        return s;
    }
    void addFaces(SUEntitiesRef entities, SUTransformation &transformation, int recursive_depth)
    {
        int su_result;
        size_t faceLen = 0;
        SUEntitiesGetNumFaces(entities, &faceLen);
        if (faceLen > 0)
        {
            std::vector<SUFaceRef> faces(faceLen);
            SUEntitiesGetFaces(entities, faceLen, &faces[0], &faceLen);
            for (size_t i = 0; i < faceLen; i++)
            {
                SUMeshHelperRef face_mesh = SU_INVALID;

                su_result = SUMeshHelperCreate(&face_mesh, faces[i]);

                size_t triangleLen;
                SUMeshHelperGetNumTriangles(face_mesh, &triangleLen);
                std::vector<SUVector3D> normalArr(triangleLen);
                SUMeshHelperGetNormals(face_mesh, triangleLen, &normalArr[0], &triangleLen);

                size_t vertexLen;
                SUMeshHelperGetNumVertices(face_mesh, &vertexLen);
                std::vector<SUPoint3D> vertexArr(vertexLen);
                SUMeshHelperGetVertices(face_mesh, vertexLen, &vertexArr[0], &vertexLen);
                for (auto &item : vertexArr)
                {
                    item.x = item.x * ratio;
                    item.y = item.y * ratio;
                    item.z = item.z * ratio;
                }

                std::vector<SUPoint3D> texture_coords(vertexLen);
                size_t num_coords;
                SUMeshHelperGetFrontSTQCoords(face_mesh, vertexLen, &texture_coords[0], &num_coords);

                SUMaterialRef material = SU_INVALID;
                SUFaceGetFrontMaterial(faces[i], &material);

                // SUMaterialRef material2 = SU_INVALID;
                // SUFaceGetBackMaterial(faces[i], &material2);
                std::string imageUri = "";
                if (material.ptr == nullptr)
                {
                    SUFaceGetBackMaterial(faces[i], &material);
                }
                if (material.ptr != nullptr)
                {
                    SUTextureRef texture = SU_INVALID;
                    SUMaterialGetTexture(material, &texture);
                    if (texture.ptr != nullptr)
                    {
                        CSUString name;
                        SUMaterialGetNameLegacyBehavior(material, name);
                        imageUri              = name.utf8();
                        materialMap[imageUri] = material;
                    }
                }
                bool has_alpha = false;
                SUMaterialGetUseOpacity(material, &has_alpha);
                double alpha;
                SUMaterialGetOpacity(material, &alpha);
                if (!has_alpha)
                {
                    alpha = 1.0;
                }

                SUColor color = SU_INVALID;
                SUMaterialGetColor(material, &color);
                if (alpha < 1)
                {
                    std::cout << "color.a=" << alpha << std::endl;
                }
                if (material.ptr == nullptr)
                {
                    color.red   = 255;
                    color.green = 255;
                    color.blue  = 255;
                }

                size_t indexLen = triangleLen * 3;
                std::vector<size_t> indexArr(indexLen);
                SUMeshHelperGetVertexIndices(face_mesh, indexLen, &indexArr[0], &indexLen);
                SUMeshHelperRelease(&face_mesh);
                // std::cout << "triangleLen=" << triangleLen << std::endl;

                for (size_t i = 0; i < triangleLen; i++)
                {
                    double normal[3]         = {normalArr[i].x, normalArr[i].y, normalArr[i].z};
                    double normalTrans[3]    = {0};
                    double vertexTrans[3][3] = {0};
                    double uvTrans[3][3]     = {0};
                    for (int ii = 0; ii < 3; ii++)
                        for (int jj = 0; jj < 3; jj++)
                            normalTrans[ii] += normal[jj] * transformation.values[pos(ii, jj)];
                    for (size_t j = 0; j < 3; j++)
                    {
                        double vertex[3] = {vertexArr[indexArr[i * 3 + j]].x, vertexArr[indexArr[i * 3 + j]].y, vertexArr[indexArr[i * 3 + j]].z};
                        for (int ii = 0; ii < 3; ii++)
                            for (int jj = 0; jj < 3; jj++)
                                vertexTrans[j][ii] += vertex[jj] * transformation.values[pos(ii, jj)];
                        for (int ii = 0; ii < 3; ii++)
                            vertexTrans[j][ii] += transformation.values[pos(ii, 3)];

                        double uv[3] = {texture_coords[indexArr[i * 3 + j]].x, texture_coords[indexArr[i * 3 + j]].y,
                                        texture_coords[indexArr[i * 3 + j]].z};
                        for (int ii = 0; ii < 3; ii++)
                            for (int jj = 0; jj < 3; jj++)
                                uvTrans[j][ii] += uv[jj] * transformation.values[pos(ii, jj)];
                        for (int ii = 0; ii < 3; ii++)
                            uvTrans[j][ii] += transformation.values[pos(ii, 3)];
                    }

                    double x0 = vertexTrans[1][0] - vertexTrans[0][0];
                    double y0 = vertexTrans[1][1] - vertexTrans[0][1];
                    double z0 = vertexTrans[1][2] - vertexTrans[0][2];

                    double x1 = vertexTrans[2][0] - vertexTrans[0][0];
                    double y1 = vertexTrans[2][1] - vertexTrans[0][1];
                    double z1 = vertexTrans[2][2] - vertexTrans[0][2];

                    double xi = y0 * z1 - z0 * y1;
                    double yi = z0 * x1 - x0 * z1;
                    double zi = x0 * y1 - y0 * x1;

                    double docProduct = xi * normalTrans[0] + yi * normalTrans[1] + zi * normalTrans[2];

                    if (docProduct < 0)
                    {
                        for (int ii = 0; ii < 3; ii++)
                        {
                            std::swap(vertexTrans[1][ii], vertexTrans[2][ii]);
                        }
                    }
                    cFacet aFacet;
                    aFacet.normal.x = normalTrans[0];
                    aFacet.normal.y = normalTrans[1];
                    aFacet.normal.z = normalTrans[2];
                    aFacet.color[0] = (double)color.red / 255.0;
                    aFacet.color[1] = (double)color.green / 255.0;
                    aFacet.color[2] = (double)color.blue / 255.0;
                    aFacet.color[3] = alpha;
                    aFacet.imageUri = imageUri;

                    aFacet.uv[0] = Vector3(uvTrans[0][0], uvTrans[0][1], 0);
                    aFacet.uv[1] = Vector3(uvTrans[1][0], uvTrans[1][1], 0);
                    aFacet.uv[2] = Vector3(uvTrans[2][0], uvTrans[2][1], 0);

                    for (int jj = 0; jj < 3; jj++)
                    {
                        aFacet.vertex[jj].x = vertexTrans[jj][0];
                        aFacet.vertex[jj].y = vertexTrans[jj][1];
                        aFacet.vertex[jj].z = vertexTrans[jj][2];
                    }
                    facet_arr_.push_back(aFacet);
                    Color color1((double)color.red / 255.0, (double)color.green / 255.0, (double)color.blue / 255.0, alpha, imageUri);
                    facetMap[color1].emplace_back(aFacet);

                    face_count_++;
                }
                SUMeshHelperRelease(&face_mesh);
            }
        }
    }
    int traversalEntities(SUEntitiesRef entities, int recursive_depth, SUTransformation transformation)
    {
        std::cout << "traversalEntities" << std::endl;
        addFaces(entities, transformation, recursive_depth);
        traversalGroupEntity(entities, recursive_depth, transformation);
        getComponentEntity(entities, recursive_depth, transformation);
        return 0;
    }
};

std::unique_ptr<GltfExporter> GltfExporter::create()
{
    return std::make_unique<GltfExporterImpl>();
}

}  // namespace skp2gltf
