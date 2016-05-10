const util = require("util");

const assert = module.exports = ok;

class AssertionError extends Error {
    constructor(options) {
        if (typeof options == "string") {
            options = {
                message: options
            };
        }
        options.message = options.message || "Default message";

        super(options.message);
        this.name = "AssertionError";
        this.message = options.message;
        this.stack = (new Error()).stack;

        if (options.stackStart) {
            let frames = this.stack.split("\n");
            for (let i = 0; i < frames.length; ++i) {
                if (frames[i].indexOf(options.stackStart) === 0) {
                    this.stack = frames.splice(i + 1).join("\n");

                    // format: <function>@<file>:<line>:<col>
                    let frame = frames[i];
                    let split1 = frame.split("@"); // 0 = function
                    let split2 = split1[1].split(":"); // 0 = file, 1 = line, 2 = col

                    this.fileName = split2[0];
                    this.lineNumber = parseInt(split2[1]);
                    this.columnNumber = parseInt(split2[2]);

                    break;
                }
            }
        }
    }
}
assert.AssertionError = AssertionError;

function fail(actual, expected, message, operator, stackStart) {
    // throw new Error(message);
    // throw new AssertionError(message);
    throw new assert.AssertionError({
        message: message,
        actual: actual,
        expected: expected,
        operator: operator,
        stackStart: stackStart,
    });
}
assert.fail = fail;

function ok(guard, message) {
    if (!guard) {
        fail(guard, true, message, "==", "ok");
    }
}
assert.ok = ok;

assert.equal = function (actual, expected, message) {
    if (actual != expected) {
        fail(actual, expected, message, "==", "assert.equal");
    }
};

assert.notEqual = function (actual, expected, message) {
    if (actual == expected) {
        fail(actual, expected, message, "!=", "assert.notEqual");
    }
};

assert.deepEqual = function (actual, expected, message) {
    if (!deepEqual(actual, expected)) {
        fail(actual, expected, message, "deepEqual", "assert.deepEqual");
    }
};

assert.notDeepEqual = function (actual, expected, message) {
    if (deepEqual(actual, expected)) {
        fail(actual, expected, message, "notDeepEqual", "assert.notDeepEqual");
    }
};

function deepEqual(actual, expected) {
    // 1) All identical values are equivalent, as determined by ===.
    if (actual === expected) {
        return true;
    }

    // 2) If the expected value is a Date object, the actual value is equivalent
    //    if it is also a Date object that refers to the same time.
    else if (util.isDate(actual) && util.isDate(expected)) {
        return actual.getTime() === expected.getTime();
    }

    // If the expected value is a RegExp, the value and flags must match.
    else if (util.isRegExp(actual) && util.isRegExp(expected)) {
        return actual.source === expected.source &&
            actual.global === expected.global &&
            actual.multiline === expected.multiline &&
            actual.lastIndex === expected.lastIndex &&
            actual.ignoreCase === expected.ignoreCase;
    }

    // 3) Other pairs that do not both pass typeof value == "object", equivalence is determined by ==.
    else if ((actual === null || typeof actual !== "object") && (expected === null || typeof expected !==
        "object")) {
        return actual == expected; // For strict, use ==.
    }

    // 4) or all other Object pairs, including Array objects, equivalence is determined by having the same
    //    number of owned properties (as verified with Object.prototype.hasOwnProperty.call), the same set
    //    of keys (although not necessarily the same order), equivalent values for every corresponding key,
    //    and an identical "prototype" property. Note: this accounts for both named and indexed properties on Arrays.
    else {
        return objectEquivalence(actual, expected);
    }

}

function objectEquivalence(a, b) {
    if (a === null || a === undefined || b === null || b === undefined) {
        return false;
    }

    if (util.isPrimitive(a) || util.isPrimitive(b)) {
        return a === b;
    }

    // ...
}

assert.strictEqual = function (actual, expected, message) {
    if (actual !== expected) {
        fail(actual, expected, message, "===", "assert.strictEqual");
    }
};

assert.notStrictEqual = function (actual, expected, message) {
    if (actual === expected) {
        fail(actual, expected, message, "!==", "assert.notStrictEqual");
    }
};

assert.throws = function (block, error, message) {

};

assert.doesNotThrow = function (block, error, message) {

};
