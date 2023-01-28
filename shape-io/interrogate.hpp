#ifndef E0_IO_INTERROGATE_H
#define E0_IO_INTERROGATE_H

#include <Precision.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Compound.hxx>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepTools_WireExplorer.hxx>

#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_DataMapOfIntegerShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

#include <Poly_Triangulation.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepCheck_Analyzer.hxx>

#include <Poly.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <GeomConvert.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Surface.hxx>
#include "data.hpp"
#include "commonIO.hpp"
#include "surfaceIO.hpp"
#include "edgeIO.hpp"

#include <TopExp.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

#include <BRep_CurveRepresentation.hxx>
#include <BRep_TFace.hxx>
#include <BRep_ListIteratorOfListOfCurveRepresentation.hxx>
#include <BRep_TEdge.hxx>
#include <Standard_DomainError.hxx>

namespace e0 {
namespace io {

typedef NCollection_IndexedDataMap<TopoDS_Shape, DATA, TopTools_ShapeMapHasher> ShapeData;

static const Handle(Poly_PolygonOnTriangulation) nullArray;

const Handle(Poly_PolygonOnTriangulation)&
PolygonOnTriangulation(const TopoDS_Edge&                E, 
                                  const Handle(Poly_Triangulation)& T,
                                  const TopLoc_Location&            L)
{
  TopLoc_Location l = L.Predivided(E.Location());
  Standard_Boolean Eisreversed = (E.Orientation() == TopAbs_REVERSED);

  // find the representation
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More()) {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if ( cr->IsPolygonOnTriangulation(T, L)) {
      if ( cr->IsPolygonOnClosedTriangulation() && Eisreversed )
        return cr->PolygonOnTriangulation2();
      else
        return cr->PolygonOnTriangulation();
    }
    itcr.Next();
  }
  
