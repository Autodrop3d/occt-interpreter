// Created on: 1993-08-12
// Created by: Bruno DUMORTIER
// Copyright (c) 1993-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <DrawTrSurf.hxx>

#include <Draw.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Geometry.hxx>
#include <Geom_Surface.hxx>
#include <GeomTools_Curve2dSet.hxx>
#include <GeomTools_CurveSet.hxx>
#include <GeomTools_SurfaceSet.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax2d.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Trsf2d.hxx>
#include <Poly.hxx>
#include <Poly_Polygon2D.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_Triangulation.hxx>
#include <Standard_Stream.hxx>
#include <TCollection_AsciiString.hxx>

static TCollection_AsciiString ColorsHint(
"The possible colors are: \n\
  white, red, green, blue, cyan,\n\
  golden, magenta, brown, orange, pink,\n\
  salmon, violet, yellow, darkgreen, coral");

static TCollection_AsciiString MarkersHint(
"The possible markers are: \n\
  square, diamond, x, plus, circle, circle_zoom");


//=======================================================================
//function : transform
//purpose  : 
//=======================================================================

static Standard_Integer transform (Draw_Interpretor& di, Standard_Integer n, const char** a)
{
  if (n < 5) return 1;
  gp_Trsf T;
  Standard_Integer i,last = n-1;
  if (!strcmp(a[0],"pscale")) {
    Standard_Real s = Draw::Atof(a[last]);
    last--;
    if (last < 4) return 1;
    gp_Pnt P(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    T.SetScale(P,s);
  }
  else if (!strcmp(a[0]+1,"mirror")) {
    if (last < 4) return 1;
    gp_Pnt P(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    if (*a[0] == 'p') {
      T.SetMirror(P);
    }
    else {
      last -= 3;
      if (last < 4) return 1;
      gp_Pnt O(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
      last -= 3;
      gp_Dir D(P.X(),P.Y(),P.Z());
      if (*a[0] == 'l') {
	T.SetMirror(gp_Ax1(O,D));
      }
      else if (*a[0] == 's') {
	gp_Pln Pl(O,D);
	T.SetMirror(Pl.Position().Ax2());
      }
    }
  }

  else if (!strcmp(a[0],"translate")) {
    if (last < 4) return 1;
    gp_Vec V(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    last -= 3;
    T.SetTranslation(V);
  }

  else if (!strcmp(a[0],"rotate")) {
    if (last < 8) return 1;
    Standard_Real ang = Draw::Atof(a[last]) * (M_PI / 180.0);
    last --;
    gp_Dir D(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    last -= 3;
    gp_Pnt P(Draw::Atof(a[last-2]),Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    last -= 3;
    T.SetRotation(gp_Ax1(P,D),ang);
  }

  for (i = 1; i <= last; i++) {
    Handle(Geom_Geometry) G = DrawTrSurf::Get(a[i]);
    if (!G.IsNull()) {
      G->Transform(T);
      Draw::Repaint();
    }
    else {
      gp_Pnt P;
      if (DrawTrSurf::GetPoint(a[i],P)) {
	P.Transform(T);
	DrawTrSurf::Set(a[i],P);
      }
    }
    di << a[i] << " ";
  }
  return 0;
}

//=======================================================================
//function : d2transform
//purpose  : 
//=======================================================================

static Standard_Integer d2transform (Draw_Interpretor& di, Standard_Integer n, const char** a)
{
  if (n < 4) return 1;
  gp_Trsf2d T;
  Standard_Integer i,last = n-1;
  if (!strcmp(a[0],"2dpscale")) {
    Standard_Real s = Draw::Atof(a[last]);
    last--;
    if (last < 3) return 1;
    gp_Pnt2d P(Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    T.SetScale(P,s);
  }
  else if ( (!strcmp(a[0],"2dpmirror")) || 
	    (!strcmp(a[0],"2dlmirror"))   ) {
    if (last < 3) return 1;
    gp_Pnt2d P(Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    if (!strcmp(a[0],"2dpmirror")) {
      T.SetMirror(P);
    }
    else {
      last -= 2;
      if (last < 3) return 1;
      gp_Pnt2d O(Draw::Atof(a[last-1]),Draw::Atof(a[last]));
      last -= 2;
      gp_Dir2d D(P.X(),P.Y());
      T.SetMirror(gp_Ax2d(O,D));
    }
  }

  else if (!strcmp(a[0],"2dtranslate")) {
    if (last < 3) return 1;
    gp_Vec2d V(Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    last -= 2;
    T.SetTranslation(V);
  }

  else if (!strcmp(a[0],"2drotate")) {
    if (last < 4) return 1;
    Standard_Real ang = Draw::Atof(a[last]) * (M_PI / 180.0);
    last --;
    gp_Pnt2d P(Draw::Atof(a[last-1]),Draw::Atof(a[last]));
    last -= 2;
    T.SetRotation(P,ang);
  }

  for (i = 1; i <= last; i++) {
    Handle(Geom2d_Curve) G = DrawTrSurf::GetCurve2d(a[i]);
    if (!G.IsNull()) {
      G->Transform(T);
      Draw::Repaint();
    }
    else {
      gp_Pnt2d P;
      if (DrawTrSurf::GetPoint2d(a[i],P)) {
	P.Transform(T);
	DrawTrSurf::Set(a[i],P);
      }
    }
    di << a[i] << " ";
  }
  return 0;
}

//=======================================================================
//function : Set
//purpose  : point
//=======================================================================
void DrawTrSurf::Set (const Standard_CString theName,
                      const gp_Pnt& thePoint)
{
  DrawTrSurf::points[theName] = thePoint;
}

//=======================================================================
//function : Set
//purpose  : point
//=======================================================================
void DrawTrSurf::Set (const Standard_CString theName,
                      const gp_Pnt2d& thePoint)
{
  DrawTrSurf::points2d[theName] = thePoint;
}

//=======================================================================
//function : Set
//purpose  : Geometry from Geom
//=======================================================================
void  DrawTrSurf::Set(const Standard_CString theName,
                      const Handle(Geom_Geometry)& theGeometry,
                      const Standard_Boolean isSenseMarker)
{
    DrawTrSurf::geom[theName] = theGeometry;
}

//=======================================================================
//function : Set
//purpose  : Curve from Geom2d
//=======================================================================
void  DrawTrSurf::Set(const Standard_CString theName,
                      const Handle(Geom2d_Curve)& theCurve,
                      const Standard_Boolean isSenseMarker)
{
  DrawTrSurf::geom2d[theName] = theGeometry;
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
void DrawTrSurf::Set(const Standard_CString Name, 
		     const Handle(Poly_Triangulation)& T)
{
  DrawTrSurf::triangulation[Name] = T;
}
//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
void DrawTrSurf::Set(const Standard_CString Name, 
		     const Handle(Poly_Polygon3D)& P)
{
  DrawTrSurf::polygons[Name] = P;
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
void DrawTrSurf::Set(const Standard_CString Name, 
		     const Handle(Poly_Polygon2D)& P)
{
  DrawTrSurf::polygons2d[Name] = P;
}

//=======================================================================
//function : Get
//purpose  : 
//=======================================================================
Handle(Geom_Geometry)  DrawTrSurf::Get(Standard_CString& Name)
{
  return DrawTrSurf::geom[Name];
}


//=======================================================================
//function : GetPoint
//purpose  : 
//=======================================================================
Standard_Boolean DrawTrSurf::GetPoint(Standard_CString& Name,
				      gp_Pnt& P)
{
  if (DrawTrSurf::points.count(Name)) {
    P = DrawTrSurf::points[Name];
    return Standard_True;
  } else {
    return Standard_False;
  }
}

//=======================================================================
//function : GetPoint2d
//purpose  : 
//=======================================================================
Standard_Boolean DrawTrSurf::GetPoint2d(Standard_CString& Name,
					gp_Pnt2d& P)
{
  if (DrawTrSurf::points2d.count(Name)) {
    P = DrawTrSurf::points2d[Name];
    return Standard_True;
  } else {
    return Standard_False;
  }
}

//=======================================================================
//function : GetCurve
//purpose  : 
//=======================================================================
Handle(Geom_Curve)  DrawTrSurf::GetCurve(Standard_CString& Name)
{
  return Handle(Geom_Curve)::DownCast(DrawTrSurf::geom[Name]);
}


//=======================================================================
//function : GetBezierCurve
//purpose  : 
//=======================================================================
Handle(Geom_BezierCurve)  DrawTrSurf::GetBezierCurve(Standard_CString& Name)
{
  return Handle(Geom_BezierCurve)::DownCast(DrawTrSurf::geom[Name]);
}


//=======================================================================
//function : GetBSplineCurve
//purpose  : 
//=======================================================================
Handle(Geom_BSplineCurve)  DrawTrSurf::GetBSplineCurve(Standard_CString& Name)
{
  return Handle(Geom_BSplineCurve)::DownCast(DrawTrSurf::geom[Name]);
}
//=======================================================================
//function : GetCurve2d
//purpose  : 
//=======================================================================
Handle(Geom2d_Curve)  DrawTrSurf::GetCurve2d(Standard_CString& Name)
{
  return Handle(Geom2d_Curve)::DownCast(DrawTrSurf::geom2d[Name]);
}
//=======================================================================
//function : GetBezierCurve2d
//purpose  : 
//=======================================================================
Handle(Geom2d_BezierCurve)  DrawTrSurf::GetBezierCurve2d(Standard_CString& Name)
{
  return Handle(Geom2d_BezierCurve)::DownCast(DrawTrSurf::geom2d[Name]);
}
//=======================================================================
//function : GetBSplineCurve2d
//purpose  : 
//=======================================================================
Handle(Geom2d_BSplineCurve)  DrawTrSurf::GetBSplineCurve2d
       (Standard_CString& Name)
{
  return Handle(Geom2d_BSplineCurve)::DownCast(DrawTrSurf::geom2d[Name]);
}
//=======================================================================
//function : GetSurface
//purpose  : 
//=======================================================================
Handle(Geom_Surface)  DrawTrSurf::GetSurface
       (Standard_CString& Name)
{
  return Handle(Geom_Surface)::DownCast(DrawTrSurf::geom[Name]);
}
//=======================================================================
//function : GetBezierSurface
//purpose  : 
//=======================================================================
Handle(Geom_BezierSurface)  DrawTrSurf::GetBezierSurface
       (Standard_CString& Name)
{
  return Handle(Geom_BezierSurface)::DownCast(DrawTrSurf::geom[Name]);  
}
//=======================================================================
//function : GetBSplineSurface
//purpose  : 
//=======================================================================
Handle(Geom_BSplineSurface) DrawTrSurf::GetBSplineSurface
       (Standard_CString& Name)
{
  return Handle(Geom_BSplineSurface)::DownCast(DrawTrSurf::geom[Name]);  
}
//=======================================================================
//function : GetTriangulation
//purpose  : 
//=======================================================================
Handle(Poly_Triangulation) DrawTrSurf::GetTriangulation(Standard_CString& Name)
{
  return DrawTrSurf::triangulation[Name];  
}
//=======================================================================
//function : GetPolygon3D
//purpose  : 
//=======================================================================
Handle(Poly_Polygon3D) DrawTrSurf::GetPolygon3D(Standard_CString& Name)
{
  return DrawTrSurf::polygons[Name];  
}
//=======================================================================
//function : GetPolygon2D
//purpose  : 
//=======================================================================
Handle(Poly_Polygon2D) DrawTrSurf::GetPolygon2D(Standard_CString& Name)
{
  return DrawTrSurf::polygons2d[Name];  
}


//=======================================================================
//function : BasicCommands
//purpose  : 
//=======================================================================

static Standard_Boolean done = Standard_False;
void  DrawTrSurf::BasicCommands(Draw_Interpretor& theCommands)
{
  if (done) return;
  done = Standard_True;


  const char* g;
  g = "geometric display commands";
  
  
  g = "Geometric transformations";
  
  theCommands.Add("translate",
		  "translate name [names...] dx dy dz",
                  __FILE__,
		  transform,g);

  theCommands.Add("rotate",
		  "rotate name [names...] x y z dx dy dz angle",
                  __FILE__,
		  transform,g);

  theCommands.Add("pmirror",
		  "pmirror name [names...] x y z",
                  __FILE__,
		  transform,g);

  theCommands.Add("lmirror",
		  "lmirror name [names...] x y z dx dy dz",
                  __FILE__,
		  transform,g);

  theCommands.Add("smirror",
		  "smirror name [names...] x y z dx dy dz",
                  __FILE__,
		  transform,g);

  theCommands.Add("pscale",
		  "pscale name [names...] x y z s",
                  __FILE__,
		  transform,g);

  theCommands.Add("2dtranslate",
		  "translate name [names...] dx dy",
                  __FILE__,
		  d2transform,g);

  theCommands.Add("2drotate",
		  "rotate name [names...] x y dx dy  angle",
                  __FILE__,
		  d2transform,g);

  theCommands.Add("2dpmirror",
		  "pmirror name [names...] x y",
                  __FILE__,
		  d2transform,g);

  theCommands.Add("2dlmirror",
		  "lmirror name [names...] x y dx dy",
                  __FILE__,
		  d2transform,g);

  theCommands.Add("2dpscale",
		  "pscale name [names...] x y s",
                  __FILE__,
		  d2transform,g);
}
