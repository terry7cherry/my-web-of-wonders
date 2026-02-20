# RN Stream #

When there is difficulty matching math results with those from the server side, you have the option to request
an RN stream from the server dev. This is a CSV file containing a sequence of random numbers use in games and
results from those games. Each game is separated into spins, and a win is provided for each spin. This
framework can automatically read in this file, run the games, and report on discrepancies.

## Setup ##

To use the RN stream there are a couple of things you need to check:

- Ensure you are recording a `SpinCompleteEvent` at the end of each spin with the spin winnings.
- If you have other phases that the server treats as separate spins (you can ask the dev about this, or just
  try it and see what happens), make sure you are recording an Event derived from `PhaseCompleteEvent` for it.
- If you have additional phases that the server DOESN'T treat as separate spins and you have an Event for
  their completion, add this line to that event: `inline static bool countsAsSpinForRnStream = false;`

You should now be ready to go!

## Usage ##

Store the CSV file in the maths sheet folder (srcBackend/thisTitle/parSheets). Then set the build config in
CLion to `[windows/macos]-rn-stream` and run.

You'll be asked to select the CSV file and then the run will begin.

The output will contain a section for each game that had a discrepancy. The first problem will be given in
full detail, and later problems in that game will be summarised briefly (since they are most likely to be
caused by the first one).

A list of events for this game is then dumped to the console. However, you may find it easier to play the
problem game in the frontend to see what's going wrong.

## Diagnosing problems with the frontend ##

After the summary of problems, and before the list of events, the frontend will display a string that you can
copy into the frontend to play the game. Make a note of this string and then run again in frontend build
config. Paste the string into the RN stream box in the frontend UI and press Enter. The frontend will then
play the game and pause each time a problem is hit. You must press continue to carry on. You may wish to slow
or speed up playback to help you identify the issue.