// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#include <vector>
#include <sstream>

#include "./xmlfile.h"
#include "./tinyxml2.h"

// XML tags
static const std::string kSkpToXMLTag("SkpToXML");
static const std::string kXMLVersionTag("xmlversion");
static const std::string kSkpVersionTag("skpversion");
static const std::string kLayersTag("Layers");
static const std::string kLayerTag("Layer");
static const std::string kCompDefsTag("ComponentDefinitions");
static const std::string kCompDefTag("ComponentDefinition");
static const std::string kTransformTag("Transformation");
static const std::string kMaterialsTag("Materials");
static const std::string kMaterialTag("Material");
static const std::string kGeometryTag("Geometry");
static const std::string kComponentInstanceTag("ComponentInstance");
static const std::string kCurveTag("Curve");
static const std::string kGroupTag("Group");
static const std::string kNameTag("Name");
static const std::string kVisibleTag("Visible");
static const std::string kAlphaTag("Alpha");
static const std::string kPathTag("Path");
static const std::string kSScaleTag("Scale_s");
static const std::string kTScaleTag("Scale_t");
static const std::string kTextureTag("Texture");
static const std::string kColorTag("Color");
static const std::string kColorFormat("#%02x%02x%02x");
static const std::string kCountTag("Count");
static const std::string kFaceTag("Face");
static const std::string kEdgeTag("Edge");
static const std::string kFrontMaterialTag("FrontMaterial");
static const std::string kBackMaterialTag("BackMaterial");
static const std::string kHasTextureTag("HasTexture");
static const std::string kTrianglesTag("Triangles");
static const std::string kPointTag("Point");
static const std::string kFrontTextureCoordsTag("FrontTextureCoords");
static const std::string kBackTextureCoordsTag("BackTextureCoords");
static const std::string kLoopTag("Loop");
static const std::string kVertexTag("Vertex");
static const std::string kXTag("x");
static const std::string kYTag("y");
static const std::string kZTag("z");
static const std::string kUTag("u");
static const std::string kVTag("v");
static const std::string kStartTag("Start");
static const std::string kEndTag("End");

using namespace XmlGeomUtils;

//------------------------------------------------------------------------------

XmlGroupInfo::XmlGroupInfo() {
  entities_ = new XmlEntitiesInfo;
}

XmlGroupInfo::XmlGroupInfo(const XmlGroupInfo& info) {
  entities_ = new XmlEntitiesInfo(*info.entities_);
  transform_ = info.transform_;
}

XmlGroupInfo::~XmlGroupInfo() {
  delete entities_;
}

const XmlGroupInfo& XmlGroupInfo::operator = (const XmlGroupInfo& info) {
  *entities_ = *info.entities_;
  transform_ = info.transform_;
  return *this;
}

//------------------------------------------------------------------------------

CXmlFile::CXmlFile()
  : xml_doc_(NULL),
    create_new_file_(false) {
}

CXmlFile::~CXmlFile() {
  delete xml_doc_;
}

bool CXmlFile::Open(const std::string& filename, bool create_new_file) {
  if (filename.empty())
    return false;

  if (xml_doc_) {
    printf("Warning! opening already open file\n");
    return true;
  }

  filename_ = filename;
  create_new_file_ = create_new_file;

  xml_doc_ = new tinyxml2::XMLDocument;
  parent_node_ = xml_doc_;

  bool ok = true;

  if (!create_new_file) {
    ok = xml_doc_->LoadFile(filename.c_str()) == tinyxml2::XML_NO_ERROR &&
         ReadHeader(); // Check for valid header
  }

  return ok;
}

void CXmlFile::Close(bool cancelled) {
  if (create_new_file_ && !cancelled)
    xml_doc_->SaveFile(filename_.c_str());
  delete xml_doc_;
  xml_doc_ = NULL;
  parent_node_ = NULL;
}

static size_t FindLastSlash(const std::string& filename) {
  size_t index = filename.rfind('/');
  if (index == -1) {
    index = filename.rfind('\\');
  }
  return index;
}

std::string CXmlFile::GetTextureDirectory() const {
  // Extract the directory in which we are writing
  size_t index = FindLastSlash(filename_);
  std::string folder = filename_.substr(0, index+1);
  return folder;
}

