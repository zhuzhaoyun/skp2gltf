/*
 * @Description:
 * @Author: yaol
 * @Date: 2021-08-06 10:22:27
 * @LastEditTime: 2021-08-06 15:02:30
 * @LastEditors: yaol
 * @FilePath: \common\xmlexporter.h
 */
// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#ifndef SKPTOXML_COMMON_XMLEXPORTER_H
#define SKPTOXML_COMMON_XMLEXPORTER_H

#include "xmlinheritancemanager.h"
#include "xmloptions.h"
#include "xmlstats.h"
#include "xmlfile.h"

#include <SketchUpAPI/import_export/pluginprogresscallback.h>
#include <SketchUpAPI/model/defs.h>
#include "gltflib/creategltfcommon.h"
class Color
{
  public:
    Color(double _r, double _g, double _b, double _a, std::string _imageUri, std::string _name)
        : r(_r), g(_g), b(_b), a(_a), imageUri(_imageUri), name(_name)
    {}
    Color() {}
    double r;
    double g;
    double b;
    double a;
    std::string name;
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
    inline bool empty() { return x == -1 && y == -1 && z == -1; }
    double x = -1, y = -1, z = -1;
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
class CXmlExporter
{
  public:
    CXmlExporter();
    virtual ~CXmlExporter();

    // Convert
    bool Convert(const std::string &from_file, const std::string &file_name, const std::string &to_file, SketchUpPluginProgressCallback *callback);

    // Set user options
    void SetOptions(const CXmlOptions &options) { options_ = options; }

    // Get stats
    const CXmlExportStats &stats() const { return stats_; }

    int exportToGltfImpl(const std::string &gltfName);
    void addFace(SUEntitiesRef entities, const SUTransformation &transformation);
    void getComponentEntity(SUEntitiesRef entities, const SUTransformation &transformation);

  private:
    // Clean up slapi objects
    void ReleaseModelObjects();
    // Write texture files to the destination directory
    void WriteTextureFiles();

    void WriteLayers();
    void WriteLayer(SULayerRef layer);

    void WriteMaterials();
    void WriteMaterial(SUMaterialRef material);

    void WriteComponentDefinitions();
    void WriteComponentDefinition(SUComponentDefinitionRef comp_def);

    void WriteGeometry();
    void WriteEntities(SUEntitiesRef entities, SUTransformation &transformation);
    void traversalGroupEntity(SUEntitiesRef entities, const SUTransformation &transformation);
    void WriteFace(SUFaceRef face, const SUTransformation &transformation);
    void WriteEdge(SUEdgeRef edge);
    void WriteCurve(SUCurveRef curve);
    void addFaces(XmlFaceInfo &info, const SUTransformation &transformation);

    XmlEdgeInfo GetEdgeInfo(SUEdgeRef edge) const;

    // 添加新的辅助方法用于分批处理
    void ProcessGeometryBatch(SUEntitiesRef entities, const SUTransformation& transformation, size_t batchSize);
    void FlushGeometryBatch();
    
    // 用于批处理的缓存
    std::vector<SUFaceRef> faceBuffer;
    static const size_t DEFAULT_BATCH_SIZE = 1000; // 可以根据实际情况调整

    // 用于顶点去重的结构
    struct VertexKey {
        double x, y, z;
        double u, v;  // UV coordinates
        
        bool operator<(const VertexKey& other) const {
            const double EPSILON = 1e-7;
            if (std::abs(x - other.x) > EPSILON) return x < other.x;
            if (std::abs(y - other.y) > EPSILON) return y < other.y;
            if (std::abs(z - other.z) > EPSILON) return z < other.z;
            if (std::abs(u - other.u) > EPSILON) return u < other.u;
            return v < other.v;
        }
    };

    // 顶点缓存
    std::map<VertexKey, size_t> vertexCache;
    std::vector<VertexKey> uniqueVertices;
    
    // 用于存储压缩后的纹理信息
    struct CompressedTexture {
        std::string originalPath;
        std::string compressedPath;
        int width;
        int height;
        bool isCompressed;
    };
    
    std::map<std::string, CompressedTexture> textureCache;
    
    // 新增的辅助方法
    void CompressAndResizeTextures();
    std::string ProcessTexture(const std::string& texturePath);
    size_t GetOrCreateVertexIndex(const VertexKey& key);
    void ClearVertexCache();

  private:
    CXmlOptions options_;

    // Export statistics. Filled in by this exporters class and used later by
    // the platform specific plugin classes to populate the results dialog.
    CXmlExportStats stats_;

    // SLAPI model and texture writer
    SUModelRef model_;
    SUTextureWriterRef texture_writer_;

    // Stack
    CInheritanceManager inheritance_manager_;

    //    std::unordered_map<Color, std::vector<cFacet>, colorHashFuc> facetMap;

    // File & stats
    CXmlFile file_;
    std::unordered_map<std::string, XmlMaterialInfo> materialMap;
    std::unordered_map<Color, std::vector<cFacet>, colorHashFuc> facetMap;
    std::string outPath;
    double ratio = 1;
};

#endif  // SKPTOXML_COMMON_XMLEXPORTER_H
