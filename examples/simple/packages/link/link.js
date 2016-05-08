
console.log("Hello World from LINK module");

let link = require.resolve("lodash");
console.log("resolved link from", __dirname, "to", link);

exports.foo = function(a, b) {
    return a + b;
};