bool CXmlFile::ReadHeader() {
  const tinyxml2::XMLNode* node = xml_doc_->FirstChild();
  const tinyxml2::XMLElement* elem = node->ToElement();
  bool ok = false;
  if (elem->Value() == kSkpToXMLTag) {
    int version = 0;
    ok = (elem->QueryIntAttribute(kXMLVersionTag.c_str(), &version) ==
          tinyxml2::XML_NO_ERROR) && (version == 3);
  }
  return ok;
}

void CXmlFile::WriteHeader(int major_ver, int minor_ver, int build_no) {
  tinyxml2::XMLElement* elem = xml_doc_->NewElement(kSkpToXMLTag.c_str());
  tinyxml2::XMLNode* node = xml_doc_->InsertFirstChild(elem);
  elem->SetAttribute(kXMLVersionTag.c_str(), 3);

  // Combine the version
  std::stringstream ss;
  ss << major_ver << '.' << minor_ver << '.' << build_no;
  elem->SetAttribute(kSkpVersionTag.c_str(), ss.str().c_str());
  elem->SetAttribute("units", "inches");
}

tinyxml2::XMLElement* CXmlFile::WriteStartTag(const char* tag) {
  tinyxml2::XMLElement* elem = xml_doc_->NewElement(tag);
  parent_node_ = parent_node_->InsertEndChild(elem);
  return elem;
}

void CXmlFile::StartLayers() {
  WriteStartTag(kLayersTag.c_str());
}

void CXmlFile::StartGeometry() {
  WriteStartTag(kGeometryTag.c_str());
}

void CXmlFile::StartGroup() {
  WriteStartTag(kGroupTag.c_str());
}

void CXmlFile::StartMaterials() {
  WriteStartTag(kMaterialsTag.c_str());
}

void CXmlFile::StartComponentDefinitions() {
  WriteStartTag(kCompDefsTag.c_str());
}

void CXmlFile::StartComponentDefinition(const std::string& name) {
  tinyxml2::XMLElement* elem = WriteStartTag(kCompDefTag.c_str());
  elem->SetAttribute(kNameTag.c_str(), name.c_str());
}

bool CXmlFile::ReadComponentDefinitionInfo(
    const tinyxml2::XMLNode* parent_node,
    bool readEntities,
    XmlComponentDefinitionInfo& info) const {
  const char* name = parent_node->ToElement()->Attribute(kNameTag.c_str());
  if (name == NULL)
    return false;
  info.name_ = name;
  
  return !readEntities || ReadEntities(parent_node, info.entities_);
}

void CXmlFile::PopParentNode() {
  parent_node_ = parent_node_->Parent();
}

bool CXmlFile::ReadLayerInfo(const tinyxml2::XMLNode* parent_node,
                             XmlLayerInfo& info) const {
  const tinyxml2::XMLElement* elem = parent_node->ToElement();
  if (elem->Value() != kLayerTag)
    return false;

  bool ok = true;

  // Name
  const char* name = elem->Attribute(kNameTag.c_str());
  if (name != NULL) {
    info.name_ = name;
  } else {
    ok = false;
  }

  // Visibility
  info.is_visible_ = elem->BoolAttribute(kVisibleTag.c_str());

  // Material info (optional)
  const tinyxml2::XMLNode* child = parent_node->FirstChild();
  info.has_material_info_ = ReadMaterialInfo(child, info.material_info_);

  return ok;
}

void CXmlFile::WriteLayerInfo(const XmlLayerInfo& info) {
  tinyxml2::XMLElement* elem = WriteStartTag(kLayerTag.c_str());
  elem->SetAttribute(kNameTag.c_str(), info.name_.c_str());
  elem->SetAttribute(kVisibleTag.c_str(), info.is_visible_);

  if (info.has_material_info_) {
    WriteMaterialInfo(info.material_info_);
  }
  PopParentNode();
}

bool CXmlFile::ReadColor(const tinyxml2::XMLNode* parent_node,
                         const SUColor& color) const {
  const char* attrib = parent_node->ToElement()->Attribute(kColorTag.c_str());
  if (attrib != NULL) {
    sscanf(attrib, kColorFormat.c_str(), &color.red, &color.green, &color.blue);
    return true;
  }
  return false;
}

