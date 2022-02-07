#ifndef TopoEngineCommands_H
#define TopoEngineCommands_H

#include <Data.hxx>
#include <Draw_Interpretor.hxx>


namespace EngineInterface {

    namespace topo {

        static Standard_Integer echo(Draw_Interpretor& di, DATA& data) {
            std::cout << data << "\n";
            return 0;
        }

    }

}

#endif