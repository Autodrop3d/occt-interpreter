#ifndef E0_IO_HISTORY_IO_H
#define E0_IO_HISTORY_IO_H

#include "commonIO.hpp"
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <BRepTest_Objects.hxx>

namespace e0 {
namespace io {

    DATA writeShapeRef(const TopoDS_Shape& shape) {

        TopAbs_ShapeEnum aType = shape.ShapeType();
        Standard_CString aTypeName;
        switch (aType)
        {
        case TopAbs_SHELL: aTypeName = "SHELL"; break;
        case TopAbs_FACE: aTypeName = "FACE"; break;
        case TopAbs_WIRE: aTypeName = "WIRE"; break;
        case TopAbs_EDGE: aTypeName = "EDGE"; break;
        case TopAbs_VERTEX: aTypeName = "VERTEX"; break;
        default: aTypeName = "SOLID";
        }

        DATA out = Array();
        out.append(aTypeName);
        out.append(e0::io::getStableRefernce(shape));
        return out;
    }

    void dumpHistory(TopTools_DataMapOfShapeListOfShape& map, DATA& out) {       
        TopTools_DataMapOfShapeListOfShape::Iterator itM(map);
        for (; itM.More(); itM.Next())
        {
            const TopoDS_Shape& key = itM.Key();
            DATA node = Object();
            DATA targets = Array();
            node["source"] = writeShapeRef(key);
            const TopTools_ListOfShape& aList = itM.Value();
            TopTools_ListOfShape::Iterator itL(aList);
            for (; itL.More(); itL.Next())
            {
                const TopoDS_Shape& shape = itL.Value();
                targets.append(writeShapeRef(shape));
            }
            node["targets"] = targets;
            out.append(node);
        }
    }

    DATA productionHistoryWrite() {            
        auto h = BRepTest_Objects::History();
        DATA hist = Object();
        DATA modified = Array();
        DATA generated = Array();
        dumpHistory(h->myShapeToModified, modified);
        dumpHistory(h->myShapeToGenerated, generated);
        hist["modified"] = modified;
        hist["generated"] = generated;
        return hist;
    }
}
}

#endif // E0_IO_HISTORY_IO_H