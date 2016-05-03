
Object.defineProperty(exports, "apiLevel", {
    value: 2,
    writable: false,
    configurable: false,
    enumerable: true
});

Object.defineProperty(exports, "name", {
    value: "delphinus",
    writable: false,
    configurable: false,
    enumerable: true
});

Object.defineProperty(exports, "versions", {
    get: () => __delphinus.get_versions(),
    configurable: false,
    enumerable: true
});

Object.defineProperty(exports, "extensions", {
    get: () => __delphinus.get_extensions(),
    configurable: false,
    enumerable: true
});

exports.abort = function(msg) {
    __delphinus.exit(msg, true);
};

exports.exit = function() {
    __delphinus.exit(msg, false);
};

exports.restart = function() {
    __delphinus.restart();
};

exports.sleep = function(time) {
    __delphinus.sleep(time * 1000.0);
};