void CXmlFile::WriteColor(const SUColor& color) {
  char buf[10] = { 0 };
  sprintf(buf, kColorFormat.c_str(), color.red, color.green, color.blue);
  parent_node_->ToElement()->SetAttribute(kColorTag.c_str(), buf);
}

bool CXmlFile::ReadMaterialInfo(const tinyxml2::XMLNode* parent_node,
                                XmlMaterialInfo& info) const {
  const tinyxml2::XMLElement* elem = parent_node->ToElement();
  if (elem->Value() != kMaterialTag)
    return false;

  bool ok = true;

  // Name
  const char* name = elem->Attribute(kNameTag.c_str());
  if (name != NULL) {
    info.name_ = name;
  } else {
    ok = false;
  }

  // Color (optional)
  info.has_color_ = ReadColor(parent_node, info.color_);
  
  // Alpha (optional)
  info.has_alpha_ = elem->QueryDoubleAttribute(kAlphaTag.c_str(), &info.alpha_)
                    == tinyxml2::XML_NO_ERROR;

  // Texture (optional)
  const tinyxml2::XMLNode* child = parent_node->FirstChild();
  if (child != NULL) {
    const tinyxml2::XMLElement* child_elem = child->ToElement();
    if (child_elem->Value() == kTextureTag) {
      info.has_texture_ = true;
      const char* str_path = child_elem->Attribute(kPathTag.c_str());
      if (str_path != NULL) {
        info.texture_path_ = str_path;
      } else {
        ok = false;
      }
      ok &= child_elem->QueryDoubleAttribute(kSScaleTag.c_str(),
            &info.texture_sscale_) == tinyxml2::XML_NO_ERROR;
      ok &= child_elem->QueryDoubleAttribute(kTScaleTag.c_str(),
            &info.texture_tscale_) == tinyxml2::XML_NO_ERROR;
    }
  }

  return ok;
}

void CXmlFile::WriteMaterialInfo(const XmlMaterialInfo& info) {
  // The material id, name, color, alpha all go on the same line
  tinyxml2::XMLElement* elem = WriteStartTag(kMaterialTag.c_str());
  elem->SetAttribute(kNameTag.c_str(), info.name_.c_str());

  if (info.has_color_) {
    WriteColor(info.color_);
  }

  if (info.has_alpha_) {
    elem->SetAttribute(kAlphaTag.c_str(), info.alpha_);
  }

  // Material texture
  if (info.has_texture_) {
    tinyxml2::XMLElement* elem = WriteStartTag(kTextureTag.c_str());
    elem->SetAttribute(kPathTag.c_str(), info.texture_path_.c_str());
    elem->SetAttribute(kSScaleTag.c_str(), info.texture_sscale_);
    elem->SetAttribute(kTScaleTag.c_str(), info.texture_tscale_);
    PopParentNode();
  }
  PopParentNode();
}

static bool ReadPoint(const tinyxml2::XMLNode* parent_node,
                      CPoint3d& point) {
  const tinyxml2::XMLElement* elem = parent_node->ToElement();
  double x, y, z;
  if (elem->QueryDoubleAttribute(kXTag.c_str(), &x) ==
      tinyxml2::XML_NO_ERROR &&
      elem->QueryDoubleAttribute(kYTag.c_str(), &y) ==
      tinyxml2::XML_NO_ERROR &&
      elem->QueryDoubleAttribute(kZTag.c_str(), &z) ==
      tinyxml2::XML_NO_ERROR) {
    point.SetLocation(x, y, z);
    return true;
  }
  return false;
}

bool CXmlFile::ReadEdgeInfo(const tinyxml2::XMLNode* parent_node,
                            XmlEdgeInfo& info) const {
  // Layer (optional)
  const tinyxml2::XMLNode* child = parent_node->FirstChild();
  if (child == NULL)
    return false;
  if (child->Value() == kLayerTag) {
    const tinyxml2::XMLElement* elem = child->ToElement();
    const char* layer_name = elem->Attribute(kNameTag.c_str());
    if (layer_name != NULL) {
      info.has_layer_ = true;
      info.layer_name_ = layer_name;
    } else {
      info.has_layer_ = false;
    }
    child = child->NextSibling();
  }

  if (child == NULL)
    return false;

  // Color (optional)
  if (child->Value() == kMaterialTag) {
    info.has_color_ = ReadColor(child, info.color_);
    child = child->NextSibling();
  }

  bool ok = true;

  // End points
  if (child != NULL && child->Value() == kStartTag) {
    ok &= ReadPoint(child, info.start_);

    child = child->NextSibling();
    if (child != NULL && child->Value() == kEndTag) {
      ok &= ReadPoint(child, info.end_);
    } else {
      ok = false;
    }
  } else {
    ok = false;
  }

  return ok;
}

