// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#include "./xmlgeomutils.h"


namespace XmlGeomUtils {

// Misc Utilities--------------------------------------
const double EqualTol = 1.0e-3;
const double EqualTolSq = EqualTol * EqualTol;

inline bool AreEqual(double val1, double val2, double tol = EqualTol) {
  double diff = val1 - val2;
  return diff <= tol && diff >= -tol;
}

// Point Class----------------------------------------
void CPoint3d::operator+=(const CVector3d& vec) {
  x_ += vec.x();
  y_ += vec.y();
  z_ += vec.z();
}

void CPoint3d::operator-=(const CVector3d& vec) {
  x_ -= vec.x();
  y_ -= vec.y();
  z_ -= vec.z();
}

CPoint3d CPoint3d::operator+(const CVector3d& vec) const {
  double x = x_ + vec.x();
  double y = y_ + vec.y();
  double z = z_ + vec.z();
  return CPoint3d(x, y, z);
}

CPoint3d CPoint3d::operator+(const CPoint3d& pt) const {
  double x = x_ + pt.x_;
  double y = y_ + pt.y_;
  double z = z_ + pt.z_;
  return CPoint3d(x, y, z);
}

CPoint3d CPoint3d::operator-(const CVector3d& vec) const {
  double x = x_ - vec.x();
  double y = y_ - vec.y();
  double z = z_ - vec.z();
  return CPoint3d(x, y, z);
}

CVector3d CPoint3d::operator-(const CPoint3d& pt) const {
  double dx = x_ - pt.x_;
  double dy = y_ - pt.y_;
  double dz = z_ - pt.z_;
  return CVector3d(dx, dy, dz);
}

// Vector Class----------------------------------------
CVector3d CVector3d::operator+(const CVector3d& vec) const {
  return CVector3d(x_ + vec.x_, y_ + vec.y_, z_ + vec.z_);
}

CVector3d CVector3d::operator-(const CVector3d& vec) const {
  return CVector3d(x_ - vec.x_, y_ - vec.y_, z_ - vec.z_);
}

void CVector3d::operator+=(const CVector3d& vec) {
  x_ += vec.x_;
  y_ += vec.y_;
  z_ += vec.z_;
}

void CVector3d::operator-=(const CVector3d& vec) {
  x_ -= vec.x_;
  y_ -= vec.y_;
  z_ -= vec.z_;
}

CVector3d CVector3d::operator*(double scale) const {
  return CVector3d(x_ * scale, y_ * scale, z_ * scale);
}

CVector3d CVector3d::operator/(double scale) const {
  return CVector3d(x_ / scale, y_ / scale, z_ / scale);
}

void CVector3d::operator*=(double scale) {
  x_ *= scale;
  y_ *= scale;
  z_ *= scale;
}

void CVector3d::operator/=(double scale) {
  x_ /= scale;
  y_ /= scale;
  z_ /= scale;
}

bool CVector3d::operator==(const CVector3d& v) const {
  return AreEqual(x_, v.x_, EqualTol) && 
         AreEqual(y_, v.y_, EqualTol) &&
         AreEqual(z_, v.z_, EqualTol);
}

bool CVector3d::operator!=(const CVector3d& v) const {
  return !operator==(v);
}

} // end namespace XmlGeomUtils
