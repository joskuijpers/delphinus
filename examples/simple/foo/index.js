tryModule("../test"); // file in other folder
tryModule("assert"); // system package
tryModule("link"); // package in subfolder
tryModule("./bar"); // next file in same folder

function tryModule(m) {
    try {
        console.log(`${m} => `, require.resolve(m));
    } catch (e) {
        console.log(e.message);
    }
}