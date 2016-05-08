require("./foo"); // More tests
const link = require("link");

exports.test = "hello";
console.log("exports '",exports,"', '",module,"'");

module.exports = { "foo": "bar" };
console.log("exports '",exports,"', '",module,"'");


let x = require("./test");

console.log("Added 1 and 2: " + x.foo(1,2));
console.log("The extensions: %s", engine.extensions);


/*
let running = true;
while (running) {
    // Handle events
    let events = __delphinus.do_events();

    if (events == -1) {
        running = false;
        break;
    }

    // Flip screen
}
*/
