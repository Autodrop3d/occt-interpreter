# occt-interpreter

occt version - https://git.dev.opencascade.org/gitweb/?p=occt.git;a=snapshot;h=80ffc5f84dae96de6ed093d3e5d2466a9e368b27;sf=tgz

This is the build environment for the version of OpenCASCADE that the JSketcher project uses. 
It includes the wrapper and javascript API extensions developed to work within the JSketcher project.
See https://github.com/xibyte/jsketcher for how it is used.  

NPM package published at https://www.npmjs.com/package/jsketcher-occ-engine


# Instructions to set up dev environment
Clone repo on to a linux machine that has docker installed. 

First run the docker build script. This only needs to be run the first time you use the sandbox.
```
./scripts/docker-build.sh

```

Next start the docker sandbox environnement
```
./scripts/docker-sandbox.sh
```

The docker sandbox environment will start and present you with a standard linux shell environment. 
The scripts folder is automatically mounted to `/scripts` with in the docker dev sandbox. 

Within the docker sandbox you will need to run the `init-cmake.sh`. This only needs to be run the first time you use the sandbox.
```
/scripts/init-cmake.sh
```

Compile the project using the `compile.sh` script. 

The first time you run the compile script it will take a while to compile.
```
/scripts/compile.sh
```



Finally run `wasm-link.sh` to produce the wasm build. 
```
/scripts/wasm-link.sh
```

You should now find the wasm file located in the build-wasm directory.
