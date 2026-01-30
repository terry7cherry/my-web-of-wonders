# Quick Start Guide

This document is intended to help you get started creating games with this template.

Before you start, make sure you've completed the setup guide (1a/1b) in this folder, and use the spreadsheet
parser docs (2) to prepare your PAR sheet.

## Overview

Code for the simulator (srcBackend) is split into two categories.

- common: shared code between games, not to be edited.
- thisTitle: game-specific code.

By only editing or adding files in thisTitle, we can ensure that anyone familiar with this template can easily
understand all games, since they know that they only need to look at the files in thisTitle.

This guide goes through each of the files in the thisTitle folder, explaining what they do and how to edit
them.

## GameLogic.h and GameLogic.cpp

This file is where you'll implement your game's flow. It primarily includes code directly related to the game
logic, so you can focus on the mechanics and not worry about stats or the frontend. The exception to this is
lines related to recording events, which are explained shortly.

### Recommendations

It is recommended to try and keep the code in GameLogic.cpp as readable, maintainable, and reusable as
possible to help others understand the game flow easily, and minimise the potential for errors. There are a
few recommendations to help with achieving this. If you look at the template code in GameLogic.cpp, you'll see
how they are applied.

1. **Keep functions short and focused:** Most functions contain only 2–5 lines of code. The fewer the better.
   This makes it easy to see at a glance what a function does.
2. **Give functions descriptive names:** Don't worry if they become long, but make sure that you update them
   if you change the logic in the function.
3. **Avoid text comments where possible:** Text comments should absolutely be included if the code wouldn't be
   easy to understand without them, but if you can use 1. and 2. above to clarify the code, this is preferable
   because:
    - it is easy to forget to update text comments when the code they describe changes.
    - the reader will usually read the code as well as the comments, so it takes longer to understand the
      code.
4. **Avoid theme-specific language in function and variable names:** E.g., instead of "Fisherman" say
   "collectionSymbol". This makes your code more reusable if a new theme is applied later.
5. **Do not refer to symbols by their index or symbol names in code:** Use a symbol type in the spreadsheet to
   define the symbol used in a certain case (or a symbolWeightTable to choose a symbol randomly) and refer to
   this in your code. This means if the symbol names are changed later or more symbols are added to the
   pay table (maybe when someone uses your game as a base for a new game in the future) everything will still
   work.
6. **Make your maths sheet the single source of truth:** Don't copy and paste values to spreadsheet parser
   input, use formulas to import them. This eliminates a common source of bugs.

### Common tasks

#### Spin the reels

The Grid object on the GameCycle object (cycle) has an applySpin() function that can take a ReelSet or a
ReelSetWeights object.

```
cycle.grid.applySpin(maths.reelSetWeights["baseGame"]);
cycle.grid.applySpin(maths.reelSets["baseGame1"]);
```

(See the spreadsheet parser guide for more info on the maths object.)

#### Evaluate the spin results

The Grid object has a evaluateWins() function that takes the betMultiplier and returns the total win (in
cents/pennies).

```
cycle.grid.evalutateWins(cycle.session.betMultiplier);
```

(See GameSession section below for more info on betMultiplier.)

#### Other common grid tasks

The Grid object has a large number of other useful and well-documented functions for tasks like counting or
expanding symbols etc. If you just type "cycle.grid.", a list of these will pop up and as you move down the
list with the arrow keys you'll see descriptions of what each does.

### Recording events

Events serve two purposes:

- Allow stats to be recorded.
- Provide information to frontend about what has happened in the game.
  You will need to record an event whenever you want to record a stat or update the frontend, and a single
  event can do both where appropriate.
  Record events as follows:

```
cycle.recordEvent<EventClassName>(params, for, event);
```

The frontend automatically handles all the stock events in srcBackend/common/events. If you need to create
your own events, see the section on events further down in this guide.

## state/GameCycle.h

The GameCycle object stores all game state information for the current cycle (paid game round). It is
recommended to add member variables to this object for any state information. While this may not be strictly
necessary for logic purposes it is a good idea because:

- If you ever want to create a force (for the frontend) that guarantees a certain state, this information must
  be on the GameCycle.
- Most of your GameLogic functions can just take cycle as a parameter and you know you always have access to
  any state info you might need.

Add any extra member variables you need into the GameCycle struct and either initialise them inline or use the
constructor (if they need initialising with values from maths, for example).

It's also a good idea to remove any unused members (like the fs... members if you don't have free spins in
your game).

### GameCycleReused

The GameCycleReused struct speeds up simulation by reusing heavy objects (like Grid) rather than instantiating
them every cycle. You can include and use a different type of Grid here (like WaysGrid), just make sure you
also update the reference in the GameCycle struct.

If your game uses other heavy objects, you can include them here too.