void CXmlFile::WriteEdgeInfo(const XmlEdgeInfo& info) {
  WriteStartTag(kEdgeTag.c_str());

  // Layer (optional)
  if (info.has_layer_) {
    tinyxml2::XMLElement* elem = WriteStartTag(kLayerTag.c_str());
    elem->SetAttribute(kNameTag.c_str(), info.layer_name_.c_str());
    PopParentNode();
  }

  // Color (optional)
  if (info.has_color_) {
    WriteStartTag(kMaterialTag.c_str());
    WriteColor(info.color_);
    PopParentNode();
  }

  // End points
  {
    tinyxml2::XMLElement* elem = WriteStartTag(kStartTag.c_str());
    elem->SetAttribute(kXTag.c_str(), info.start_.x());
    elem->SetAttribute(kYTag.c_str(), info.start_.y());
    elem->SetAttribute(kZTag.c_str(), info.start_.z());
    PopParentNode();
  }
  {
    tinyxml2::XMLElement* elem = WriteStartTag(kEndTag.c_str());
    elem->SetAttribute(kXTag.c_str(), info.end_.x());
    elem->SetAttribute(kYTag.c_str(), info.end_.y());
    elem->SetAttribute(kZTag.c_str(), info.end_.z());
    PopParentNode();
  }

  PopParentNode();
}

bool CXmlFile::ReadFaceInfo(const tinyxml2::XMLNode* parent_node,
                            XmlFaceInfo& info) const {
  // Front material (optional)
  const tinyxml2::XMLNode* child = parent_node->FirstChild();
  if (child->Value() == kFrontMaterialTag) {
    const tinyxml2::XMLElement* elem = child->ToElement();
    const char* mat_name = elem->Attribute(kNameTag.c_str());
    if (mat_name != NULL)
      info.front_mat_name_ = mat_name;
    else
      info.front_mat_name_.clear();
    elem->QueryBoolAttribute(kHasTextureTag.c_str(), &info.has_front_texture_);
    child = child->NextSibling();
  }

  // Back material (optional)
  if (child->Value() == kBackMaterialTag) {
    const tinyxml2::XMLElement* elem = child->ToElement();
    const char* mat_name = elem->Attribute(kNameTag.c_str());
    if (mat_name != NULL)
      info.back_mat_name_ = mat_name;
    else
      info.back_mat_name_.clear();
    elem->QueryBoolAttribute(kHasTextureTag.c_str(), &info.has_back_texture_);
    child = child->NextSibling();
  }

  // Layer (optional)
  if (child->Value() == kLayerTag) {
    const tinyxml2::XMLElement* elem = child->ToElement();
    const char* layer_name = elem->Attribute(kNameTag.c_str());
    if (layer_name != NULL) {
      info.layer_name_ = layer_name;
    }
    child = child->NextSibling();
  }

  // Loop or Triangles
  bool ok = false;
  int triangle_count = 0;
  if (child->Value() == kLoopTag) {
    info.has_single_loop_ = true;
    ok = true;
  } else if (child->Value() == kTrianglesTag) {
    info.has_single_loop_ = false;
    const tinyxml2::XMLElement* elem = child->ToElement();
    ok = elem->QueryIntAttribute(kCountTag.c_str(), &triangle_count) == 
         tinyxml2::XML_NO_ERROR;
  }
  if (ok) {
    const tinyxml2::XMLNode* vertex_node = child->FirstChild();
    while (ok && vertex_node != NULL && vertex_node->Value() == kVertexTag) {
      // Vertex position
      const tinyxml2::XMLNode* pt_node = vertex_node->FirstChild();
      if (pt_node != NULL) {
        const tinyxml2::XMLElement* elem = pt_node->ToElement();
        XmlFaceVertex vertex;
        if (ReadPoint(pt_node, vertex.vertex_)) {
          // Front texture coords
          const tinyxml2::XMLNode* node = pt_node;
          if (info.has_front_texture_) {
            node = node->NextSibling();
            if (node != NULL && node->Value() == kFrontTextureCoordsTag) {
              elem = node->ToElement();
              double u, v;
              if (elem->QueryDoubleAttribute(kUTag.c_str(), &u) ==
                  tinyxml2::XML_NO_ERROR &&
                  elem->QueryDoubleAttribute(kVTag.c_str(), &v) ==
                  tinyxml2::XML_NO_ERROR) {
                vertex.front_texture_coord_.SetLocation(u, v, 0);
              } else {
                ok = false;
              }
            } else {
              ok = false;
            }
          }
          // Back texture coords
          if (info.has_back_texture_) {
            node = node->NextSibling();
            if (node != NULL && node->Value() == kBackTextureCoordsTag) {
              elem = node->ToElement();
              double u, v;
              if (elem->QueryDoubleAttribute(kUTag.c_str(), &u) ==
                  tinyxml2::XML_NO_ERROR &&
                  elem->QueryDoubleAttribute(kVTag.c_str(), &v) ==
                  tinyxml2::XML_NO_ERROR) {
                vertex.back_texture_coord_.SetLocation(u, v, 0);
              } else {
                ok = false;
              }
            } else {
              ok = false;
            }
          }
          
          info.vertices_.push_back(vertex);
        } else {
          ok = false;
        }
      } else {
        ok = false;
      }
      
      vertex_node = vertex_node->NextSibling();
    } // Vertex loop

    // If a mesh is given, check the number of vertices
    if (!info.has_single_loop_) {
      ok &= (info.vertices_.size() == triangle_count * 3);
    }
  } // if (ok)

  return ok;
}

