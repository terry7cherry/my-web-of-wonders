# Frontend Guide

The frontend should work to some extent out of the box. Just run the windows-frontend or macos-frontend build
configuration and point your browser (Chrome recommended) to http://localhost:5173.

If you have created a new Event class in the backend with a toJson method, the frontend will give a warning in
the ticker when this event occurs. If you look in the console (press F12 to display the console), you will
find instructions on how to configure behaviour for this event. More information on this is given in the
Behaviour section below.

If you need to customise the UI by adding or editing elements, look at the UI section below.

**NOTE:** Any config data and variables should be loaded from the maths spreadsheet via the backend math
config API and **not** hardcoded in the frontend. Instructions for how to do this are given in the Math Config
section at the end of this document.

## Javascript basics

Below are a few quick javascript concepts that may be helpful if you are new to javascript and more familiar
with C++.

### Objects and classes

Although new versions of javascript have introduced classes, they are mainly syntactic sugar, and are not
needed to create or edit objects.

```js
// Create an object with properties
const myObject = {
    property1: "value1",
    property2: "value2"
}

// Add other properties.
myObject.property3 = "value3";

// Create an empty object.
const myEmptyObject = {};
```

Even if an object was created from a class, you can still add new properties to it that were not in the
original class in just the same way demonstrated above.

### Passing arguments to functions

Unlike in C++ where you control whether arguments are passed by value or reference, in JavaScript, this
follows a rule:

- Primitives (numbers, strings, booleans, null, undefined), are passed by value. This means a copy is made and
  changing the argument within the function does not modify the original variable.
- All other types (including object, arrays, and functions) are passed by reference. This means that changing
  the variable within the function will also change the original variable.

### Functions are objects

In JavaScript, all functions are objects. This means you can assign them to variables, store them as
properties of other objects, pass them as arguments to other functions, and return them from other functions.

### Arrow functions

A class method in JavaScript looks like this:

```js
myMethod(argument1, argument2)
{
    // Logic
    return "return value"; // (optional)
}
```

Note that unlike the C++ there is not need to specify the types of the arguments or return value.

Functions outside classes usually look like this:

```js
function myFunction(argument1, argument2) {
    // Logic
    return "return value"; // (optional)
}
```

There is also another type of function in JavaScript called an arrow function. They look like this:

```js
const myArrowFunction = (argument1, argument2) => {
    // Logic
    return "return value";
}
```

If only a single line of logic is needed, they can look like this:

```js
const myArrowFunction = argument1 => "return value";
// (Note, the brackets are unnecessary for a single argument.
```

They differ from other functions in a subtle way that has to do with the "this" keyword. You shouldn't need to
worry about this for the purposes of the frontend though.

## Behaviour

### srcFrontend/src/thisTitle/events.js

This file contains the list of events that the frontend will need to display. Familiarise yourself with the
existing events to understand how to adjust them or add your own.

When adding events to map, the key for your event should be the eventType property specified in the toJson
method of your backend Event class.

The value should be an async function with two arguments, "eventData" (the JSON object returned by the Event's
toJson method) and "game" (the object representing the frontend game app, which you will use to manipulate the
UI.)

```js
export default {
    // ...,
    "myEvent": async (eventData, game) => {
        // Do something with eventData and game here.
    }
}
```

You may notice that the existing events have a list of properties in curly brackets in place of "eventData".
E.g.

```js
export default {
    "winAnalysis":
        async ({totalWin, wins}, game) => {
            //...
        }
}
```

This is just JavaScript shorthand for:

```js
export default {
    "winAnalysis":
        async (eventData, game) => {
            const totalWin = eventData.totalWin;
            const wins = eventData.wins;
            //...
        }
}
```

