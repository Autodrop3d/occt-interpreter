#ifndef E0_CRAFT_STEP_H
#define E0_CRAFT_STEP_H

#include <BRepTools.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <STEPControl_Reader.hxx>

extern "C" {
  
EMSCRIPTEN_KEEPALIVE
int ImportStepFile(const char* shapeName, const char* fileName, bool oneOnly) {

  STEPControl_Reader reader;
  
  std::cout << "reading step file " << fileName << std::endl;
  IFSelect_ReturnStatus stat = reader.ReadFile(fileName);

  std::cout << "step file has read: " << std::endl;

  Standard_Integer NbRoots = reader.NbRootsForTransfer();
  Standard_Integer num = reader.TransferRoots();

  std::cout << "number of roots: " << NbRoots << std::endl;
  std::cout << "transfered: " << num << std::endl;

  if (oneOnly) {
    DBRep::Set(shapeName, reader.OneShape());  
    return -1;      
  } else {    
    int nbShapes = reader.NbShapes();
    int shapeNameLen = strlen(shapeName);
    char genname[shapeNameLen + 9];
    strcpy(genname, shapeName);
    char* p = genname;
    p += strlen(shapeName);
    *p = '_';
    p++;
    Standard_Integer i = 0;
    for (int i = 1; i <= nbShapes && i < 1e6; ++i) {
        Sprintf(p,"%d",i);        
        DBRep::Set(genname, reader.Shape(i));        
    }
    DBRep::Set(shapeName, reader.OneShape());        
    return nbShapes;
  }
}

}

#endif // E0_CRAFT_STEP_H
