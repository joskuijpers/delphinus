let x = require("./test");
x = require("./test");
x = require("./test");

console.log("Hello World from main module");
console.warn("Hello World from main module");
console.error("Hello World from main module");
console.log("'" + __delphinus.get_extensions() + "'");

console.log("Added 1 and 2: " + x.foo(1,2));