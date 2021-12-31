#include <Standard_Macro.hxx>
#include <Standard_TypeDef.hxx>
#include <map>
#include <string.h>

#ifndef _MyMap_HeaderFile
#define _MyMap_HeaderFile

struct CstrCmp {
    bool operator()(const char* a, const char* b) const {
      return strcmp(a, b) < 0;
    }
};



  template <typename T>    
    using CStringMap = std::map<Standard_CString, T, CstrCmp>;    


#endif