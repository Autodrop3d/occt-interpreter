docker run -it \
-v $(pwd)/build-wasm:/build \
-v $(pwd)/occt:/occt \
-v $(pwd)/shape-io:/shape-io \
-v $(pwd)/scripts:/scripts \
xibyte/occt:wasm-builder_1.0 -i