## GameSession.h

This object contains the game config (bet, rtp, betType) and an automatically calculated betMultiplier.

betMultiplier = bet / betTypeCost / minBet

In other words, it is the amount that pay table awards should be multiplied by to get the actual amount that
the player will win.

It also contains the simForce value (see section on forces further down in this guide).

The GameSession is available on the cycle object. You can access the above as follows:

```
cycle.session.bet;
cycle.session.betMultiplier;
cycle.session.rtp;
cycle.session.betType;
cycle.session.simForce;
```

GameSession is also the place where any persistent state is stored. You only need to edit GameSession.h if
your game has persistence (state that does not reset after each paid game
cycle). You will find the instructions for how to do so within GameSession.h.

## Forces

Forces are a way to guarantee certain outcomes. There are two types of forces:

1. Sim Force: These are designed for simulating certain parts of the game (like free spins) quickly.
2. Frontend Force: These are designed to give a random outcome in the frontend that satisfies a certain
   condition.

### Sim Forces

To implement a sim force, you must create a logic branch in your GameLogic.cpp file that depends on
cycle.session.simForce (this is set to 0 when no force is selected). See the example for ForceFreeSpins in the
template GameLogic code.

You will also need to add your new force in forces/simForces.h. You will need to add it as a new entry in the
SimForce enum, and also add a string name to the simForceNames vector. This name will appear as an option in
the CLI when you run a sim.

### Frontend Forces

Frontend forces work by running the game randomly over and over again until a certain condition is met, then
this game is presented in the frontend.

Frontend forces only need to be added in forces/frontendForces.h.

Follow the pattern in this file when you add a new force. The FrontendForce class takes two parameters to its
constructor:

- A string name to be displayed in the force list in the front end.
- A function that takes a GameCycle object and returns true if that cycle meets the condition for the force.

Note that 100 million attempts will be made to find a cycle that meets the condition. If no matching cycle is
found, an error is shown in the frontend. For forces that are too rare to be found within this time, the
player is free to try again.

## stats/StatsOutput.h and stats/StatsOutput.cpp

The outputStats method in this file is called at the end of a simulation. You can edit this file to output any
extra stats you want. There is some documentation in StatsOutput.cpp to help with this.

## Events

Events work differently depending on whether you run in frontend or sim mode.

- Sim mode: the cycle.recordEvent method just calls the static updateMeters method which stores any stats
  required.
- Frontend mode: the cycle.recordEvent method instantiates the Event class and the resulting object is added
  to an array of events for the GameCycle. When the GameCycle is ready to go to the frontend, the toJson
  method is called on each event in the array, and the resulting JSON is sent to the frontend.

Use the existing Events where practical.

For a new Event class:

- It must inherit from BasicEvent or one of the templates in srcBackend/common/events/eventTemplates.
- It must have a static updateMeters method with a function signature (parameters) EXACTLY the same as the
  signature of its constructor. This can be empty if no stats are required for the event.
- If the event needs to be visualised on the frontend, it must have a toJson method that returns a JSON
  object (Json::Value).

Note that when extending one of the templates, you must also pass the child class name as a template
parameter.
See how this is done in FreeSpinsCompleteEvent, for example.

The available templates are:

- **GridEventTemplate**: for events where the frontend needs the array of symbols in the grid. (Note:
  **GridUpdateEvent** is derived from this and the frontend handles this event automatically. The new grid is
  compared to the old one, and the changed positions are updated with a scrolling animation. The message
  passed into the Event constructor is also displayed in the ticker simultaneously.)
- **PhaseCompleteEventTemplate**: for events at the end of a game phase (free spins, bonus, etc.). This event
  automates the creation of distributions and stats.
- **SpinsCompleteEventTemplate**: Like PhaseCompleteEventTemplate, but specifically for free spins type
  phases, where a number of spins are awarded and more may be retriggered during the phase. Provides all the
  stats provided by PhaseCompleteEventTemplate plus some more specific ones.

When making your own Events that include stats, bear in mind that the simulator is multithreaded and so your
meters must be thread safe. A number of threadsafe meters are provided in
srcBackend/common/events/threadSafeMeters. When using these, declare them as static members in your event, and
update them in the static updateMeters method. The meters included are:

- **IntSumMeter**: A simple sum added to each time the event is recorded (for example, total winnings for the
  whole simulation).
- **IntMaxMeter**: A max updated each time the event is recorded (for example, max winnings for the whole
  simulation).
- **DistributionMeter**: Essentially a thread-safe unordered_map. For, e.g. win distributions or the total
  winnings from free spins broken down by number of spins awarded.
- **PayTableMeter**: A 2D (Symbol - OAK) array of uint64_t values. Can store, e.g. total winnings or hits for
  each award.