void CXmlFile::WriteFaceInfo(const XmlFaceInfo& info) {
  WriteStartTag(kFaceTag.c_str());

  // Front material (optional)
  if (!info.front_mat_name_.empty()) {
    tinyxml2::XMLElement* elem = WriteStartTag(kFrontMaterialTag.c_str());
    elem->SetAttribute(kNameTag.c_str(), info.front_mat_name_.c_str());
    elem->SetAttribute(kHasTextureTag.c_str(), info.has_front_texture_);
    PopParentNode();
  }

  // Back material (optional)
  if (!info.back_mat_name_.empty()) {
    tinyxml2::XMLElement* elem = WriteStartTag(kBackMaterialTag.c_str());
    elem->SetAttribute(kNameTag.c_str(), info.back_mat_name_.c_str());
    elem->SetAttribute(kHasTextureTag.c_str(), info.has_back_texture_);
    PopParentNode();
  }

  // Layer (optional)
  if (!info.layer_name_.empty()) {
    tinyxml2::XMLElement* elem = WriteStartTag(kLayerTag.c_str());
    elem->SetAttribute(kNameTag.c_str(), info.layer_name_.c_str());
    PopParentNode();
  }

  // Loop or Triangles
  size_t count = info.vertices_.size();
  if (info.has_single_loop_) {
    WriteStartTag(kLoopTag.c_str());
  } else {
    tinyxml2::XMLElement* elem = WriteStartTag(kTrianglesTag.c_str());
    elem->SetAttribute(kCountTag.c_str(), static_cast<unsigned>(count / 3));
  }

  // Vertices
  for (size_t i = 0; i < count; i++) {
    WriteStartTag(kVertexTag.c_str());
    const XmlFaceVertex& vertex_info = info.vertices_[i];
    {
      tinyxml2::XMLElement* elem = WriteStartTag(kPointTag.c_str());
      elem->SetAttribute(kXTag.c_str(), vertex_info.vertex_.x());
      elem->SetAttribute(kYTag.c_str(), vertex_info.vertex_.y());
      elem->SetAttribute(kZTag.c_str(), vertex_info.vertex_.z());
      PopParentNode();
    }

    if (info.has_front_texture_) {
      tinyxml2::XMLElement* elem =
          WriteStartTag(kFrontTextureCoordsTag.c_str());
      elem->SetAttribute(kUTag.c_str(), vertex_info.front_texture_coord_.x());
      elem->SetAttribute(kVTag.c_str(), vertex_info.front_texture_coord_.y());
      PopParentNode();
    }

    if (info.has_back_texture_) {
      tinyxml2::XMLElement* elem = WriteStartTag(kBackTextureCoordsTag.c_str());
      elem->SetAttribute(kUTag.c_str(), vertex_info.back_texture_coord_.x());
      elem->SetAttribute(kVTag.c_str(), vertex_info.back_texture_coord_.y());
      PopParentNode();
    }
    PopParentNode();
  }

  PopParentNode(); // Loop or Triangles
  PopParentNode(); // Face
}

