# Fresh Random Civ Picker

**This is the C++ GUI version of the Fresh Random Civ Picker.**




## Purpose
Purpose of Fresh Random Civ Picker: the user can randomly iterate through the entire civ pool or a user-defined civ pool without drawing the same civ twice.

## GUI Description
The program has a graphical user interface which is a window. This window contains three tabs: "Draw", "Log", and "Civ Pool". On the Draw tab the user can draw a fresh random civ and see its icon and hear its jingle. There is also a tech tree button which links to the tech tree of the currently drawn civ. On the "Log" tab, the user can view the log of drawn civs as well as of remaining civs in the pool. On the "Civ Pool", the user can customise which civs are part of the civ pool.


## How do I use this application?
How to use: the user can click the "Draw" button (or press spacebar) to have the centre label display a fresh random civ. There is an indicator at the bottom right which tells how many civs have been drawn so far. If the user wishes to see the tech tree of the drawn civ, they can click the Tech Tree button near the bottom right corner of the window.


Once all civs have been drawn, the next draw will reset the program and all civs are once again available to draw. The user can at any time reset the program manually by clicking the "Reset" button (or pressing Return). The user can at any time navigate to the "Log" tab (by either clicking it or pressing Tab) to view the history of drawn civs. The user can at any time navigate to the "Civ Pool" tab to customise which civs are enabled in the pool. Disabled civs are never drawn.


## Features
- **Draw a fresh random civ!** *Never draw the same civ twice within the civ pool.*
- **Reset the drawn civ pool at any time!** *Whenever you want, reset the program and render all enabled civs drawable.*
- **Customise your own civ pool!** *Tailor the exact civ pool you want using the civ pool tab.*
- **Save your custom civ pool preset!** *At any time, save your civ pools to a file which can be loadable at any time.*
- **Pick up where you left off!** *Even without saving manually, when the program starts it will remember what civs you had enabled and drawn last time you ran the program. This can be turned off in options if the user prefers the program to reset upon startup.*
- **Quickdraw!** *The Fresh Random Civ Picker can draw a fresh random civ for you automatically as soon as you start the program. This can be turned off or on in options.*
- **Civ icons!** *See the civ icon of the civ you draw.*
- **Civ jingles!** *Hear the civ jingle of the civ you draw. Legacy jingles toggleable.*
- **Join an AoE2DE lobby straight from the app!** *Works the same as Windows run prompt but only accepts AoE2DE lobby code inputs.*
- **Go to the tech tree!** *At any time, go to the tech tree of the drawn civ. aoe2techtree.net courtesy of Siege Engineers.*
- **Undraw a drawn civ!** *Go back and undraw the civ you drew with Ctrl+Z. If you change your mind again right away, redraw the undrawn civ with Ctrl+X.*
- **Use hotkeys!** *Most of the program is entirely controllable with key bindings.*

## How to install
1. Go to releases
2. Click Fresh_Random_Civ_Picker.zip to download it
3. Extract at any directory which is not privileged (does not require admin rights)
4. Run FRCP_X.X.X.exe
5. Click More info
6. Click Run anyway

### Why does Microsoft Defender want to protect my PC against this program?
When you run the executable, Microsoft Defender Smart Screen will give you a popup telling you your PC has been protected. This doesn’t mean that it has scanned the executable, in fact you can go ahead and scan it with Microsoft Defender by right-clicking the executable and clicking "Scan with Microsoft Defender...". The popup only means that it has found that the application application is not digitally signed with a Microsoft-approved certificate. The smart screen popup is warning you that I have not acquired a certificate under the Microsoft Authenticode programme which requires some hundreds of euros in digital signature services per year which I cannot afford at this point in time.

## System requirements
OS: Windows x64

Drive space: at least 56.6 megabytes available

## Project Milestones Achieved
- v1.0: GUI with simple log
- v1.1: Customisable civ pool and user options
- v1.2: Persistent feature states and automatically logged current civ pool over program lifetimes and sessions
- v1.3: Ctrl+Z/Ctrl+X undraw/redraw feature, possibility to save and load presets, and ability to join AoE2DE lobby using lobby code

## What is this application?
This is an application which generates a randomly drawn civilisation name out of the civilisations available to play in a computer game called "Age of Empires II: Definitive Edition" which is a real-time strategy game. What my application offers which the random civ name generator in the game itself doesn't is the "freshness" which is that a civilisation is not repeated until all the civilisations have been iterated through.

## Why was it created?
I got the idea to create this application when I was playing the game one day and got to play the same civilisation twice throughout five matches, despite the fact that I picked random every time and the fact that the game has 45 civilisations. This made me feel frustrated and that I was wasting my time. As a full-time student and with other priorities in life I do not have the time to play the game every day. I might only have the opportunity to play five matches in a month. I want to play random civilisations and I also want to minimise the repetition of some factors like the civilisation played. The frustrating experience of having to replay the same civilisation during my precious few five matches in a game that has 45(!) civilisations gave me the motivation to create a solution for this problem. Thus, the idea, work, and birth of this program came to.


## Regarding usage of Microsoft assets in this project
Age of Empires II © Microsoft Corporation. Fresh Random Civ Picker was created under Microsoft's "Game Content Usage Rules" using assets from Age of Empires II: Age of Kings, Age of Empires II: HD Edition, and Age of Empires II: Definitive Edition, and it is not endorsed by or affiliated with Microsoft. https://www.xbox.com/en-us/developers/rules
