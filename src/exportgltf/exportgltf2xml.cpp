/*
 * @Author: yaol 
 * @Date: 2025-02-18 17:30:37 
 * @Last Modified by:   yaol 
 * @Last Modified time: 2025-02-18 17:30:37 
 */

 #include "exportgltf2xml.h"
#include <iostream>
using namespace std;
Exportgltf2xml::Exportgltf2xml(std::string xmlPath)
{
    pugi::xml_document doc;
    if (!doc.load_file(xmlPath.c_str(), pugi::parse_default, pugi::encoding_utf8))
    {
        throw std::runtime_error("failed read xmlfile");
    }
    pugi::xml_node materials = doc.child("Materials");
    parseMaterial(materials);
    pugi::xml_node geometry = doc.child("Geometry");
    parseGeometry(geometry);
}
void Exportgltf2xml::exportGltf(const std::string gltfName)
{
    gltf::CreateGltfCommon createGltf;
    std::vector<gltf::Node> nodeVec;
    std::vector<gltf::MeshData> meshDataVec;

    int meshIndex   = 0;
    int vertexIndex = 0;
    for (auto &item : geometry.groups)
    {
        gltf::Node node;
        node.meshIndex = meshIndex++;
        nodeVec.emplace_back(node);
        gltf::MeshData meshData;
        gltf::Primitive pri;
        for (auto &item_face : item.faces)
        {
            if (item_face.vertexs.size() % 3 != 0)
                continue;
            for (int i = 0; i < 3; ++i)
            {
                pri.position.emplace_back(item_face.vertexs[i].x);
                pri.position.emplace_back(item_face.vertexs[i].y);
                pri.position.emplace_back(item_face.vertexs[i].z);
                pri.normal.emplace_back(0);
                pri.normal.emplace_back(0);
                pri.normal.emplace_back(1);
                pri.indec.emplace_back(vertexIndex++);
            }
        }
        meshData.primitives.emplace_back(pri);
        meshDataVec.emplace_back(meshData);
    }
    createGltf.setMeshData(nodeVec, meshDataVec);
    createGltf.createGltf("gltf", gltfName);
}
void Exportgltf2xml::parseGeometry(pugi::xml_node geometry_node)
{
    ComponentInstance componentInstance;
    pugi::xml_node componentInstance_node     = geometry_node.child("ComponentInstance");
    pugi::xml_node transformation_node        = componentInstance_node.child("Transformation");
    componentInstance.componentDefinitionName = componentInstance_node.attribute("Name").as_string();
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m00").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m10").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m20").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m30").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m01").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m11").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m21").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m31").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m02").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m12").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m22").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m32").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m03").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m13").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m23").as_double());
    componentInstance.transformationVec.emplace_back(transformation_node.attribute("m33").as_double());
    geometry.componentInstance = componentInstance;
    std::vector<Group> groups;
    for (pugi::xml_node group_node = geometry_node.child("Group"); group_node; group_node = group_node.next_sibling("Group"))
    {
        Group group;
        for (pugi::xml_node face_node = group_node.child("Face"); face_node; face_node = face_node.next_sibling("Face"))
        {
            Face face;
            pugi::xml_node frontMaterial_node = face_node.child("FrontMaterial");
            face.hasTexture                   = frontMaterial_node.attribute("HasTexture").as_int() == 1 ? true : false;
            face.frontMaterialName            = frontMaterial_node.attribute("Name").as_string();
            pugi::xml_node loop_node          = face_node.child("Loop");
            for (pugi::xml_node vertex_node = loop_node.child("Vertex"); vertex_node; vertex_node = vertex_node.next_sibling("Vertex"))
            {
                Vertex vertex;
                pugi::xml_node point_node = vertex_node.child("Point");
                vertex.x                  = point_node.attribute("x").as_double();
                vertex.x                  = point_node.attribute("y").as_double();
                vertex.x                  = point_node.attribute("z").as_double();
                if (face.hasTexture)
                {
                    pugi::xml_node frontTextureCoords_node = vertex_node.child("FrontTextureCoords");
                    vertex.u                               = frontTextureCoords_node.attribute("u").as_double();
                    vertex.v                               = frontTextureCoords_node.attribute("v").as_double();
                }
                face.vertexs.emplace_back(vertex);
            }
            group.faces.emplace_back(face);
        }
        groups.emplace_back(group);
    }
    geometry.groups = groups;
}
void Exportgltf2xml::parseMaterial(pugi::xml_node materials)
{
    for (pugi::xml_node material_node = materials.child("Material"); material_node; material_node = material_node.next_sibling("Material"))
    {
        MaterialXml materialXml;
        pugi::xml_attribute nameAtt = material_node.attribute("Name");
        if (!nameAtt.empty())
        {
            materialXml.name = nameAtt.as_string();
        }
        pugi::xml_attribute alphaAtt = material_node.attribute("Alpha");
        if (!alphaAtt.empty())
        {
            materialXml.alpha = alphaAtt.as_double();
        }
        pugi::xml_node textureNode = material_node.child("Texture");
        if (!textureNode.empty())
        {
            Texture texture;
            pugi::xml_attribute pathAtt = textureNode.attribute("Path");
            if (!pathAtt.empty())
            {
                texture.path = pathAtt.as_string();
            }
            pugi::xml_attribute scalesAtt = textureNode.attribute("Scale_s");
            if (!scalesAtt.empty())
            {
                texture.scaleS = scalesAtt.as_double();
            }
            pugi::xml_attribute scaletAtt = textureNode.attribute("Scale_t");
            if (!scaletAtt.empty())
            {
                texture.scaleT = scaletAtt.as_double();
            }
            materialXml.texture = texture;
        }
    }
}