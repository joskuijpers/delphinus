//let x = require("./test");

//const assert = require("assert");
//assert(5,1+4,"Samething");

tryModule("./test"); // without extension
tryModule("./test.js"); // with extension
tryModule("../test"); // sub folder (fails here)
tryModule("assert"); // system module
tryModule("link"); // package
tryModule("./foo"); // folder with index file
tryModule("./foo/bar"); // folder with other file

require("./foo"); // More tests

function tryModule(m) {
    try {
        console.log(`${m} => `, require.resolve(m));
    } catch (e) {
        console.log(e.message);
    }
}


//console.log("Added 1 and 2: " + x.foo(1,2));
console.log("The extensions: %s", engine.extensions);
