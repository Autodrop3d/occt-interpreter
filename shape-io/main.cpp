#include <iostream>

#include <emscripten.h>
#include <DBRep.hxx>
#include "interrogate.hpp"


using namespace std;
using namespace e0;

extern "C" {

  void SPI_publish_result(io::DATA res) {
    EM_ASM_({
      __E0_ENGINE_EXCHANGE(UTF8ToString($0));
    }, res.dumpJSON().c_str());
  }

  EMSCRIPTEN_KEEPALIVE
  void Interogate(const char* shapeName) {
    TopoDS_Shape shape = DBRep::Get(shapeName);
    try {
      io::DATA out = io::interrogate(shape, 2);  
      SPI_publish_result(out);
    } catch (Standard_Failure const& anException) {
      std::cout << anException.GetMessageString() << std::endl;
    }
  }

}
