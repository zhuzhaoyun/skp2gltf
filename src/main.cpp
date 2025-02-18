/*
 * @Author: yaol 
 * @Date: 2025-02-18 17:26:05 
 * @Last Modified by:   yaol 
 * @Last Modified time: 2025-02-18 17:26:05 
 */

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <SketchUpAPI/sketchup.h>

#include <memory>
#include <vector>
#include <cstdarg>
#include "exportgltf/exportgltf.h"
#include "exportgltf/exportgltf2xml.h"
#include "skp2xml/xmlexporter.h"
#include "tinyxml2/tinyxml2.h"
#include "skp2xml/xmlexporter.h"
double getRatio(SUModelUnits units)
{
    switch (units)
    {
        case SUModelUnits_Inches:
            return 0.0254;
        case SUModelUnits_Feet:
            return 0.3048;
        case SUModelUnits_Millimeters:
            return 0.001;
        case SUModelUnits_Centimeters:
            return 0.01;
        case SUModelUnits_Meters:
            return 1;
        default:
            return 1;
    }
}
int main(int argc, char **argv)
{
    const char *skp_file  = argv[1];
    const char *file_path = argv[2];
    const char *gltf_file = argv[3];
    CXmlExporter cXmlExporter;
    std::cout << "Start conversion" << std::endl;
    cXmlExporter.Convert(skp_file, gltf_file, file_path, nullptr);
    std::cout << "finished" << std::endl; 
    return 0;
}
