/*
 * @Description: 
 * @Author: yaol
 * @Date: 2025-01-02 11:26:07
 * @LastEditTime: 2021-08-04 09:32:54
 * @LastEditors: yaol
 * @FilePath: \skp2gltf\gltflib\tinygltf\gltfstruct.hpp
 */
#ifndef GLTFSTRUCT_H
#define GLTFSTRUCT_H
#include <float.h>
#include <stdio.h>
// #include <unistd.h>
// #include <uuid/uuid.h>
#include <algorithm>
#include <chrono>
// #include <eigen3/Eigen/Dense>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
// #include "gltflib/tinygltf/sole.hpp"
#include "tiny_gltf.h"
typedef unsigned char uchar;
namespace gltf
{
struct Primitive
{
    int materialIndex;             
    tinygltf::Material material;   
    std::string materialName;     
    std::vector<double> position; 
    std::vector<double> normal;   
    std::vector<double> uv;       
    std::vector<double> color;    
    std::vector<size_t> indec;     
    std::string imageUri;          
    int mode = 4;
    void clear()
    {
        indec.clear();
        position.clear();
        normal.clear();
        uv.clear();
        color.clear();
    }
};
struct MeshData
{
    std::string name;                  
    std::vector<Primitive> primitives;  
    void clear() { primitives.clear(); }
};
struct Node
{
    Node() = default;
    Node(std::string _guid, std::vector<double> _matrix) : guid(_guid), matrix(_matrix) {}
    int meshIndex;
    std::string guid;
    std::string uuid;
    std::vector<double> matrix;
    void clear()
    {
        meshIndex = -1;
        guid      = "";
        matrix.clear();
    }
};
struct GeoData
{
    std::vector<double> position;
    std::vector<double> normal;
    std::vector<size_t> indec;
    std::vector<Node> mapping;
};
}  // namespace gltf
#endif  // GLTFSTRUCT_H