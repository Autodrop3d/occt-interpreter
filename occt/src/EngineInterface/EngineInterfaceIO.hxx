#ifndef EngineInterfaceIO_H
#define EngineInterfaceIO_H

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax2.hxx>
#include <Data.hpp>
#include <TopoEngineCommands.hpp>

namespace EngineInterface {

gp_Pnt pntRead(DATA& pt) {
  return gp_Pnt(pt[0].ToFloat(), pt[1].ToFloat(), pt[2].ToFloat());
}

gp_Vec vecRead(DATA& pt) {
  return gp_Vec(pt[0].ToFloat(), pt[1].ToFloat(), pt[2].ToFloat());
}

gp_Dir dirRead(DATA& pt) {
  return gp_Dir(pt[0].ToFloat(), pt[1].ToFloat(), pt[2].ToFloat());
}


TColgp_Array1OfPnt pointArrayRead(DATA points) {
  TColgp_Array1OfPnt out(1, points.length());
  int i = 1;
  for(auto &pnt : points.ArrayRange() ) {
    out.ChangeValue(i++) = pntRead(pnt);
  }
  return out;
}

TColStd_Array1OfReal realArrayRead(DATA reals) {
  TColStd_Array1OfReal out(1, reals.length());
  int i = 1;
  for( auto &v : reals.ArrayRange() ) {
    out.ChangeValue(i++) = v.ToFloat();
  }
  return out;
}

TColStd_Array1OfInteger intArrayRead(DATA ints) {
  TColStd_Array1OfInteger out(1, ints.length());
  int i = 1;
  for( auto &v : ints.ArrayRange() ) {
    out.ChangeValue(i++) = v.ToInt();
  }
  return out;
}

TColStd_Array1OfInteger intVectorRead(std::vector<int> ints) {
  TColStd_Array1OfInteger out(1, ints.size());
  int i = 1;
  for( auto &v : ints) {
    out.ChangeValue(i++) = v;
  }
  return out;
}

TColStd_Array1OfReal realVectorRead(std::vector<double> reals) {
  TColStd_Array1OfReal out(1, reals.size());
  int i = 1;
  for( auto &v : reals) {
    out.ChangeValue(i++) = v;
  }
  return out;
}

}

#endif // EngineInterfaceIO_H

