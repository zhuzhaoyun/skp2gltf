// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#include <set>
#include <string>
#include <vector>

#include "./xmltexturehelper.h"

#include <SketchUpAPI/model/component_definition.h>
#include <SketchUpAPI/model/component_instance.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/group.h>
#include <SketchUpAPI/model/image.h>
#include <SketchUpAPI/model/layer.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/texture.h>
#include <SketchUpAPI/model/texture_writer.h>
#include <SketchUpAPI/model/typed_value.h>

CXmlTextureHelper::CXmlTextureHelper() {
}

size_t CXmlTextureHelper::LoadAllTextures(SUModelRef model,
                                          SUTextureWriterRef texture_writer,
                                          bool textures_from_layers) {
  if (SUIsInvalid(texture_writer)) return 0;

  if (textures_from_layers) {
    // Layers only
    size_t num_layers;
    SUResult result = SUModelGetNumLayers(model, &num_layers);
    if ((result == SU_ERROR_NONE) && (num_layers > 0)) {
      std::vector<SULayerRef> layers(num_layers);
      result = SUModelGetLayers(model, num_layers, &layers[0], &num_layers);
      if (result == SU_ERROR_NONE) {
        for (size_t i = 0; i < num_layers; i++) {
          SULayerRef layer = layers[i];
          long texture_id = 0;
          result = SUTextureWriterLoadEntity(texture_writer,
                                             SULayerToEntity(layer),
                                             &texture_id);
        }
      }
    }
  } else {
    // Get the root component
    SUEntitiesRef model_entities;
    if (SU_ERROR_NONE == SUModelGetEntities(model, &model_entities)) {
      LoadEntities(texture_writer, model_entities);

      // Next, all the component definitions
      size_t num_definitions = 0;
      SUModelGetNumComponentDefinitions(model, &num_definitions);
      if (num_definitions > 0) {
        std::vector<SUComponentDefinitionRef> component_definitions(num_definitions);
        SUModelGetComponentDefinitions(model, num_definitions, 
                             &component_definitions[0],
                             &num_definitions);

        for (size_t i = 0; i < num_definitions; i++) {
          SUComponentDefinitionRef component_definition =
              component_definitions[i];
          LoadComponent(texture_writer, component_definition);
        }
      }
    }
  }

  // Return the number of textures
  size_t count = 0;
  if (SU_ERROR_NONE !=
      SUTextureWriterGetNumTextures(texture_writer, &count)) {
    count = 0;
  }
  return count;
}

void CXmlTextureHelper::LoadComponent(SUTextureWriterRef texture_writer,
                                      SUComponentDefinitionRef component) {
  SUEntitiesRef entities = SU_INVALID;
  SUComponentDefinitionGetEntities(component, &entities);
  LoadEntities(texture_writer, entities);
}

void CXmlTextureHelper::LoadEntities(SUTextureWriterRef texture_writer,
                                     SUEntitiesRef entities) {
  // Top level faces, instances, groups, and images
  LoadFaces(texture_writer, entities);
  LoadComponentInstances(texture_writer, entities);
  LoadGroups(texture_writer, entities);
  LoadImages(texture_writer, entities);
}

void CXmlTextureHelper::LoadFaces(SUTextureWriterRef texture_writer,
                                  SUEntitiesRef entities) {
  SUResult hr;
  if (!SUIsInvalid(entities)) {
    size_t num_faces = 0;
    SUEntitiesGetNumFaces(entities, &num_faces);
    if (num_faces > 0) {
      std::vector<SUFaceRef> faces(num_faces);
      SUEntitiesGetFaces(entities, num_faces, &faces[0], &num_faces);

      for (size_t i = 0; i < num_faces; i++) {
        SUFaceRef face = faces[i];
        long front_texture_id = 0;
        long back_texture_id = 0;
        hr = SUTextureWriterLoadFace(texture_writer, face, &front_texture_id,
            &back_texture_id);
      }
    }
  }
}


void CXmlTextureHelper::LoadComponentInstances(
    SUTextureWriterRef texture_writer, SUEntitiesRef entities) {
  SUResult hr;
  if (!SUIsInvalid(entities)) {
    size_t num_instances = 0;
    SUEntitiesGetNumInstances(entities, &num_instances);
    if (num_instances > 0) {
      std::vector<SUComponentInstanceRef> instances(num_instances);
      SUEntitiesGetInstances(entities, num_instances,
                             &instances[0], &num_instances);

      for (size_t i = 0; i < num_instances; i++) {
        SUComponentInstanceRef instance = instances[i];
        if (!SUIsInvalid(instance)) {
            long texture_id = 0;
            hr = SUTextureWriterLoadEntity(texture_writer,
                                           SUComponentInstanceToEntity(instance),
                                           &texture_id);
        }
      }
    }
  }
}

void CXmlTextureHelper::LoadGroups(SUTextureWriterRef texture_writer,
                                   SUEntitiesRef entities) {
  if (!SUIsInvalid(entities)) {
    size_t num_groups;
    SUEntitiesGetNumGroups(entities, &num_groups);
    if (num_groups > 0) {
      std::vector<SUGroupRef> groups(num_groups);
      SUEntitiesGetGroups(entities, num_groups, &groups[0], &num_groups);

      for (size_t i = 0; i < num_groups; i++) {
        SUGroupRef group = groups[i];
        if (!SUIsInvalid(group)) {
          // Get the component part of the group
          SUEntitiesRef group_entities = SU_INVALID;
          SUGroupGetEntities(group, &group_entities);
          LoadEntities(texture_writer, group_entities);
        }
      }
    }
  }
}

void CXmlTextureHelper::LoadImages(SUTextureWriterRef texture_writer,
                                   SUEntitiesRef entities) {
  if (!SUIsInvalid(entities)) {
    size_t num_images = 0;
    SUEntitiesGetNumImages(entities, &num_images);
    if (num_images > 0) {
      std::vector<SUImageRef> images(num_images);
      if (SU_ERROR_NONE == SUEntitiesGetImages(entities,
          num_images, &images[0], &num_images)) {
        for (size_t i = 0; i < num_images; i++) {
          SUImageRef image = images[i];
          if (!SUIsInvalid(image)) {
            long texture_id = 0;
            SUTextureWriterLoadEntity(texture_writer,
                                      SUImageToEntity(image),
                                      &texture_id);
          }
        }
      }
    }
  }
}
