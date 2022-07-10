#include <iostream>

#include <emscripten.h>
#include <DBRep.hxx>
#include <gp_Trsf.hxx>
#include "interrogate.hpp"
#include "historyIO.hpp"
#include "classify.hpp"


using namespace std;
using namespace e0;

extern "C" {

  void SPI_publish_result(io::DATA res) {
    EM_ASM_({
      __OCI_EXCHANGE(UTF8ToString($0));
    }, res.dumpJSON().c_str());
  }

  EMSCRIPTEN_KEEPALIVE
  void Interogate(const char* shapeName, bool structOnly = false) {
    TopoDS_Shape shape = DBRep::Get(shapeName);
    try {
      io::DATA out = io::interrogate(shape, 2, structOnly);  
      TopoDS_Shape* shapePtr = new TopoDS_Shape(shape);
      out["ptr"] = (std::uintptr_t) shapePtr;
      SPI_publish_result(out);
    } catch (Standard_Failure const& anException) {
      std::cout << anException.GetMessageString() << std::endl;
    }
  }

  EMSCRIPTEN_KEEPALIVE
  void GetProductionHistory() {
    io::DATA out = io::productionHistoryWrite();
    SPI_publish_result(out);
  }

  EMSCRIPTEN_KEEPALIVE
  std::uintptr_t GetRef(const char* shapeName) {
    TopoDS_Shape shape = DBRep::Get(shapeName);
    return e0::io::getStableRefernce(shape);
  }

  EMSCRIPTEN_KEEPALIVE
  int ClassifyPointToFace(int facePtr, int x, int y, int z, double tol) {
    TopoDS_Face* face = reinterpret_cast<TopoDS_Face*>(facePtr);
    gp_Pnt p3d(x, y, z);
    return e0::classifyPointToFace(*face, p3d, tol);
  }

  EMSCRIPTEN_KEEPALIVE
  int ClassifyFaceToFace(int face1Ptr, int face2Ptr, double tol) {
    TopoDS_Face* f1 = reinterpret_cast<TopoDS_Face*>(face1Ptr);
    TopoDS_Face* f2 = reinterpret_cast<TopoDS_Face*>(face2Ptr);
    return e0::classifyFaceToFace(*f1, *f2, tol);
  }

  EMSCRIPTEN_KEEPALIVE
  int ClassifyEdgeToFace(int edgePtr, int facePtr, double tol) {
    TopoDS_Edge* e = reinterpret_cast<TopoDS_Edge*>(edgePtr);
    TopoDS_Face* f = reinterpret_cast<TopoDS_Face*>(facePtr);
    return e0::classifyEdgeToFace(*e, *f, tol);
  }

  EMSCRIPTEN_KEEPALIVE
  bool IsEdgesOverlap(int e1Ptr, int e2Ptr, double tol) {
    TopoDS_Edge* e1 = reinterpret_cast<TopoDS_Edge*>(e1Ptr);
    TopoDS_Edge* e2 = reinterpret_cast<TopoDS_Edge*>(e2Ptr);
    
    return e0::isEdgesOverlap(*e1, *e2, tol);
  }

  EMSCRIPTEN_KEEPALIVE
  void UpdateTessellation(int shapePtr, double deflection) {
    TopoDS_Shape* shape = reinterpret_cast<TopoDS_Shape*>(shapePtr);
    e0::io::UpdateTessellation(*shape, deflection);
  }

  EMSCRIPTEN_KEEPALIVE
  void SetLocation(const char* shapeName, float mx0, float mx1, float mx2, float mx3, float mx4, float mx5, float mx6, float mx7, float mx8, float mx9, float mx10, float mx11) {
    try {

      std::cout << "Getting " << shapeName << std::endl;

      TopoDS_Shape shape = DBRep::Get(shapeName);

      std::cout << "Got shape " << std::endl;
      
      gp_Trsf trfs;

      trfs.SetValues(
        mx0,
        mx1,
        mx2,
        mx3,
        mx4,
        mx5,
        mx6,
        mx7,
        mx8,
        mx9,
        mx10,
        mx11
      );
      trfs.SetScaleFactor(1);

      std::cout << "Creating location object " << std::endl;
      TopLoc_Location loc(trfs);

      std::cout << "Setting location " << std::endl;

      shape.Location(loc);

      std::cout << "Writing shape back " << shapeName << std::endl;
      DBRep::Set(shapeName, shape);

    } catch (Standard_Failure const& anException) {
      std::cerr << "ERROR: " << anException.GetMessageString() << std::endl;
    }
  }

}