bool CXmlFile::ReadCurveInfo(const tinyxml2::XMLNode* parent_node,
                             XmlCurveInfo& info) const {
  bool ok = true;
  const tinyxml2::XMLNode* child = parent_node->FirstChild();
  while (child != NULL) {
    XmlEdgeInfo edge_info;
    if (ReadEdgeInfo(child, edge_info)) {
      info.edges_.push_back(edge_info);
    } else {
      ok = false;
    }
    child = child->NextSibling();
  }
  return ok;
}

void CXmlFile::WriteCurveInfo(const XmlCurveInfo& info) {
  WriteStartTag(kCurveTag.c_str());
  
  for (std::vector<XmlEdgeInfo>::const_iterator it = info.edges_.begin();
       it != info.edges_.end(); ++it) {
    WriteEdgeInfo(*it);
  }

  PopParentNode();
}

static std::string MakeMatrixAttribName(int row, int col) {
  std::stringstream ss;
  ss << 'm' << row << col;
  return ss.str();
}

bool CXmlFile::ReadTransformation(const tinyxml2::XMLNode* parent_node,
                                  SUTransformation& transform) const {
  const tinyxml2::XMLElement* elem = parent_node->LastChildElement();
  if (elem == NULL || elem->Value() != kTransformTag)
    return false;

  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      std::string tag = MakeMatrixAttribName(row, col);
      transform.values[col * 4 + row] = elem->DoubleAttribute(tag.c_str());
    }
  }

  return true;
}

void CXmlFile::WriteTransformation(const SUTransformation& transform) {
  tinyxml2::XMLElement* elem = WriteStartTag(kTransformTag.c_str());
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      std::string tag = MakeMatrixAttribName(row, col);
      elem->SetAttribute(tag.c_str(), transform.values[col * 4 + row]);
    }
  }
  PopParentNode();
}

bool CXmlFile::GetModelInfo(XmlModelInfo& model_info) const {
  // Clear out the given model info
  model_info = XmlModelInfo();

  bool ok = true;

  // Loop through top level tags of the file
  tinyxml2::XMLNode* child = xml_doc_->FirstChild();
  while (child != NULL) {
    const char* tag = child->ToElement()->Value();
    if (tag == kLayersTag) {
      ok &= ReadLayers(child, model_info.layers_);
    } else if (tag == kMaterialsTag) {
      ok &= ReadMaterials(child, model_info.materials_);
    } else if (tag == kCompDefsTag) {
      ok &= ReadComponentDefinitions(child, model_info.definitions_);
    } else if (tag == kGeometryTag) {
      ok &= ReadEntities(child, model_info.entities_);
    }
    child = child->NextSibling();
  }

  return ok;
}

bool CXmlFile::ReadLayers(const tinyxml2::XMLNode* parent_node,
                          std::vector<XmlLayerInfo>& layer_infos) const {
  bool ok = true;
  const tinyxml2::XMLNode* child = parent_node->FirstChild();
  while (child != NULL) {
    XmlLayerInfo info;
    if (ReadLayerInfo(child, info)) {
      layer_infos.push_back(info);
    } else {
      ok = false;
    }
    child = child->NextSibling();
  }
  return ok;
}

bool CXmlFile::ReadMaterials(const tinyxml2::XMLNode* parent_node,
                             std::vector<XmlMaterialInfo>& mat_infos) const {
  bool ok = true;
  const tinyxml2::XMLNode* child = parent_node->FirstChild();
  while (child != NULL) {
    XmlMaterialInfo info;
    if (ReadMaterialInfo(child, info)) {
      mat_infos.push_back(info);
    } else {
      ok = false;
    }
    child = child->NextSibling();
  }
  return ok;
}

