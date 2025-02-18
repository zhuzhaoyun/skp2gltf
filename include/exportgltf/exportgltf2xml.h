/*
 * @Author: yaol 
 * @Date: 2025-02-18 17:31:06 
 * @Last Modified by:   yaol 
 * @Last Modified time: 2025-02-18 17:31:06 
 */

 
#include <string>
#include <vector>
#include <unordered_map>
#include "pugixml/pugixml.hpp"
#include "creategltfcommon.h"
struct Texture
{
    std::string path = "";
    double scaleS    = 1.0;
    double scaleT    = 1.0;
};
struct MaterialXml
{
    std::string name = "";
    double alpha     = 0.0;
    Texture texture;
};
struct ComponentInstance
{
    std::string componentDefinitionName = "";
    std::string layerName               = "";
    std::vector<double> transformationVec;
};
struct Vertex
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double u = 0.0;
    double v = 0.0;
};
struct Face
{
    std::string frontMaterialName = "";
    std::string layerName         = "";
    bool hasTexture               = false;
    std::vector<Vertex> vertexs;
};
struct Group
{
    std::string guid = "";
    std::vector<Face> faces;
};
struct Geometry
{
    std::vector<Group> groups;
    ComponentInstance componentInstance;
};
class Exportgltf2xml
{
  public:
    Exportgltf2xml(std::string);
    
    void exportGltf(const std::string);

  private:
    void parseMaterial(pugi::xml_node);

    void parseGeometry(pugi::xml_node);

  public:
    std::unordered_map<std::string, MaterialXml> materialMap;
    Geometry geometry;

  private:
    std::string xmlName;
};