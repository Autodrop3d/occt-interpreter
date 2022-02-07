#ifndef IOEngineCommands_H
#define IOEngineCommands_H

#include <Data.hxx>
#include <Draw_Interpretor.hxx>
#include <TopoDS_Shape.hxx>
#include <DBRep.hxx>

namespace EngineInterface {

    namespace io {

        static Standard_Integer pushModel(Draw_Interpretor& di, DATA& data) {
            std::uintptr_t modelPtr = data["operand"].ToInt();
            std::string modelName = data["name"].ToString();

            TopoDS_Shape* model = reinterpret_cast<TopoDS_Shape*>(modelPtr);

            DBRep::Set(modelName.c_str(), *model);                

            return 0;
        }


    }

}

#endif