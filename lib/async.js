

function RunLoop() {
    let running = true;

    let previous = getCurrentTime();
    let lag = 0.0;

    while(running) {
        let current = getCurrentTime();
        let elapsed = current - previous;
        previous = current;
        lag += elapsed;

        /*
         Do all the polling:
         
         (Input)Events:
            Poll events
            Add handler callbacks to the loop
        
         Networking events:
            Poll sockets on new connections and new data
            Add events to the loop

         */

        const startTime = engine.hrTime();
        // Handle events for max X time
        do {
            for(do 5) {
                handleEvent()
            }
        } while(engine.hrTime() < startTime + THRESHOLD) // two loops to not call HrTime too often

        // Update loop
            while (lag >= MS_PER_UPDATE) {
                update();

                lag -= MS_PER_UPDATE;
            }

        // Render
        render(lag / MS_PER_UPDATE);
    }
}


function NotificationCenter() {


    static sharedMain() {

    }

    pushEvent(Event)

    registerForEvent(EventType, Function)
}

function SocketIO {

}