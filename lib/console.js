let print = require("engine")._print;


export.log = function (...args) {
    print("Print something to the console\n");
};