bool CXmlFile::ReadComponentDefinitions(const tinyxml2::XMLNode* parent_node,
    std::vector<XmlComponentDefinitionInfo>& def_infos) const {
  bool ok = true;
  const tinyxml2::XMLNode* child = parent_node->FirstChild();
  while (child != NULL) {
    XmlComponentDefinitionInfo info;
    if (ReadComponentDefinitionInfo(child, true, info)) {
      def_infos.push_back(info);
    } else {
      ok = false;
    }
    child = child->NextSibling();
  }

  return ok;
}

void CXmlFile::WriteComponentInstanceInfo(
    const XmlComponentInstanceInfo& info) {
  tinyxml2::XMLElement* elem = WriteStartTag(kComponentInstanceTag.c_str());
  
  // Definition name
  StartComponentDefinition(info.definition_name_);
  PopParentNode();

  // Material (optional)
  if (!info.material_name_.empty()) {
    tinyxml2::XMLElement* elem = WriteStartTag(kMaterialTag.c_str());
    elem->SetAttribute(kNameTag.c_str(), info.material_name_.c_str());
    PopParentNode();
  }

  // Layer (optional)
  if (!info.layer_name_.empty()) {
    tinyxml2::XMLElement* elem = WriteStartTag(kLayerTag.c_str());
    elem->SetAttribute(kNameTag.c_str(), info.layer_name_.c_str());
    PopParentNode();
  }

  // Transformation
  WriteTransformation(info.transform_);
  PopParentNode();
}

bool CXmlFile::ReadComponentInstanceInfo(const tinyxml2::XMLNode* parent_node,
                                         XmlComponentInstanceInfo& info) const {
  bool ok = true;

  // Definition name
  XmlComponentDefinitionInfo comp_def;
  const tinyxml2::XMLNode* child = parent_node->FirstChild();
  ok &= ReadComponentDefinitionInfo(child, false, comp_def);
  info.definition_name_ = comp_def.name_;

  // Material (optional)
  child = child->NextSibling();
  if (child != NULL && child->Value() == kMaterialTag) {
    info.material_name_ = child->ToElement()->Attribute(kNameTag.c_str());
  }

  if (child != NULL) {
    bool foundLayer = false;
    if (child->Value() == kLayerTag) {
      foundLayer = true;
    } else {
      child = child->NextSibling();
      if (child != NULL && child->Value() == kLayerTag) {
        foundLayer = true;
      }
    }
    if (foundLayer) {
      info.layer_name_ = child->ToElement()->Attribute(kNameTag.c_str());
    }
  }

  // Transformation
  ok &= ReadTransformation(parent_node, info.transform_);
  return ok;
}

bool CXmlFile::ReadEntities(const tinyxml2::XMLNode* parent_node,
                            XmlEntitiesInfo& entities) const {

  bool ok = true;

  const tinyxml2::XMLNode* child = parent_node->FirstChild();
  while (child != NULL) {
    const char* tag = child->ToElement()->Value();
    if (tag == kComponentInstanceTag) {
      XmlComponentInstanceInfo instance;
      ReadComponentInstanceInfo(child, instance);
      entities.component_instances_.push_back(instance);
    } else if (tag == kGroupTag) {
      XmlGroupInfo group;
      // Recurse into group entities
      ok &= ReadEntities(child, *group.entities_);
      // Read the transformation
      ok &= ReadTransformation(child, group.transform_);
      entities.groups_.push_back(group);
    } else if (tag == kFaceTag) {
      // Read faces
      XmlFaceInfo face_info;
      ok &= ReadFaceInfo(child, face_info);
      entities.faces_.push_back(face_info);
    } else if (tag == kEdgeTag) {
      // Read edges
      XmlEdgeInfo edge_info;
      ok &= ReadEdgeInfo(child, edge_info);
      entities.edges_.push_back(edge_info);
    } else if (tag == kCurveTag) {
      // Read curves
      XmlCurveInfo curve_info;
      ok &= ReadCurveInfo(child, curve_info);
      entities.curves_.push_back(curve_info);
    }
    child = child->NextSibling();
  }

  return ok;
}
