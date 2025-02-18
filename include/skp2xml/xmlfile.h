// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#ifndef SKPTOXML_COMMON_XMLFILE_H
#define SKPTOXML_COMMON_XMLFILE_H

#include <string>
#include <vector>
#include <map>

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/transformation.h>

#include "./xmlgeomutils.h"

// Forward declarations
namespace tinyxml2
{
class XMLDocument;
class XMLNode;
class XMLElement;
}  // namespace tinyxml2

// Helper data transfer types storing model information.

struct XmlMaterialInfo
{
    XmlMaterialInfo() : has_color_(false), has_alpha_(false), alpha_(0.0), has_texture_(false), texture_sscale_(0.0), texture_tscale_(0.0) {}

    std::string name_;
    bool has_color_;
    SUColor color_;
    bool has_alpha_;
    double alpha_;
    bool has_texture_;
    std::string texture_path_;
    std::string picture_name_;
    double texture_sscale_;
    double texture_tscale_;
};

struct XmlLayerInfo
{
    XmlLayerInfo() : has_material_info_(false), is_visible_(false) {}

    std::string name_;
    bool has_material_info_;
    XmlMaterialInfo material_info_;
    bool is_visible_;
};

struct XmlEdgeInfo
{
    XmlEdgeInfo() : has_layer_(false), has_color_(false) {}

    bool has_layer_;
    std::string layer_name_;
    bool has_color_;
    SUColor color_;
    XmlGeomUtils::CPoint3d start_;
    XmlGeomUtils::CPoint3d end_;
};

struct XmlCurveInfo
{
    std::vector<XmlEdgeInfo> edges_;
};

struct XmlFaceVertex
{
    XmlGeomUtils::CPoint3d vertex_;
    XmlGeomUtils::CPoint3d normal_;
    XmlGeomUtils::CPoint3d front_texture_coord_;
    XmlGeomUtils::CPoint3d back_texture_coord_;
};

struct XmlFaceInfo
{
    XmlFaceInfo() : has_front_texture_(false), has_back_texture_(false), has_single_loop_(false) {}

    std::string layer_name_;
    std::string front_mat_name_;
    std::string back_mat_name_;
    bool has_front_texture_;
    bool has_back_texture_;
    bool has_single_loop_;
    // if single loop, vertices_ are the points in the loop
    // if triangles, vertices_ are 3 per triangle
    std::vector<XmlFaceVertex> vertices_;
};

struct XmlEntitiesInfo;
struct XmlComponentDefinitionInfo;

struct XmlGroupInfo
{
    XmlGroupInfo();
    XmlGroupInfo(const XmlGroupInfo &);
    ~XmlGroupInfo();
    const XmlGroupInfo &operator=(const XmlGroupInfo &);

    XmlEntitiesInfo *entities_;
    SUTransformation transform_;
};

struct XmlComponentInstanceInfo
{
    std::string definition_name_;
    std::string layer_name_;
    std::string material_name_;
    SUTransformation transform_;
};

struct XmlEntitiesInfo
{
    std::vector<XmlComponentInstanceInfo> component_instances_;
    std::vector<XmlGroupInfo> groups_;
    std::vector<XmlFaceInfo> faces_;
    std::vector<XmlEdgeInfo> edges_;
    std::vector<XmlCurveInfo> curves_;
};

struct XmlComponentDefinitionInfo
{
    std::string name_;
    XmlEntitiesInfo entities_;
};

struct XmlModelInfo
{
    std::vector<XmlLayerInfo> layers_;
    std::vector<XmlMaterialInfo> materials_;
    std::vector<XmlComponentDefinitionInfo> definitions_;
    XmlEntitiesInfo entities_;
};

class CXmlFile
{
  public:
    CXmlFile();
    ~CXmlFile();

    bool Open(const std::string &filename, bool create_new_file);
    void Close(bool cancelled);

    std::string GetTextureDirectory() const;

    // Converts the XML DOM into XmlModelInfo
    bool GetModelInfo(XmlModelInfo &model_info) const;

    // XML modification functions
    void StartLayers();
    void StartGeometry();
    void StartGroup();
    void StartMaterials();
    void StartComponentDefinitions();
    void StartComponentDefinition(const std::string &name);
    void PopParentNode();

    void WriteHeader(int major_ver, int minor_ver, int build_no);
    void WriteLayerInfo(const XmlLayerInfo &info);
    void WriteMaterialInfo(const XmlMaterialInfo &info);
    void WriteEdgeInfo(const XmlEdgeInfo &info);
    void WriteFaceInfo(const XmlFaceInfo &info);
    void WriteCurveInfo(const XmlCurveInfo &info);
    void WriteComponentInstanceInfo(const XmlComponentInstanceInfo &info);
    void WriteTransformation(const SUTransformation &transform);

  private:
    tinyxml2::XMLElement *WriteStartTag(const char *tag);
    void WriteColor(const SUColor &color);

    bool ReadHeader();
    bool ReadColor(const tinyxml2::XMLNode *parent_node, const SUColor &color) const;

    bool ReadLayers(const tinyxml2::XMLNode *parent_node, std::vector<XmlLayerInfo> &layer_infos) const;
    bool ReadLayerInfo(const tinyxml2::XMLNode *parent_node, XmlLayerInfo &info) const;
    bool ReadMaterialInfo(const tinyxml2::XMLNode *parent_node, XmlMaterialInfo &info) const;
    bool ReadMaterials(const tinyxml2::XMLNode *parent_node, std::vector<XmlMaterialInfo> &mat_infos) const;
    bool ReadComponentDefinitionInfo(const tinyxml2::XMLNode *parent_node, bool readEntities, XmlComponentDefinitionInfo &info) const;
    bool ReadComponentDefinitions(const tinyxml2::XMLNode *parent_node, std::vector<XmlComponentDefinitionInfo> &def_infos) const;
    bool ReadEntities(const tinyxml2::XMLNode *parent_node, XmlEntitiesInfo &entities) const;
    bool ReadEdgeInfo(const tinyxml2::XMLNode *parent_node, XmlEdgeInfo &info) const;
    bool ReadFaceInfo(const tinyxml2::XMLNode *parent_node, XmlFaceInfo &info) const;
    bool ReadCurveInfo(const tinyxml2::XMLNode *parent_node, XmlCurveInfo &info) const;
    bool ReadTransformation(const tinyxml2::XMLNode *parent_node, SUTransformation &transform) const;
    bool ReadComponentInstanceInfo(const tinyxml2::XMLNode *parent_node, XmlComponentInstanceInfo &info) const;

  private:
    // Let TinyXML do the xml handling
    tinyxml2::XMLDocument *xml_doc_;
    tinyxml2::XMLNode *parent_node_;

    // The path to the file to which we are writing
    std::string filename_;
    bool create_new_file_;
};

#endif  // SKPTOXML_COMMON_XMLFILE_H
