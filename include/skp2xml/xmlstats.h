// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#ifndef SKPTOXML_COMMON_XMLSTATS_H
#define SKPTOXML_COMMON_XMLSTATS_H

// Export Statistics - a struct to help count our exported objects.  These
// are displayed at the end of the export in the results dialog.
class CXmlExportStats {
 public:
  CXmlExportStats() {
    textures_ = 0;
    faces_ = 0;
    edges_ = 0;
    layers_ = 0;
    options_ = 0;
  }

  inline void set_textures(size_t num) { textures_ = num; }
  inline void AddEdge() { edges_++; }
  inline void AddFace() { faces_++; }
  inline void AddLayer() { layers_++; }
  inline void AddOption() { options_++; }

  size_t textures() const { return textures_; }
  size_t faces() const { return faces_; }
  size_t edges() const { return edges_; }
  size_t layers() const { return layers_; }
  size_t options() const { return options_; }

 protected:
  size_t textures_;
  size_t faces_;
  size_t edges_;
  size_t layers_;
  size_t options_;
};

#endif // SKPTOXML_COMMON_XMLSTATS_H
