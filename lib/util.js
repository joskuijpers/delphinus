//https://github.com/nodejs/node/blob/master/lib/internal/bootstrap_node.js
//https://github.com/nodejs/node/blob/v0.10/lib/module.js
//https://github.com/fatcerberus/minisphere/blob/pegasus-api/src/minisphere/api/engine_api.c
exports.repr = function (object) {
    return "SomeRepresentation"
};

exports.isArray = Array.isArray;

exports.isBoolean = function (arg) {
    return typeof arg === "boolean";
};

exports.isNull = function (arg) {
    return (arg) === null;
};

exports.isNullOrUndefined = function (arg) {
    return arg === null || arg === undefined;
};

exports.isNumber = function (arg) {
    return typeof arg === "number";
};

exports.isString = function (arg) {
    return typeof arg === "string";
};

exports.isUndefined = function (arg) {
    return arg === void 0;
};

exports.isRegExp = function (arg) {
    return isObject(arg) && objectToString(arg) === "[object RegExp]";
};

function isObject(arg) {
    return typeof arg === "object" && arg !== null;
};
exports.isObject = isObject;

exports.isDate = function (arg) {
    return isObject(arg) && objectToString(arg) === "[object Date]";
};

exports.isError = function (arg) {
    return isObject(arg) && (objectToString(arg) === "[object Error]" || arg instanceof Error);
};

exports.isFunction = function (arg) {
    return typeof arg === "function";
};

exports.isSymbol = function (arg) {
    return typeof arg === "symbol";
};

exports.isPrimitive = function (arg) {
    return arg === null || (typeof arg !== "object" && typeof arg !== "function")
};

exports.inherits = function (constructor: Function, superConstructor: Function): void {
    constructor.super_ = superConstructor;
    constructor.prototype = Object.create(superConstructor.prototype, {
        constructor: {
            value: constructor,
            enumerable: false,
            writable: true,
            configurable: true
        }
    });
};