(It's called destructuring.)

### async/await

The async and await keywords are used in JavaScript to handle asynchronous operations (operations that take
time, like animations). This can be a bit confusing if you're new to JavaScript and you may want to a watch a
YouTube video or two, but here are the key points you need to know to use the frontend:

- If you want to wait for an animation to finish before continuing, put await before the function call. E.g.

  ```await game.ui.currentPhase.grid.spinReels(reelSymbolArray);```
- If you want to run multiple animations at the same time, use await Promise.all([...]). See the "winAnalysis"
  event
  handler for an example.
- Your event handlers will typically have at least one await statement, unless they only involve instant
  operations (like updating the session stats or adding something to the history text box).

### `game` components

The game object has a number of properties that you can use to interact with the frontend in your event
handlers:

- `game.ui`: The main game UI within the PIXI app. (That is, everything to the left of
  the blue session stats box and above the two scroll boxes at the bottom of the page.) It includes:
    - `game.ui.currentPhase`: A reference to the UI of the phase being played right now. (Includes the grid,
      and
      any added top, bottom, or side panels for the phase, as well as balance or spins left tickers for the
      phase). You will need:
        - `game.ui.currentPhase.balance`: You must update the phase balance when wins are awarded with
          `game.ui.currentPhase.balance.increment(winnings)`. At the end of the phase this balance will be
          compared with the phase winnings sent by the server and an error shown if they do not match. This
          helps to catch bugs.
        - `game.ui.currentPhase.spinsLeft`: You can update the number of spins left for the current phase with
            - `game.ui.currentPhase.spinsLeft.update(numSpinsLeft)` or
            - `game.ui.currentPhase.spinsLeft.increment(-1)`.
        - `game.ui.currentPhase.grid`: The grid object has the following methods:
            - `spinReels(reelSymbolArray)`: Triggers a spin animation landing the passed symbols. You can pass
              an options object containing any of the following properties:
                - `spinAll`: Relevant for hold and spin. Ignore locked symbols and spin all mini-reels.
                - `stickyReels`: Array of reel indexes that should not be spun.
                - `stickyPositions`: Array of {reel, row} objects representing sticky positions.
                - `textOverlays`: Text overlays to add to symbols before they spin in. 2d array of either
                  undefined (no overlay), string (overlay with standard formatting), or {text, textStyle}
                  objects for formatting options.
                - `frames`: Add frames to certain symbols before spinning in. 2d array of either undefined,
                  boolean (true for default frame), or {colour, width} objects for frame options.
                - `backgrounds`: Add backgrounds to certain symbols before spinning in. 2d array of either
                  undefined or colour string.
            - `updateGrid(reelSymbolArray)`: Compares the passed symbols to the current grid and updates
              only the changed symbols using a scrolling animation.
            - `animateWins(wins)`: Animates the wins from a WinAnalysisEvent (or similar) using appropriate
              animations for lines, scatters, ways etc.
            - `transformSymbol(reelNum, symbolNum, newSymbolName, speed)`: Changes a single symbol with an
              animation played as the specified (optional) `speed`.
            - `addBackgroundToSymbol(reelNum, symbolNum, colour)`: Adds a coloured background to a symbol for
              highlighting purposes (will only be visible if the symbol has some transparency).
            - `removeBackgroundFromSymbol(reelNum, symbolNum)`: Removes any added background.
            - `addFrameToSymbol(reelNum, symbolNum, colour, width)`: Adds a frame on top of the symbol for
              highlighting purposes.
            - `removeFrameFromSymbol(reelNum, symbolNum)`: Removes any added frame.
            - `addTextToSymbol(reelNum, symbolNum, text, textStyle)`: Adds a text overlay to a symbol.
            - `removeTextFromSymbol(reelNum, symbolNum)`: Removes any added text overlay.
    - `game.ui.endCurrentPhase(totalWin)`: A function that ends the current phase, adding the phase winnings
      to the total winnings and checking them against the value from the server. It also displays the phase
      winnings in the ticker. (This is automatically called when a new phase begins or when the round ends but
      the winnings check is not carried out in this case, so it should ideally be called explicitly when the
      PhaseCompleteEvent is triggered.)
    - `game.ui.setPhase(phaseName)`: A function that hides the UI for the current phase and shows the UI for
      the phase with the given name, while also initialising the balance for the new phase.
    - `game.ui.ticker`: The large ticker that appears to the right of the Spin buttons, used to explain game
      events as they happen. Use `game.ui.ticker.updateText(text)` to update the text with a scrolling
      animation.
- `game.shell`: The shell UI, comprised of all the UI elements around the PIXI app. The key properties you may
  need are:
    - `game.shell.eventHistory`: The text box at the bottom of the page that displays all events. Anything you
      display in the main ticker with `game.ui.ticker.updateText()` will also be displayed here automatically.
      To add additional output use `game.shell.eventHistory.addText(text)`.
    - `game.shell.sessionStats`: The session stats box at the top right of the page.
        - Use `game.shell.sessionStats.increment(statName)` to increment a stat by 1.
        - Use `game.shell.sessionStats.increment(statName, value)` to add a specific value.
        - The `statName` is the HTML element id without the "session-" at the start. (See section on Session
          Stats below.)
    - `game.shell.addWinsToHistory(wins)`: A function that takes the wins from a WinAnalysisEvent (or similar)
      and outputs them in a readable format to the event history.

### Custom animations

You can do simple animations of elements using the moveElement function that is included in events.js. For
more complex animations, speak to Gabriel or the dedicated frontend resource.

## UI

### srcFrontend/src/thisTitle/GameUI.js

This file is where you can customise the UI for your game. The UI is divided into two parts, the global UI (
which stays visible regardless of what phase is being played) and the phase UI (which is specific to the
current phase).

### Global UI

Near the top the page, inside the constructor, you can see a call to `super()`. The first two arguments of
this call are the layout and colours config objects. Adding properties to these allow you to easily make
simple changes to the global UI. If you create a new line in one of these objects and then press Ctrl-Space,
you will see the options available. The most important ones concern top, bottom, and side panels.

If you set `topPanelHeight`, `bottomPanelHeight`, `leftPanelWidth`, or `rightPanelWidth` in `layout`, a panel
will be created in which you can add additional UI elements. The rest of the UI elements will be automatically
moved to accommodate your panel. The panel will be visible throughout the game and available on
game.ui.topPanel etc. You can give the panel a background colour by adding the `topPanelColour`,
`bottomPanelColour`, `leftPanelColour`, or `rightPanelColour` properties in `colours`.

To add UI elements to these panels, scroll down to the `async init(dependencies)` function and add your UI
elements after the call to `super.init()`.

The prebuilt UI elements you can add include:

- `Button`: A rectangular button with a colour, some text, and an (optional) onClick action.
- `NiceText`: Formatted text (like the game title).
- `Panel`: A rectangular panel with a background colour which you can add other elements inside.
- `Ticker`: A text box that can be updated with new text and a scrolling animation (like the balance ticker).
- `MaskedContainer`: A container with a mask so its child elements are only visible within its bounds.

The constructor for each of these elements takes an object including a parent (e.g. this.topPanel), x and y
position (relative to top left of parent element), width, height, and other parameters specific to the
element.

It is recommended that when creating these elements you store them as properties of the parent panel (see the
commented example in GameUI.js) if you need to access/manipulate them in your event handlers.

### Phase UI

Below the layout and colours config objects in the constructor is the `phases` object. Each game phase must
appear here with an id and a phase constructor. The phase constructor has four arguments:

- `name`: A display-friendly name for the phase.
- `hasSpins`: A boolean indicating whether this phase should include a count of remaining spins.
- `layout`: Similar to global UI.
- `colours`: Similar to global UI.
- `grid`: Specifies options related to the grid:
    - `displayReelSet`: The reelset to be used for spin animations. Note how this is passed from the backend
      in Math Config below.
    - `suspenseSymbol`: When two or more of this symbol have appeared on the grid, the remaining reels will
      spin slower.
    - `simultaneousWinLineAnimations`: For games with large numbers of win lines, displays all wins
      simultaneously to save time.
    - `numReels` and `numRows`: These are set from `maths.numReels` and `maths.numRows` by default but can be
      overridden.
    - `isHoldAndSpin`: If true, constructs a grid where each symbol spins separately on a minireel.
    - `holdAndSpinBlankSymbols`: An array of symbol names (strings) that should not be locked during spins.
      The first of these will be used to fill the starting grid before the initial spin.
    - `weightedDisplayReelSet`: A different type of reelset to be used for spinning animations, where each
      symbol is chosen from a weighted distribution. Must be an array of objects each containing weight
      (number) and outcome (string) properties. Can also be an array of these arrays, one for each reel.

Note that Phase UIs can also have top, bottom, and side panels created in the same way as global UI. However,
adding elements to these panels must be done in the `buildPhaseUIs()` function after the call the
`super.buildPhaseUIs()` function.

## Math Config

### srcBackend/thisTitle/frontendConfig/FrontendConfig.cpp

This file is responsible for constructing a JSON object containing data from the maths spreadsheet that is
required by the frontend. You can add anything else that you require in the frontend here. The resulting
object is available in event handlers on `game.maths`, and in GameUI.js via `maths` in the constructor and
`this.maths` in the other functions.

This file is also where you choose the maths sheet that will be used by the frontend.

## Session Stats

### srcFrontend/src/thisTitle/sessionStats.js

This file contains the list of session stats, and you can add your own here, following the pattern of the
other stats.

Note that the element name and id must be identical and start with "session-" and the rest of the id will be
used to identify the stat when updating it.

Note also that the class list is important:

- `session-data` must be set for every stat.
- `currency` causes the stat to be formatted as currency.
- `percent` causes the stat to be formatted as a percentage.
- `calculated` denotes a stat that is not updated manually but calculated by dividing one stat by another.
- `numerator-*****` and `divisor-*****` are used for `calculated` stats to specify the other stats used to
  calculate their value. The ***** must be replaced with the relevant stat id (without the "session-" prefix).
- `feeds-****` must be set for on stats that are used as the numerator or divisor in a `calculated` stat.
  The ***** must be replaced with the `calculated` stat id (without the "session-" prefix). This will cause
  the `calculated` stat to automatically update when the feed stat changes.