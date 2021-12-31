#ifndef E0_IO_CURVE_H
#define E0_IO_CURVE_H

#include "data.hpp"
#include "commonIO.hpp"
#include <Geom_Plane.hxx>

#include <Geom_OffsetCurve.hxx>
#include <Geom_Conic.hxx>
#include <Geom_Line.hxx>
#include <Geom_BoundedCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <BRepTools_NurbsConvertModification.hxx>

namespace e0 {
namespace io {

DATA onlyType(char const* type) {
  DATA out = {
    "TYPE", type
  };
  return out;
}

DATA curveWriteBSpline(Handle(Geom_BSplineCurve)& curve)
{
    
  Standard_Boolean periodic = curve->IsPeriodic();
  if (periodic) {
    curve = Handle(Geom_BSplineCurve)::DownCast(curve->Copy());
    curve->SetNotPeriodic();
  }

//  Standard_Real U0 = curve->FirstParameter();
//  Standard_Real U1 = curve->LastParameter();

  Standard_Integer Deg = curve->Degree();
  Standard_Boolean rational = curve->IsRational();
  Standard_Integer NbPoles = curve->NbPoles();

  DATA knots = Array();
  
  const TColStd_Array1OfReal& KU = curve->Knots();
  
  for (Standard_Integer knotIndex=KU.Lower(); knotIndex<=KU.Upper(); knotIndex++) { 
    for (int i = 0; i < curve->Multiplicity(knotIndex); ++i) {
      knots.append(KU.Value(knotIndex));
    }
  }

  // ----------------------------------------------
  DATA Weights = Array();
  for (Standard_Integer iw = 1; iw<= NbPoles; iw++) {
    Weights.append(rational ? curve->Weight(iw) :  1.0);
  }
  // ---------------------------------------------
    
  DATA Poles = Array();

  for (Standard_Integer ipole = 1; ipole<= NbPoles; ipole++) {
    gp_Pnt tempPnt = curve->Pole(ipole);
    Poles.append(pntWrite(tempPnt));
  }

  if (knots.size() > 2) {
    //workaround weirdness
    knots[0] = knots[1];
    knots[knots.size() - 1] = knots[knots.size() - 2]; 
  }

  DATA out = {
    "TYPE", "B-SPLINE",
    "deg", Deg,
    "knots",knots,
    "weights", Weights,
    "cp", Poles
  };

  return out;
}

DATA curveWrite(const Handle(Geom_BoundedCurve)& curve) {
  if (curve->IsKind(STANDARD_TYPE(Geom_BSplineCurve))) {
    Handle(Geom_BSplineCurve) tc = Handle(Geom_BSplineCurve)::DownCast(curve);
    return curveWriteBSpline(tc);
  } else if (curve->IsKind(STANDARD_TYPE(Geom_BezierCurve))) {
    return onlyType("BEZIER");
  } else if (curve->IsKind(STANDARD_TYPE(Geom_TrimmedCurve))) {
    return onlyType("TRIMMED");
  } else {
    return onlyType("UNKNOWN");
  }
}


DATA curveWrite(const Handle(Geom_Curve)& curve) {
  if (curve->IsKind(STANDARD_TYPE(Geom_BoundedCurve))) {
    return curveWrite(Handle(Geom_BoundedCurve)::DownCast(curve));
  } else if (curve->IsKind(STANDARD_TYPE(Geom_Conic))) {
    return onlyType("CONIC");
//    return curveWrite((Handle(Geom_Conic)) curve);
  } else if ( curve->IsKind(STANDARD_TYPE(Geom_OffsetCurve))) {
    return onlyType("OFFSET");
//    return curveWrite((Handle(Geom_OffsetCurve)) curve);
  } else if ( curve->IsKind(STANDARD_TYPE(Geom_Line))) {
    return onlyType("LINE");
  } else {
    return onlyType("UNKNOWN");
  }
}

}
}

#endif // E0_IO_CURVE_H
