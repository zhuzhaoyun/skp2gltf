// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#ifndef SKPTOXML_COMMON_XMLTEXTUREHELPER_H
#define SKPTOXML_COMMON_XMLTEXTUREHELPER_H

#include <SketchUpAPI/model/defs.h>

class CXmlTextureHelper {
 public:
  CXmlTextureHelper();
  virtual ~CXmlTextureHelper() {}

  // Load all textures, return number of textures
  // Input: model to load textures from
  // Input: texture writer
  // Input: layers or entities
  size_t LoadAllTextures(SUModelRef model, SUTextureWriterRef texture_writer,
                         bool textures_from_layers);

 private:
  // Load textures from all of the entities that have textures
  void LoadComponent(SUTextureWriterRef texture_writer,
                     SUComponentDefinitionRef component);
  void LoadEntities(SUTextureWriterRef texture_writer,
                    SUEntitiesRef entities);
  void LoadComponentInstances(SUTextureWriterRef texture_writer,
                              SUEntitiesRef entities);
  void LoadGroups(SUTextureWriterRef texture_writer,
                  SUEntitiesRef entities);
  void LoadFaces(SUTextureWriterRef texture_writer,
                 SUEntitiesRef entities);
  void LoadImages(SUTextureWriterRef texture_writer,
                  SUEntitiesRef entities);
};

#endif // SKPTOXML_COMMON_XMLTEXTUREHELPER_H
