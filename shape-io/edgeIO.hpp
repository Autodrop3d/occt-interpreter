#ifndef E0_IO_EDGE_H
#define E0_IO_EDGE_H

#include <Geom_Plane.hxx>

#include <TopLoc_Location.hxx>
#include <BRep_Tool.hxx>
#include <gp_Trsf.hxx>
#include <gp_Pnt.hxx>

#include "curveIO.hpp"
#include "data.hpp"
#include "commonIO.hpp"


namespace e0 {
namespace io {


DATA edgeWrite(const TopoDS_Edge& edge)
{

  // returns the 3d curve of the edge and the parameter range
  TopLoc_Location L;
  Standard_Real tol;

  Handle(Geom_Curve) Curve3d;
  BRepTools_NurbsConvertModification nurbsMod;
  nurbsMod.NewCurve(edge, Curve3d, L, tol);
  Standard_Real First, Last;

  bool getParams = true;

  if (Curve3d.IsNull()) {
    std::cout << "can't transform curve to NURBS, defaulting to base" << std::endl;
    Curve3d = BRep_Tool::Curve(edge, L, First, Last);
    getParams = false;
  }

  
  DATA edgeOut = Object();

  if (!Curve3d.IsNull()) {
    gp_Trsf Tr = L.Transformation();
    if (Tr.Form() != gp_Identity) {
      Curve3d = Handle(Geom_Curve)::DownCast(Curve3d->Transformed (Tr));
    } else {
      Curve3d = Handle(Geom_Curve)::DownCast(Curve3d->Copy()); 
    }
    
    if (getParams) {
      First = Curve3d->FirstParameter();
      Last = Curve3d->LastParameter();
    }

    DATA domain = Array();
    domain.append(First);
    domain.append(Last);

    //if it needs to be bound 
    //curve->Segment ( Umin, Umax );

    gp_Pnt pt;
    
    Curve3d->D0(First, pt);
    edgeOut["a"] = pntWrite(pt);
    
    Curve3d->D0(Last, pt);
    edgeOut["b"] = pntWrite(pt);

    edgeOut["inverted"] = edge.Orientation() == TopAbs_REVERSED;
    edgeOut["curveBounds"] = domain;
    DATA curveOut = curveWrite(Curve3d);
    edgeOut["curve"] = curveOut;
  }

  return edgeOut;
}

}
}

#endif // E0_IO_EDGE_H
