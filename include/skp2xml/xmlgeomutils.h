// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#ifndef SKPTOXML_COMMON_XMLGEOMUTILS_H
#define SKPTOXML_COMMON_XMLGEOMUTILS_H

#include <SketchUpAPI/geometry.h>

// This module defines geometric classes that are useful in processing
// the objects coming from SketchUp.

namespace XmlGeomUtils {

// Vector Class----------------------------------------
class CVector3d {
 public:

  CVector3d(): x_(0.0), y_(0.0), z_(0.0) {}
  CVector3d(double x, double y, double z): x_(x), y_(y), z_(z) {}
  ~CVector3d() {}

  void SetDirection(double x, double y, double z){x_=x; y_=y; z_=z;}

  double x() const {return x_;}
  double y() const {return y_;}
  double z() const {return z_;}

  void set_x(double x) { x_ = x; }
  void set_y(double y) { y_ = y; }
  void set_z(double z) { z_ = z; }

  CVector3d operator+(const CVector3d& vec) const;
  CVector3d operator-(const CVector3d& vec) const;
  void operator+=(const CVector3d& vec);
  void operator-=(const CVector3d& vec);

  CVector3d operator*(double scale) const;
  CVector3d operator/(double scale) const;
  void operator*=(double scale);
  void operator/=(double scale);

  bool operator==(const CVector3d& vec) const;
  bool operator!=(const CVector3d& vec) const;

 protected:
  double x_;
  double y_;
  double z_;
};


// Point Class----------------------------------------
class CPoint3d {
 public:
  CPoint3d(): x_(0.0), y_(0.0), z_(0.0) {}
  CPoint3d(SUPoint3D pt) : x_(pt.x), y_(pt.y), z_(pt.z) {}
  CPoint3d(double x, double y, double z): x_(x), y_(y), z_(z) {}
  ~CPoint3d() {}

  void SetLocation(double x, double y, double z) {
    x_=x;
    y_=y;
    z_=z;
  }

  double x() const { return x_; }
  double y() const { return y_; }
  double z() const { return z_; }

  void set_x(double x) { x_ = x; }
  void set_y(double y) { y_ = y; }
  void set_z(double z) { z_ = z; }

  void operator+=(const CVector3d &vec);
  void operator-=(const CVector3d &vec);

  CPoint3d operator+(const CVector3d& vec) const;
  CPoint3d operator+(const CPoint3d& pt) const;
  CPoint3d operator-(const CVector3d& vec) const;
  CVector3d operator-(const CPoint3d& pt) const;

  CPoint3d operator* (double scale) const {
    return CPoint3d (x_ * scale, y_ * scale, z_ * scale);
  }
  void operator*= (double scale) {*this = *this * scale;}
  CPoint3d operator/ (double invScale) const {
    return *this * (1.0 / invScale);
  }
  void operator/= (double invScale) {*this = *this / invScale;}

 protected:
  double x_;
  double y_;
  double z_;
};

} // end namespace XmlGeomUtils

#endif // SKPTOXML_COMMON_XMLGEOMUTILS_H
