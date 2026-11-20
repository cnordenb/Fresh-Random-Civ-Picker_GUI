requirement - a numbered statement in REQUIREMENTS.md of what the project must do, e.g. "Requirement 4"
rule - a numbered statement in RULES.md of how the project must be made and kept, e.g. "Rule 12"
chief developer - the person who alone writes the requirements, named in Rule 2
test - a check of the project, run on every push and listed by its number in TESTS.md, e.g. "Test 15"
log - the development log in documentation/DEVLOG/: what changed in the project, why, and what is left
part - one file of the log, DEVLOG1.md, DEVLOG2.md and so on, each of at most 1000 lines
day - the entry of the log for one date, titled by it, e.g. "2026-11-02", never split between two parts
section - one change recorded under its day, titled by its time if it was the first written in that minute
decision - a choice made in a section and listed by subject in DECISIONS.md, with a link to that section
question - a numbered point for the chief developer at the end of a section, open until a later section answers it
success criterion - a numbered WCAG 2.2 statement at level A or AA that the interface must meet, e.g. "2.1.1 Keyboard"
human verification - a person's check that a success criterion holds, needed where no test can check it in full
civ - a civilisation of the game Age of Empires II, e.g. "Britons"
edition - a release of the game, with its own civs and those of every older edition, e.g. "Definitive Edition"
DLC - a pack of civs that belongs to one edition, e.g. "The Conquerors"; an edition's first civs count as one too
web version - the same program as a web page, at frcp.net, whose look and data this program shares
data.json - the file that alone lists every edition, DLC and civ, kept identical to the web version's
limit - the most of a thing the program has room for, named in CivDataLimits.h, e.g. MAX_CIVS
startup - everything the program does before it starts to wait for input, the building of its window included
catalog - the editions, DLCs and civs as read from data.json and checked at startup; in the code, CivCatalog
pool - the civs the user has switched on, from which civs are picked
draw - to pick a random civ from the pool, none twice until all have been picked; also the civ so picked
reset - to make every civ of the pool free to draw again
round - the draws from one reset to the next
tab - one of the main window's three pages: "Draw Civ", "Log" and "Civ Pool"
toggle - a control on the "Civ Pool" tab that switches a civ, or all civs of a DLC, into or out of the pool
drawn log - the list on the "Log" tab of the civs drawn; not the log, which is the development log
continuous freshness - the option that keeps the civs drawn last out of the next draws, at most half the pool
asset folder - images/ or sounds/ in the program's folder, with its subfolders, exempt from Rules 14 and 15
icon - the picture of a civ, a DLC or an edition, a .png file in an asset folder, named as data.json names it
atlas - one image, built at startup, that holds many icons side by side in cells
jingle - a civ's short tune, a .wav file in an asset folder named after the civ, played when the civ is drawn
legacy jingle - the original game's jingle of a civ, for the civs of an edition with "legacyJingles" in data.json
history - a civ's text in the "History" dialog, read from a file named after the civ beside the program
share link - an address of the web version that carries an edition, a pool and, if wanted, the round's draws
hotkey - a keystroke that runs one of the program's actions, which the user can change, e.g. "Ctrl+S"
model - the code that holds the pool, the draws, the settings and every rule of the program, without Windows
view - the code that shows the model's state in windows and takes the user's input
controller - the code that turns the user's input from the view into calls of the model
observer - the object that the model tells of each change of its state; in the program, the view
assertion - a check with c_assert of what should never happen; when it fails, the function recovers
Parchment - the view's own controls, windows and drawing, with the web version's look, in the parchment/ folders
nine-slice - an image cut in nine parts, so that its corners keep their size when it is stretched into a frame
skin - a see-through window over a Parchment window that shows the translucent parts of its picture
