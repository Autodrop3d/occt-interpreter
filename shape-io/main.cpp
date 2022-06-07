#include <iostream>

#include <emscripten.h>
#include <DBRep.hxx>
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
  bool IsEdgesOverlap(int e1Ptr, int e2Ptr, double tol) {
    TopoDS_Edge* e1 = reinterpret_cast<TopoDS_Edge*>(e1Ptr);
    TopoDS_Edge* e2 = reinterpret_cast<TopoDS_Edge*>(e2Ptr);
    
    return e0::isEdgesOverlap(*e1, *e2, tol);
  }

  void UpdateTessellation(int shapePtr, double deflection) {
    TopoDS_Shape* shape = reinterpret_cast<TopoDS_Shape*>(shapePtr);
    e0::io::UpdateTessellation(*shape, deflection);
  }

}
