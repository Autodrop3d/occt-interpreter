#ifndef E0_CLASSIFY_H
#define E0_CLASSIFY_H

#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <TopoDS_Shape.hxx>
#include <ShapeAnalysis_Edge.hxx>

namespace e0 {

  static const int GEOM_CLASSIFICATION_UNRELATED = 0;
  static const int GEOM_CLASSIFICATION_INSIDE = 1;
  static const int GEOM_CLASSIFICATION_BOUNDS = 2;

  static const int LOGICAL_CLASSIFICATION_UNRELATED = 0;
  static const int LOGICAL_CLASSIFICATION_SAME = 1;
  static const int LOGICAL_CLASSIFICATION_PARTIAL = 2;

  int classifyPointToFace(const TopoDS_Face& face, const gp_Pnt& p3d, float tol = -1) {
    Handle(Geom_Surface) surf = BRep_Tool::Surface(TopoDS::Face(face));
    if (tol < 0) {
        tol = BRep_Tool::Tolerance(face);        
    }
    
    GeomAPI_ProjectPointOnSurf proj(p3d, surf);

    if (proj.LowerDistance() <= tol) {
        Standard_Real u, v;
        proj.LowerDistanceParameters(u,v);
        BRepClass_FaceClassifier classifier;
        const gp_Pnt2d uv(u, v);
        classifier.Perform(face, uv, tol);
        switch (classifier.State()) {
          case TopAbs_IN: return GEOM_CLASSIFICATION_INSIDE;
          case TopAbs_ON: return GEOM_CLASSIFICATION_BOUNDS;
          case TopAbs_OUT: 
          case TopAbs_UNKNOWN: 
          default:
            return GEOM_CLASSIFICATION_UNRELATED;
        }
    } else {
      return GEOM_CLASSIFICATION_UNRELATED;    
    }    
  }

  int classifyFaceToFace(const TopoDS_Face& face1, const TopoDS_Face& face2, float tol = -1) {

    if (tol < 0) {
        tol = BRep_Tool::Tolerance(face1);        
    }

    TopLoc_Location aLocation;  
    Handle(Poly_Triangulation) aTr = BRep_Tool::Triangulation(face2, aLocation);  

    if(aTr.IsNull()) {  
      std::cout << "classifyFaceToFace: No triangulation found" << std::endl;
      return GEOM_CLASSIFICATION_UNRELATED;
    }

    Handle(Geom_Surface) surface = BRep_Tool::Surface(face2);

    const Poly_Array1OfTriangle& triangles = aTr->Triangles();  
    Standard_Integer nnn = aTr->NbTriangles(); 
    Standard_Integer nt,n1,n2,n3; 

    bool wasMatch = false;
    bool wasMissMatch = false;

    for( nt = 1 ; nt < nnn+1 ; nt++) { 
      // takes the node indices of each triangle in n1,n2,n3: 
      triangles(nt).Get(n1,n2,n3); 

      gp_Pnt2d uv1 = aTr->UVNode(n1);
      gp_Pnt2d uv2 = aTr->UVNode(n1);
      gp_Pnt2d uv3 = aTr->UVNode(n1);

      gp_Pnt2d centroidUV = gp_Pnt2d((uv1.X()+uv2.X()+uv3.X())/3, (uv1.Y()+uv2.Y()+uv3.Y())/3);

      gp_Pnt evalPoint = surface->Value(centroidUV.X(), centroidUV.Y()).Transformed(aLocation);

      auto pfClassification = classifyPointToFace(face1, evalPoint, tol);

      switch (pfClassification) {
        case GEOM_CLASSIFICATION_BOUNDS:
        case GEOM_CLASSIFICATION_INSIDE:
          wasMatch = true;
          break;
        case GEOM_CLASSIFICATION_UNRELATED:
        default:
          wasMissMatch = true;
      }
    }   

    if (wasMatch && !wasMissMatch) {
      return LOGICAL_CLASSIFICATION_SAME;
    } else if (wasMatch && wasMissMatch) {
      return LOGICAL_CLASSIFICATION_PARTIAL;
    } else {
      return LOGICAL_CLASSIFICATION_UNRELATED;
    }
  }

  bool isEdgesOverlap(const TopoDS_Edge& e1, const TopoDS_Edge& e2, double tol = -1, double domainDist = 0.0) {
      if (tol < 0) {
        tol = BRep_Tool::Tolerance(e1);        
      }
      ShapeAnalysis_Edge sae;
      return sae.CheckOverlapping(e1, e2, tol, domainDist);
  } 
}

#endif // E0_CLASSIFY_H
