#ifndef GeomEngineCommands_H
#define GeomEngineCommands_H

#include <Data.hxx>
#include <Draw_Interpretor.hxx>


namespace EngineInterface {

    namespace geom {

        static Standard_Integer arc(Draw_Interpretor& di, DATA& data) {
            
            Handle(Geom_TrimmedCurve) anArcOfCircle  = GC_MakeArcOfCircle(pntRead(data["a"]), 
                                                                        vecRead(data["tangent"]), 
                                                                        pntRead(data["b"]));
            
            return 0;
        }

    }

}

#endif