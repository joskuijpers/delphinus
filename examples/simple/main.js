class Game {
    constructor() {
        assert(false && "Can't create another Game object");
    }

    get name() {
        return "MyGame";
    }

    static quit(message) {
        System.delegate.willQuit(message);
        System.quit(message);

        assert(false && "Application quit");
    }

    static registerDelegate(delegateObject) {
        System.delegate = delegateObject;
    }
}

class GameDelegate {
    didLaunch() {} // nothing

    handleEvents(events) {
        for (let event of events) {
            if (event === "QuitEvent") {
                Game.quit("QuitEvent happened");
                break;
            }
        }
    }

    update() {} // nothing
    render() {} // nothing
    willQuit() {} // nothing
}

export
default class MyGameDelegate extends GameDelegate {

    didLaunch() {
        this.myScene = new Scene();
    }
    /**
     * Handle events
     */
    handleEvents(events) {
        super(events); // Optional to quit automatically.

        if (event.isSomeOtherQuitEventLikeQKey) {
            Game.quit("possible message");
        }
    }

    /**
     * Update physics, animations, anything
     */
    update() {}

    /**
     * Render
     */
    render() {
        console.log("Render");

        this.myScene.render();
    }

    willQuit(message) {
        console.log("Goodbye!");
    }
}

Game.registerDelegate(new MyMainDelegate());

/*

willLaunch? // for options?

Setup SDL/graphics engine depending on the options

delegate.didLaunch // for startup

running = true
while(running) {
    eventlist = []
    while (has events) {
        add event to list
    }
    delegate.handleEvents(eventlist)

    if(!running)
        break;

    delegate.update() // steps calculated

    ClearScreen
    delegate.render();
    FlipScreen
}

 */
