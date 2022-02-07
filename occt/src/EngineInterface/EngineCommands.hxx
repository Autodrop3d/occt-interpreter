#ifndef EngineCommands_H
#define EngineCommands_H

#include <Draw_Interpretor.hxx>
#include <TopoEngineCommands.hxx>>
#include <IOEngineCommands.hxx>>
#include <Data.hxx>

namespace EngineInterface {

  typedef Standard_Integer (*EngineFunction )(Draw_Interpretor& di,
                                               DATA& data);


    static std::map<std::string, EngineFunction> functions;

    static int EngineCommand(Draw_Interpretor& di, Standard_Integer n, const char** a) {

        if (n != 2) {
            di << "Invalid number of arguments" << "\n";
            return 0;
        }

        std::string method(a[0]);
        
        DATA data = DATA::Load( a[1] ) ;

        auto func = functions[method];

        return func(di, data);
    }

    static void Init(Draw_Interpretor& interpretor) {
        interpretor.Add("EngineCommand", "Generic webcad engine command",
            __FILE__, EngineCommand, "webcad engine");

        functions["topo.echo"] = EngineInterface::topo::echo;       
        functions["io.pushModel"] = EngineInterface::io::pushModel;       

    }
}

#endif