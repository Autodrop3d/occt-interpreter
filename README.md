# occt-interpreter

occt version - https://git.dev.opencascade.org/gitweb/?p=occt.git;a=snapshot;h=80ffc5f84dae96de6ed093d3e5d2466a9e368b27;sf=tgz

This is the build environment for the version of OpenCASCADE that the JSketcher project uses. 
It includes the wrapper and javascript API extensions developed to work within the JSketcher project.
See https://github.com/xibyte/jsketcher for how it is used.  

NPM package published at https://www.npmjs.com/package/jsketcher-occ-engine

# docker commands

docker build -t xibyte/occt:wasm-builder_1.0 .

docker run -it -v $(pwd)/build-wasm:/build -v $(pwd)/occt:/occt xibyte/occt:wasm-builder_1.0 -i
