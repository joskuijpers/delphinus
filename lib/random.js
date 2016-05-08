const precondition = require("assert").ok;
const util = require("util");

const rand = Math.random;
const rand31 = () => (rand() * 0x7fffffff) | 0;

exports.seed = function (seedValue) {
};

/**
 * Run a p-value dice roll.
 *
 * @param  {Number} odds p-value [0.0-1.0]
 * @return {Boolean}     True when the odd was in your favor
 */
exports.chance = function (odds) {
    precondition(util.isNumber());
    precondition(odds >= 0.0 && odds <= 1.0);

    return odds > rand; // real2
};

let normal_s_y, normal_s_have_y = false;

/**
 * Calculate a random normal variable.
 *
 * @param  {Number} mean  Mean
 * @param  {Number} sigma Sigma, must be >= 0.0
 * @return {Number}       Random
 */
exports.normal = function (mean, sigma) {
    precondition(util.isNumber(mean));
    precondition(util.isNumber(sigma));
    precondition(sigma >= 0.0);

    let u, v, w, x;

    // We calculate two normal variables at once so
    // we can use the second one in the next call.
    if (!normal_s_have_y) {
        do {
            u = 2.0 * rand() - 1.0; // genrand_rand53
            v = 2.0 * rand() - 1.0;
            w = u * u + v * v;
        } while (w >= 1.0);

        w = Math.sqrt(-2 * Math.log(w) / w);
        x = u * w;
        normal_s_y = v * w;
        normal_s_have_y = true;
    } else {
        x = normal_s_y;
        normal_s_have_y = false;
    }

    return mean + x * sigma;
};

exports.random = function () {
    return rand();
};

/**
 * Get a ranged random value.
 *
 * @param  {Number} min Lower end of range
 * @param  {Number} max Upper end of range, >= min
 * @return {Number}     Random value within range
 */
exports.integer = function (min, max) {
    precondition(max >= min);

    const range = Math.abs(max - min) + 1;
    return min + rand31() % range; // int31
};

/**
 * Grab a value from given array randomly with uniform distribution.
 *
 * @param  {any[]} array Array of elements
 * @return {any}       Value or undefined if array is empty.
 */
exports.sample = function (array) {
    precondition(util.isArray(array));

    if (array.length === 0) {
        return undefined;
    }

    return array[this.integer(0, array.length - 1)];
};

/**
 * Create a random string with number, lower- and uppercase characters (radius 62)
 *
 * @param  {Number} length Length of the string, >= 0
 * @return {Number}
 */
exports.string = function (length) {
    return exports.uuid(length, 62);
};

/**
 * Random uniform variable
 *
 * @param  {Number} average  Average
 * @param  {Number} variance Variance, >= 0.0
 * @return {Number}
 */
exports.uniform = function (average, variance) {
    precondition(util.isNumber(average));
    precondition(util.isNumber(variance));
    precondition(variance >= 0.0);

    const diff = variance * 2.0 * (0.5 - rand()); // genrand_real2

    return average + diff;
};

const UUID_CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwzyz+/";
exports.uuid = function (length, radix) {
    precondition(util.isNullOrUndefined(length) || (util.isNumber(length) && length >= 0.0));
    precondition(util.isNullOrUndefined(radix) || (util.isNumber(radix) && radix >= 2.0 && radix <= 64));

    let uuid = [];
    radix = radix || UUID_CHARS.length;

    if (util.isNullOrUndefined(length)) {
        uuid[8] = uuid[13] = uuid[18] = uuid[23] = "-";
        uuid[14] = "4"; // version 4

        // 16 octets = 32 characters + 4 dashes
        for (let i = 0; i < 36; ++i) {
            if (uuid[i] === undefined) {
                const r = 0 | rand() * 16;
                uuid[i] = UUID_CHARS[(i === 19) ? (r & 0x3) | 0x8 : r & 0xf];
            }
        }
    } else {
        // Generate a random string
        for (let i = 0; i < length; ++i) {
            uuid[i] = UUID_CHARS[0 | rand() * radix];
        }
    }

    return uuid.join("");
};