  return nullArray;
}

gp_Vec computeNormal(const gp_Pnt2d& aUVNode, const Handle(Geom_Surface)& aSurface)
{
  gp_Pnt aDummyPnt;
  gp_Vec aV1, aV2;
  aSurface->D1 (aUVNode.X(), aUVNode.Y(), aDummyPnt, aV1, aV2);
  gp_Vec aNormal = aV1.Crossed (aV2);
  
  aNormal.Multiply (1 / aNormal.Magnitude());
  return aNormal;
}

void writeFaceTessellation(const Handle(Poly_Triangulation)& aTr, const TopLoc_Location& aLocation, 
  const Handle(Geom_Surface)& aSurface, const TColgp_Array1OfPnt& fPoints, DATA& tessOut) {
  
  const Poly_Array1OfTriangle& triangles = aTr->Triangles();  
  Standard_Integer nnn = aTr->NbTriangles(); 
  Standard_Integer nt,n1,n2,n3; 
  bool isPlane = aSurface->IsKind("Geom_Plane");

  for( nt = 1 ; nt < nnn+1 ; nt++) { 
    // takes the node indices of each triangle in n1,n2,n3: 
    triangles(nt).Get(n1,n2,n3); 

    gp_Pnt aPnt1 = fPoints(n1); 
    gp_Pnt aPnt2 = fPoints(n2); 
    gp_Pnt aPnt3 = fPoints(n3); 

    DATA def = Array();

    DATA tr = Array();
    tr.append(pntWrite(aPnt1));  
    tr.append(pntWrite(aPnt2));  
    tr.append(pntWrite(aPnt3));

    def.append(tr);  

    if (!isPlane) {
      DATA norms = Array();
      norms.append(dirWrite(computeNormal(aTr->UVNode(n1), aSurface).Transformed(aLocation)));  
      norms.append(dirWrite(computeNormal(aTr->UVNode(n2), aSurface).Transformed(aLocation)));  
      norms.append(dirWrite(computeNormal(aTr->UVNode(n3), aSurface).Transformed(aLocation)));
      def.append(norms);  
    }
    
    tessOut.append(def);
  }   
}

void writeFaceEvalationPoints(const Handle(Poly_Triangulation)& aTr, const TColgp_Array1OfPnt& fPoints, 
  DATA& out) {
  
  const Poly_Array1OfTriangle& triangles = aTr->Triangles();  
  Standard_Integer nnn = aTr->NbTriangles(); 
  Standard_Integer nt,n1,n2,n3; 

  for( nt = 1 ; nt < nnn+1 ; nt++) { 
    // takes the node indices of each triangle in n1,n2,n3: 
    triangles(nt).Get(n1,n2,n3); 

    gp_Pnt p1 = fPoints(n1); 
    gp_Pnt p2 = fPoints(n2); 
    gp_Pnt p3 = fPoints(n3); 

    
    out.append(xyzWrite(
      (p1.X() + p2.X() + p3.X())/3,   
      (p1.Y() + p2.Y() + p3.Y())/3,   
      (p1.Z() + p2.Z() + p3.Z())/3
    ));
  }   
}

void checkShape(const TopoDS_Shape& aShape)
{


  std::cout << "Validate shape" << std::endl;

 // Checking
  BRepCheck_Analyzer aChecker(aShape);
  if (!aChecker.IsValid())
  {
    // Initialize sequence of subshapes
    TopTools_SequenceOfShape aShapes;
    TopTools_MapOfShape aMap;
    TopAbs_ShapeEnum aTypes[] = {TopAbs_VERTEX, TopAbs_EDGE,
      TopAbs_WIRE, TopAbs_FACE, TopAbs_SHELL};
    TopExp_Explorer ex;
    for (int i=0; i < 5; i++)
      for (ex.Init(aShape, aTypes[i]); ex.More(); ex.Next())
        if (!aMap.Contains(ex.Current()))
        {
          aShapes.Append(ex.Current());
          aMap.Add(ex.Current());
        }

    for (int i=1; i <= aShapes.Length(); i++)
    {
      const TopoDS_Shape& aSubShape = aShapes(i);
      Handle(BRepCheck_Result) aResult = aChecker.Result(aSubShape);
      if (!aResult.IsNull())
      {
        BRepCheck_ListOfStatus aLstStatus;
        // get statuses for a subshape
        aLstStatus = aResult->Status();
        // get statuses for a subshape in context
        aResult->InitContextIterator();
        while (aResult->MoreShapeInContext())
        {
          BRepCheck_ListOfStatus aLst1;
          aLst1 = aResult->StatusOnShape();
          aLstStatus.Append(aLst1);
          aResult->NextShapeInContext();
        }
        BRepCheck_ListIteratorOfListOfStatus itl(aLstStatus);
        for (; itl.More(); itl.Next())
        {
          BRepCheck_Status aStatus = itl.Value();
          // use aStatus as you need
          // ...

          if (aStatus == BRepCheck_NoError)
            continue;

          TopAbs_ShapeEnum aType = aSubShape.ShapeType();
          Standard_CString aTypeName;
          switch (aType)
          {
          case TopAbs_FACE: aTypeName = "FACE"; break;
          case TopAbs_WIRE: aTypeName = "WIRE"; break;
          case TopAbs_EDGE: aTypeName = "EDGE"; break;
          case TopAbs_VERTEX: aTypeName = "VERTEX"; break;
          case TopAbs_SHELL: 
          case TopAbs_COMPOUND:
          case TopAbs_COMPSOLID:
          case TopAbs_SOLID:
          case TopAbs_SHAPE: 
            aTypeName = "SHELL"; break;
          }

          Standard_CString aError;
          switch (aStatus)
          {
            // for vertices
          case BRepCheck_InvalidPointOnCurve: aError = "InvalidPointOnCurve"; break;
          case BRepCheck_InvalidPointOnCurveOnSurface: aError = "InvalidPointOnCurveOnSurface"; break;
          case BRepCheck_InvalidPointOnSurface: aError = "InvalidPointOnSurface"; break;
            // for edges
          case BRepCheck_No3DCurve: aError = "No3DCurve"; break;
          case BRepCheck_Multiple3DCurve: aError = "Multiple3DCurve"; break;
          case BRepCheck_Invalid3DCurve: aError = "Invalid3DCurve"; break;
          case BRepCheck_NoCurveOnSurface: aError = "NoCurveOnSurface"; break;
          case BRepCheck_InvalidCurveOnSurface: aError = "InvalidCurveOnSurface"; break;
          case BRepCheck_InvalidCurveOnClosedSurface: aError = "InvalidCurveOnClosedSurface"; break;
          case BRepCheck_InvalidSameRangeFlag: aError = "InvalidSameRangeFlag"; break;
          case BRepCheck_InvalidSameParameterFlag: aError = "InvalidSameParameterFlag"; break;
          case BRepCheck_InvalidDegeneratedFlag: aError = "InvalidDegeneratedFlag"; break;
          case BRepCheck_FreeEdge: aError = "FreeEdge"; break;
          case BRepCheck_InvalidMultiConnexity: aError = "InvalidMultiConnexity"; break;
          case BRepCheck_InvalidRange: aError = "InvalidRange"; break;
            // for wires
          case BRepCheck_EmptyWire: aError = "EmptyWire"; break;
          case BRepCheck_RedundantEdge: aError = "RedundantEdge"; break;
          case BRepCheck_SelfIntersectingWire: aError = "SelfIntersectingWire"; break;
            // for faces
          case BRepCheck_NoSurface: aError = "NoSurface"; break;
          case BRepCheck_InvalidWire: aError = "InvalidWire"; break;
          case BRepCheck_RedundantWire: aError = "RedundantWire"; break;
          case BRepCheck_IntersectingWires: aError = "IntersectingWires"; break;
          case BRepCheck_InvalidImbricationOfWires: aError = "InvalidImbricationOfWires"; break;
            // for shells
          case BRepCheck_EmptyShell: aError = "EmptyShell"; break;
          case BRepCheck_RedundantFace: aError = "RedundantFace"; break;
            // for shapes
          case BRepCheck_UnorientableShape: aError = "UnorientableShape"; break;
          case BRepCheck_NotClosed: aError = "NotClosed"; break;
          case BRepCheck_NotConnected: aError = "NotConnected"; break;
          case BRepCheck_SubshapeNotInShape: aError = "SubshapeNotInShape"; break;
          case BRepCheck_BadOrientation: aError = "BadOrientation"; break;
          case BRepCheck_BadOrientationOfSubshape: aError = "BadOrientationOfSubshape"; break;
          case BRepCheck_InvalidToleranceValue: aError = "InvalidToleranceValue"; break;
          default : aError = "Undefined error";
          }
          std::cout << aTypeName << " : " << aError << std::endl;
        }
      }
    }
  }
  else
  {
    std::cout << "No errors in shape" << std::endl;

  }

}


DATA 
interrogate(const TopoDS_Shape& aShape, Standard_Real aDeflection = 3, Standard_Boolean INTERROGATE_STRUCT_ONLY = false)
{

  DATA out = Object();
  checkShape(aShape);

  if (aDeflection <= 0) {
    aDeflection = 3;
  }

  // removes all the triangulations of the faces , 
  //and all the polygons on the triangulations of the edges: 
  // BRepTools::Clean(aShape);  
  
  TopTools_IndexedDataMapOfShapeListOfShape edgeFaceMap;
  TopExp::MapShapesAndAncestors(aShape, TopAbs_EDGE, TopAbs_FACE, edgeFaceMap);

  BRepMesh_IncrementalMesh(aShape,aDeflection);  
  DATA facesOut = Array();
  TopExp_Explorer aExpFace; 
  for(aExpFace.Init(aShape,TopAbs_FACE);aExpFace.More();aExpFace.Next()) 
  {   
    DATA faceOut = Object();
    DATA tessOut = Array();
    
    TopoDS_Face aFace = TopoDS::Face(aExpFace.Current());

    Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);
    DATA surfaceOut = NULL;
    if (aSurface->IsKind("Geom_BoundedSurface")) {
      try {
        Handle(Geom_BSplineSurface) bSpline = GeomConvert::SurfaceToBSplineSurface(aSurface);
        surfaceOut = surfaceWrite(bSpline);
      } catch(Standard_DomainError e) {
        surfaceOut = {"TYPE", "UNKNOWN"};
      }
      //if BRepPrimAPI_MakePrism(,,,canonicalize = true ) then all swept surfaces(walls) are forced to planes if possible
    } else if (aSurface->IsKind("Geom_ElementarySurface")) {
//        printf("INTER TYPE: Geom_ElementarySurface \n");
      if (aSurface->IsKind("Geom_Plane")) {
        surfaceOut = surfaceWrite(Handle(Geom_Plane)::DownCast(aSurface));
      } else {
        surfaceOut = {"TYPE", "UNKNOWN"};
      }
    } else if ( aSurface->IsKind("Geom_SweptSurface")) {
//        printf("INTER TYPE: Geom_SweptSurface \n");
      surfaceOut = {"TYPE", "SWEPT"};
    } else if ( aSurface->IsKind("Geom_OffsetSurface")) {
//        printf("INTER TYPE: Geom_OffsetSurface \n");
      surfaceOut = {"TYPE", "OFFSET"};
    } else {
      surfaceOut = {"TYPE", "UNKNOWN"};
    }
    faceOut["surface"] = surfaceOut;

    TopLoc_Location aLocation;  

    Handle(Poly_Triangulation) aTr = BRep_Tool::Triangulation(aFace,aLocation);  
    
    if(!aTr.IsNull()) {  

      // create array of node points in absolute coordinate system 
      TColgp_Array1OfPnt fPoints(1, aTr->NbNodes()); 
      for( Standard_Integer i = 1; i < aTr->NbNodes()+1; i++) { 
        fPoints(i) = aTr->Node(i).Transformed(aLocation);  
      }

      writeFaceTessellation(aTr, aLocation, aSurface, fPoints, tessOut);
      
      //BRepTools::OuterWire(face)  - return outer wire for classification if needed 
      DATA loopsOut = Array();
      TopExp_Explorer wires(aFace, TopAbs_WIRE);
      while (wires.More()) {
        std::cout << "processing wires" << std::endl;
        TopoDS_Wire wire = TopoDS::Wire(wires.Current()); 
        wires.Next();
        BRepTools_WireExplorer aExpEdge(wire);
        DATA edgesOut = Array();
        while (aExpEdge.More()) {
          TopoDS_Edge aEdge = TopoDS::Edge(aExpEdge.Current()); 
          aExpEdge.Next(); 
          
          if(aEdge.IsNull()) {
            std::cout << "edge is null, skipping" << std::endl;
            continue;
          }

          DATA edgeOut = edgeWrite(aEdge);
          if (!edgeOut.hasKey("a") || !edgeOut.hasKey("b")) {
            std::cout << "can't write edge, skipping" << std::endl;
            continue;
          }
          DATA edgeTessOut = Array();
          
          Handle(Poly_PolygonOnTriangulation) edgePol = BRep_Tool::PolygonOnTriangulation(aEdge, aTr, aLocation);  
          if(!edgePol.IsNull())  {
            const TColStd_Array1OfInteger& edgeIndices = edgePol->Nodes(); 
            for( Standard_Integer j = 1; j <= edgeIndices.Length(); j++ ) {
              gp_Pnt edgePoint = fPoints(edgeIndices(j));
              edgeTessOut.append(pntWrite(edgePoint));        
            }
          } else {
            Handle(Poly_PolygonOnTriangulation) pt;
            Handle(Poly_Triangulation) edgeTr;
            TopLoc_Location edgeLoc;
            BRep_Tool::PolygonOnTriangulation(aEdge, pt, edgeTr, edgeLoc);
            if(!pt.IsNull())  {
              for( Standard_Integer j = 1; j <= edgeTr->NbNodes(); j++ ) {
                gp_Pnt edgePoint = edgeTr->Node(j).Transformed(edgeLoc);
                edgeTessOut.append(pntWrite(edgePoint));        
              }
            }
          }
          if (!INTERROGATE_STRUCT_ONLY) {
            edgeOut["tess"] = edgeTessOut;
          }
          TopoDS_Edge* persistEdge = new TopoDS_Edge(aEdge);

          edgeOut["ptr"] = ((std::uintptr_t)persistEdge);
          edgeOut["edgeRef"] = edgeFaceMap.FindIndex(aEdge);
          edgeOut["ref"] = e0::io::getStableRefernce(aEdge);
          edgesOut.append(edgeOut);  
        }
        loopsOut.append(edgesOut);
      }        
      
      faceOut["loops"] = loopsOut;
      faceOut["inverted"] = aFace.Orientation() == TopAbs_REVERSED;
      if (!INTERROGATE_STRUCT_ONLY) {
        faceOut["tess"] = tessOut;
        // DATA evalPts = Array();
        // writeFaceEvalationPoints(aTr, fPoints, evalPts);
        // faceOut["evaluationPoints"] = evalPts;                     
      }
      TopoDS_Face* persistFace = new TopoDS_Face(aFace);
      faceOut["ref"] = e0::io::getStableRefernce(aFace);                     
      faceOut["ptr"] = ((std::uintptr_t)persistFace);
      facesOut.append(faceOut);
    } 
  }  
  out["faces"] = facesOut; 
  return out;  
}

