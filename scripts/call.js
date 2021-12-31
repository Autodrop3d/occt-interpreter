function str2C(s) {
    var size = lengthBytesUTF8(s) + 1;
    var ret = _malloc(size);
    stringToUTF8Array(s, HEAP8, ret, size);
    return ret;
}

function CallCommand(command, args) {

  let c_strings = args.map(x => str2C(x));

  let c_arr = _malloc(c_strings.length * 4); // 4-bytes per pointer
  c_strings.forEach(function (x, i) {
      Module.setValue(c_arr + i * 4, x, "i32");
  });

  const commandPtr = str2C(command);

  // invoke our C function
  let rc = Module._CallCommand(commandPtr, c_strings.length, c_arr);

  // free c_strings
  for (let i = 0; i < c_strings.length; i++)
    //   _free(c_strings[i]);

  // free c_arr
  _free(c_arr);

//   _free(commandPtr);

  // return
  return rc;
}

function Interogate(shapeName, args) {
  const shapeNamePtr = str2C(shapeName);
  Module._Interogate(shapeNamePtr);
  _free(shapeNamePtr);
}


window.__OCI_EXCHANGE_VAL = null;
window.__OCI_EXCHANGE = function(objStr) {
  __OCI_EXCHANGE_VAL = JSON.parse(objStr);
  console.log("EXCHANGE VALUE:");
  console.log(__OCI_EXCHANGE_VAL);
};