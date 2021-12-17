FROM emscripten/emsdk:2.0.30

RUN \
  apt update -y && \
  apt install -y \
    bash \
    build-essential \
    cmake \
    curl \
    git \
    libffi-dev \
    libgdbm-dev \
    libncurses5-dev \
    libnss3-dev \
    libreadline-dev \
    libsqlite3-dev \
    libssl-dev \
    libbz2-dev \
    npm \
    python3 \
    python3-pip \
    python3-setuptools \
    zlib1g-dev

COPY occt ./occt

RUN mkdir /build/ 
WORKDIR /build/
RUN emcmake cmake \
  -DCMAKE_SUPPRESS_REGENERATION:BOOL=ON  \
  -DBUILD_USE_PCH:BOOLEAN=OFF \
  -DUSE_TBB:BOOLEAN=OFF \
  -DBUILD_LIBRARY_TYPE=Static \
  -DBUILD_SHARED_LIBS:BOOL=OFF \
  -DBUILD_TESTING:BOOLEAN=OFF \
  -DBUILD_MODULE_ApplicationFramework:BOOLEAN=OFF \
  -DBUILD_MODULE_DataExchange:BOOLEAN=ON \
  -DBUILD_MODULE_Draw:BOOLEAN=OFF \
  -DBUILD_MODULE_FoundationClasses:BOOLEAN=ON \
  -DBUILD_MODULE_MfcSamples:BOOLEAN=OFF \
  -DBUILD_MODULE_ModelingAlgorithms:BOOLEAN=ON \
  -DBUILD_MODULE_ModelingData:BOOLEAN=ON \
  -DBUILD_MODULE_Visualization:BOOLEAN=OFF \
  ../occt

RUN emmake make -j16  
