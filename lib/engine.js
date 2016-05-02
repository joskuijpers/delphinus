
Object.defineProperty(exports, "apiLevel", {
    value: 2,
    writable: false
});

Object.defineProperty(exports, "name", {
    value: "delphinus",
    writable: false
});

Object.defineProperty(exports, "versions", {
    get: () => __delphinus.get_versions(),
    writable: false
});

Object.defineProperty(exports, "mainModule", {
    get: () => __delphinus.get_main_module(),
    writable: false
});

Object.defineProperty(exports, "extensions", {
    get: () => __delphinus.get_extensions(),
    writable: false
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
