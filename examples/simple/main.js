// let x = require("./test");
// x = require("./test");
// x = require("./test");


// console.log("Hello World from main module");
// console.warn("Hello World from main module");
// console.error("Hello World from main module");
// console.log("'" + __delphinus.get_extensions() + "'");

// parseConsoleArgs("A string ", "'%s', float: '%d'", "hello", 2.5, "yeah");
// console.log(parseConsoleArgs("A string '%s', float: '%d'", "hello", 2.5, "yeah"));
// console.log(parseConsoleArgs("A", "boem", "yeah"));

// console.log(parseConsoleArgs(["a","b"]));
// parseConsoleArgs({"a": 1,"b": 2});

// console.log("Added 1 and 2: " + x.foo(1,2));

// parseConsoleArgs("Hello");


// function parseConsoleArgs(...args) {
//     const argsLength = args.length;

//     if (argsLength === 0 || typeof args[0] !== "string") {
//         const objects = new Array(argsLength);
//         for (let index = 0; index < argsLength; ++index) {
//             objects[index] = inspect(args[index]);
//         }
//         return objects.join(" ");
//     }

//     if (argsLength === 1) {
//         return args[0];
//     }

//     let result = "";
//     let argIndex = 1;
//     let lastPos = 0;

//     const fmt = args[0];
//     for (let i = 0; i < fmt.length;) {
//         if (fmt.charCodeAt(i) === 37 /*%*/ && i + 1 < fmt.length) {
//             switch(fmt.charCodeAt(i + 1)) {
//                 case 100: // d
//                     if (argIndex >= argsLength) { // Safeguard
//                         break;
//                     }
//                     if (lastPos < i) {
//                         result += fmt.slice(lastPos, i);
//                     }

//                     result += Number(args[argIndex++]);
//                     lastPos = i = i + 2;
//                     continue;
//                 case  106: // j
//                     if (argIndex >= argsLength) { // Safeguard
//                         break;
//                     }
//                     if (lastPos < i) {
//                         result += fmt.slice(lastPos, i);
//                     }

//                     try {
//                         result += JSON.stringify(args[argIndex++]);
//                     } catch (e) {
//                         result += "[Circular]";
//                     }

//                     lastPos = i = i + 2;
//                     continue;
//                 case 115: // s
//                     if (argIndex >= argsLength) { // Safeguard
//                         break;
//                     }
//                     if (lastPos < i) {
//                         result += fmt.slice(lastPos, i);
//                     }

//                     result += String(args[argIndex++]);
//                     lastPos = i = i + 2;
//                     continue;
//                 case 37: // %
//                     if (lastPos < i) {
//                         result += fmt.slice(lastPos, i);
//                     }

//                     result += "%";
//                     lastPos = i = i + 2;
//                     continue;
//             }
//         }
//         ++i;
//     }

//     if (lastPos === 0) {
//         result = fmt;
//     } else if (lastPos < fmt.length) {
//         result += fmt.slice(lastPos);
//     }

//     // Add the rest parameters
//     while (argIndex < argsLength) {
//         const elem = args[argIndex++];

//         if (elem === null || (typeof elem !== "object" && typeof elem !== "symbol")) {
//             result += " " + elem;
//         } else {
//             result += " " + inspect(elem);
//         }
//     }

//     return result;
// }

function inspect(v) {
    return JSON.stringify(v);
}
