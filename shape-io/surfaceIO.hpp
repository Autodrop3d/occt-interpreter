#ifndef _SURFACE_IO
#define _SURFACE_IO

#include "data.hpp"
#include "commonIO.hpp"
#include <gp_Pln.hxx>
#include <Geom_Plane.hxx>

namespace e0 {
namespace io {


DATA surfaceWrite(const Handle(Geom_BSplineSurface)& surface)
{
  //  a b-spline surface is defined by :
  //         The U and V Degree (up to 25)
  //         The Poles  (and the weights if it is rational)
  //         The U and V Knots and Multiplicities
  //         
  //  The knot vector   is an  increasing  sequence  of reals without  repetition. 
  //  The multiplicities are the repetition of the knots.
  //           
  //  If the knots are regularly spaced (the difference of two consecutive knots  
  //  is a constant),
  //  the knots repartition (in U or V) is :
  //              - Uniform if all multiplicities are 1.
  //              -  Quasi-uniform if  all multiplicities are  1
  //              but the first and the last which are Degree+1.
  //              -   PiecewiseBezier if  all multiplicites  are
  //              Degree but the   first and the  last which are
  //              Degree+1. 
  //              
  //         The surface may be periodic in U and in V. 
  //              On a U periodic surface if there are k U knots
  //              and the poles table  has p rows.  the U period
  //              is uknot(k) - uknot(1)
  //              
  //              the poles and knots are infinite vectors with :
  //                uknot(i+k) = uknot(i) + period
  //                pole(i+p,j) = pole(i,j)


//  Standard_Boolean PeriodU = surface->IsUPeriodic();
//  Standard_Boolean PeriodV = surface->IsVPeriodic();
//  Standard_Boolean CloseU = surface->IsUClosed();
//  Standard_Boolean CloseV = surface->IsVClosed();

  Standard_Real U0,U1,V0,V1;
  surface->Bounds(U0,U1,V0,V1);

  Standard_Integer DegU = surface->UDegree();
  Standard_Integer DegV = surface->VDegree();
  Standard_Boolean RationU = surface->IsURational();
  Standard_Boolean RationV = surface->IsVRational();
  Standard_Integer NbUPoles = surface->NbUPoles();
  Standard_Integer NbVPoles = surface->NbVPoles();
  Standard_Integer IndexU = NbUPoles -1;
  Standard_Integer IndexV = NbVPoles -1;

  DATA uKnots = Array();
  
  TColStd_Array1OfReal KU(1, NbUPoles+ DegU+ 1);
  surface->UKnotSequence(KU);
  for (Standard_Integer Knotindex=KU.Lower(); Knotindex<=KU.Upper(); Knotindex++) { 
    uKnots.append(KU.Value(Knotindex));
  }
  
  DATA vKnots = Array();
  TColStd_Array1OfReal KV(1, NbVPoles+ DegV+ 1);
  surface->VKnotSequence(KV);
  for (Standard_Integer Knotindex=KV.Lower(); Knotindex<=KV.Upper(); Knotindex++) { 
    vKnots.append(KV.Value(Knotindex));
  }

  // ----------------------------------------------
  DATA Weights = Array();
  Standard_Integer iw, jw;

  bool rational = RationU || RationV;

  for ( iw = 1; iw<= IndexU+1; iw++) {
    DATA col = Array();
    for ( jw = 1; jw<= IndexV+1; jw++) {
      col.append(rational ? surface->Weight(iw,jw) :  1.0);
    }
    Weights.append(col);
  }
  // ---------------------------------------------
    
  DATA Poles = Array();

  Standard_Integer ipole, jpole;

  for ( ipole = 1; ipole<= IndexU+1; ipole++) {
    DATA col = Array();
    for ( jpole = 1; jpole<= IndexV+1; jpole++) {
      gp_Pnt tempPnt = surface-> Pole(ipole, jpole);
      col.append(pntWrite(tempPnt));
    }     
    Poles.append(col);
  }

  DATA out = {
    "TYPE", "B-SPLINE",
    "degU", DegU,
    "degV", DegV,
    "knotsU", uKnots,
    "knotsV", vKnots,
    "weights", Weights,
    "cp", Poles
  };

  return out;
}

DATA surfaceWrite(const Handle(Geom_Plane)& surface) {

  gp_Pln aPln = surface->Pln();

  DATA out = {
    "TYPE", "PLANE",
    "normal", dirWrite(aPln.Axis().Direction()),
    "origin", pntWrite(aPln.Location()),
    "direct", aPln.Direct()
  };
  return out;
  
}

}
}

#endif