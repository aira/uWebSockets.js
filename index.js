const uWS = require('./build/Release/aira-uweb.node');

module.exports = (() => {
    try {
        process.nextTick = (f, ...args) => {
            Promise.resolve().then(() => {
                f(...args);
            });
        };
        process.on('exit', uWS.free);
        return uWS;
    } catch (e) {
        throw new Error('This version of µWS is not compatible with your Node.js build:\n\n' + e.toString());
    }
})();
