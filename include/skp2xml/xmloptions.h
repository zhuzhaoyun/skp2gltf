// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#ifndef SKPTOXML_COMMON_XMLOPTIONS_H
#define SKPTOXML_COMMON_XMLOPTIONS_H

class CXmlOptions {
 public:
  CXmlOptions(void) {
   export_materials_ = true;
   export_faces_ = true;
   export_edges_ = true;
   export_materials_by_layer_ = false;
   export_layers_ = true;
   export_options_ = false;
  }

  virtual ~CXmlOptions(void) {}

  inline bool export_materials() const { return export_materials_; }
  inline void set_export_materials(bool value) { export_materials_ = value; }

  inline bool export_faces() const { return export_faces_; }
  inline void set_export_faces(bool value) { export_faces_ = value; }

  inline bool export_edges() const { return export_edges_; }
  inline void set_export_edges(bool value) { export_edges_ = value; }

  inline bool export_materials_by_layer() const {
      return export_materials_by_layer_;
  }
  inline void set_export_materials_by_layer(bool value) {
      export_materials_by_layer_ = value;
  }

  inline bool export_layers() const { return export_layers_; }
  inline void set_export_layers(bool value) { export_layers_ = value; }

  inline bool export_options() const { return export_options_; }
  inline void set_export_options(bool value) { export_options_ = value; }

 private:
  bool export_materials_;
  bool export_faces_;
  bool export_edges_;
  bool export_materials_by_layer_;
  bool export_layers_;
  bool export_options_;
};

#endif // SKPTOXML_COMMON_XMLOPTIONS_H
