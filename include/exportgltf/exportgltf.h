/*
 * @Author: yaol
 * @Date: 2025-02-18 17:30:53
 * @Last Modified by: yaol
 * @Last Modified time: 2025-02-18 17:33:34
 */

#ifndef EXPORTGLTF_H
#define EXPORTGLTF_H
#include <string>
#include <vector>
#include <memory>
#include <vector>
#include "creategltfcommon.h"
#include <SketchUpAPI/sketchup.h>
#define SUSetInvalid(VARIABLE) (VARIABLE).ptr = 0
namespace skp2gltf
{

  class GltfExporter
  {
  public:
    static std::unique_ptr<GltfExporter> create();

    virtual int loadFromEntities(SUEntitiesRef, std::string, double) = 0;
    virtual int exportToGltf(const std::string) const = 0;
  };
}

#endif
