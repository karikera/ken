
import * as childProcess from 'child_process';

function runScript(scriptPath:string, callback:(err:Error)=>void) {

    // keep track of whether callback has been invoked to prevent multiple invocations
    var invoked = false;

    var process = childProcess.fork(scriptPath);

    // listen for errors as they may prevent the exit event from firing
    process.on('error', err=>{
        if (invoked) return;
        invoked = true;
        callback(err);
    });

    // execute the callback once the process has finished running
    process.on('exit', code=>{
        if (invoked) return;
        invoked = true;
        var err = code === 0 ? null : new Error('exit code ' + code);
        callback(err);
    });
}

// Now we can run a script and invoke a callback when complete, e.g.
runScript('./node_modules/glsl-minifier/glsl-minifier.js', err=>{
    if (err) throw err;
    console.log('finished running some-script.js');
});