DATA createShapePreview(const TopoDS_Shape& aShape, Standard_Real aDeflection = 3) {
  if (aDeflection <= 0) {
    aDeflection = 3;
  }
  BRepMesh_IncrementalMesh(aShape, aDeflection);  
  DATA previewOut = Array();
  TopExp_Explorer aExpFace; 
  for(aExpFace.Init(aShape,TopAbs_FACE);aExpFace.More();aExpFace.Next()) {   
    DATA faceOut = Object();
    DATA tessOut = Array();
    TopoDS_Face aFace = TopoDS::Face(aExpFace.Current());
    Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);

    TopLoc_Location aLocation;  

    Handle(Poly_Triangulation) aTr = BRep_Tool::Triangulation(aFace,aLocation);  
    
    if(!aTr.IsNull()) {        
      TColgp_Array1OfPnt fPoints(1, aTr->NbNodes()); 
      for( Standard_Integer i = 1; i < aTr->NbNodes()+1; i++) { 
        fPoints(i) = aTr->Node(i).Transformed(aLocation);  
      }
      writeFaceTessellation(aTr, aLocation, aSurface, fPoints, previewOut);
    }
  }
  return previewOut;
}

io::DATA getModelData(io::DATA request) {

  std::uintptr_t bptr = request["model"].ToInt();
  TopoDS_Shape* body = reinterpret_cast<TopoDS_Shape*>(bptr);

  io::DATA out = io::interrogate(*body);
  out["ptr"] = (std::uintptr_t) body;

  return out;
}

void dispose(io::DATA request) {

  std::uintptr_t bptr = request["model"].ToInt();
  TopoDS_Shape* body = reinterpret_cast<TopoDS_Shape*>(bptr);

  delete body;
}

void UpdateTessellation(TopoDS_Shape& shape, double deflection) {
  BRepTools::Clean(shape);  
  BRepMesh_IncrementalMesh(shape, deflection);  
}

} //io
} //e0

#endif // E0_IO_INTERROGATE_H
