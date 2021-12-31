INC=/build/include/opencascade
# INC=/home/xibyte/webcad/cad/opencascade-7.2.0/inc
LIB=/build/lin32/clang/lib/
export LD_LIBRARY_PATH=$LIB

printf "\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

em++ \
  -I$INC -L$LIB \
  -lTKTopTest \
  -lTKBool \
  -lTKBO \
  -lTKBRep \
  -lTKernel \
  -lTKFeat \
  -lTKFillet \
  -lTKG2d \
  -lTKG3d \
  -lTKGeomAlgo \
  -lTKGeomBase \
  -lTKHLR \
  -lTKMath \
  -lTKMesh \
  -lTKOffset \
  -lTKPrim \
  -lTKShHealing \
  -lTKTopAlgo \
  -lTKXSBase \
  -lTKSTEPBase \
  -lTKSTEPAttr \
  -lTKSTEP209 \
  -lTKSTEP \
  /shape-io/main.cpp \
  -o main.html \
  -DIGNORE_NO_ATOMICS \
  -DOCC_CONVERT_SIGNALS \
  -DHAVE_LIMITS_H \
  -DHAVE_IOSTREAM \
  -DHAVE_IOMANIP \
  -DNDEBUG \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s WASM=1 \
  -std=c++0x -Wall -Wextra \
  -s LLD_REPORT_UNDEFINED \
  -s ASSERTIONS=1 \
  -s NO_DISABLE_EXCEPTION_CATCHING \
  -s EXPORTED_RUNTIME_METHODS="['setValue']" \
  --extern-post-js /scripts/call.js \
  -O2 
