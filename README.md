
![social-preview](https://github.com/user-attachments/assets/5ae32863-a995-4922-9a32-4850bed951d9)

# Fresh Random Civ Picker

**This is the Fresh Random Civ Picker.**  
![demo gif](https://github.com/user-attachments/assets/04f98b24-e096-4030-825b-c0cd147f80bf)

### Table of Contents (click to jump)  
[Purpose](#Purpose)  
[Application Description](#What-is-this-application)  
[Features](#Features)  
[GUI Description](#GUI-Description)  
[Usage](#How-do-I-use-this-application)   
[Installation](#How-to-install)  
[System requirements](#System-requirements)  
[Project Milestones](#Project-Milestones-Achieved)  
[Origin Story](#Why-was-it-created)  
[Acknowledgements](#Acknowledgements)  
[Microsoft Assets](#Regarding-usage-of-Microsoft-assets-in-this-project)  

## Purpose
The user can randomly iterate through the entire civ pool or a user-defined civ pool without drawing the same civ twice.

## What is this application?
This is an application which generates a randomly drawn civilisation out of the civilisations available to play in a computer game called "Age of Empires II: Definitive Edition" which is a real-time strategy game. What my application offers which the random civ generator in the game itself doesn't is the "freshness" which is that a civilisation is not repeated until all the civilisations have been iterated through.  

## Features
- **Draw a fresh random civ!**
*Never draw the same civ twice within the civ pool. Click the Drawn button or press the spacebar.*

- **Reset the drawn civ pool at any time!**
*Whenever you want, reset the program and render all enabled civs drawable. Click the Reset button or press Enter.*

- **Shortcut to the tech tree!**
*At any time, go to the [tech tree](https://aoe2techtree.net/) (made by [Siege Engineers](https://siegeengineers.org/)) of the drawn civ. Click the Tech Tree button or press T (global hotkey F4).*

- **Shortcut to the sustainability webapp!**
*At any time, go to the [sustainability app](https://aoe2-de-tools.herokuapp.com/villagers-required/) (made by [Survivalist](https://www.youtube.com/channel/UCmFJUeU8Oubp5Neldbq0JRA)) to see how many villagers are required for a production. Click the S button or press R (global hotkey Ctrl+T).*

- **Track drawn and remaining civs!**
*See which civs have been drawn and which are remaining on the log tab. Navigate to the log by clicking the Log tab, or by pressing 2 or tab.*

- **Customise your own civ pool!**
*Tailor the exact civ pool you want using the civ pool tab. Navigate to the civ pool by clicking the Civ Pool tab or pressing 3 or tab. Toggle civs by clicking their corresponding checkbox.*

- **Save your custom civ pool preset!**
*Save your civ pools to a file which can be loadable at any time. Press Ctrl+S or click Save Preset from the File menu. To load, press Ctrl+R or click Load Preset from the File menu. To quicksave and quickload, press F5 and F9.*

- **Autosave pool and preset; pick up where you left off!**  
*Even without saving manually, when the program starts it will remember what civs you had enabled and drawn last time you ran the program. This can be turned off in options if the user prefers the program to reset upon startup.*

- **Quickdraw a fresh random civ!**  
*The Fresh Random Civ Picker can draw a fresh random civ for you automatically as soon as you start the program. This can be turned off or on in options.*

- **Civ icons!**  
*See the civ icon of the civ you draw. Civ icons can be toggled off or on in options.*

- **Civ jingles!**  
*Hear the civ jingle of the civ you draw. Legacy jingles toggleable. Civ jingles can be toggled off or on in options.*

- **Join an AoE2DE lobby straight from the app!**  
*Works the same as Windows run prompt but only accepts AoE2DE lobby code inputs. To open lobby code prompt, press Ctrl+F or click Join Lobby from the Lobbies menu.*

- **Undraw a drawn civ!**  
*Go back and undraw the civ you drew with Ctrl+Z. If you change your mind again right away, redraw the undrawn civ with Ctrl+X. If you have automatic logging enabled, you can also undo a reset.*

- **Use hotkeys!**  
*Most of the program is entirely controllable with key bindings. To see which hotkeys there are and what they do, navigate to Hotkeys from either Options or the Help menu.*

## GUI Description  
The program has a graphical user interface which is a window. This window contains three tabs: "Draw Civ", "Log", and "Civ Pool".  
![tabs gif](https://github.com/user-attachments/assets/97fc3d9d-7ffe-4455-9a60-3cb42a53002f)

On the Draw tab the user can draw a fresh random civ and see its icon and hear its jingle. There is also a tech tree button which links to the tech tree of the currently drawn civ. 
![draw demo gif](https://github.com/user-attachments/assets/66808895-0ee1-4d99-8677-20baf540a3ff)


On the "Log" tab, the user can view the log of drawn civs as well as of remaining civs in the pool.  
![log demo gif](https://github.com/user-attachments/assets/0994a260-4cd9-47ac-8e6a-9562c4059237)


On the "Civ Pool" tab, the user can customise which civs are part of the civ pool.  
![civpool demo gif](https://github.com/user-attachments/assets/66256e41-59f8-4cc6-9961-2dca608e435e)



## How do I use this application?
How to use: the user can click the "Draw" button (or press spacebar) to have the centre label display a fresh random civ. There is an indicator at the bottom right which tells how many civs have been drawn so far. If the user wishes to see the tech tree of the drawn civ, they can click the Tech Tree button near the bottom right corner of the window.


Once all civs have been drawn, the next draw will reset the program and all civs are once again available to draw. The user can at any time reset the program manually by clicking the "Reset" button (or pressing Return). The user can at any time navigate to the "Log" tab (by either clicking it or pressing Tab) to view the history of drawn civs. The user can at any time navigate to the "Civ Pool" tab to customise which civs are enabled in the pool. Disabled civs are never drawn.


## How to install
**Step 1. Go to Releases, located on this page in the right-hand sidebar below About and above Packages**

![step 1](https://github.com/user-attachments/assets/c8340f36-7072-42e7-a3a3-15648a694656)

**Step 2. Under Assets, click Fresh_Random_Civ_Picker.zip to download it**

![step 2](https://github.com/user-attachments/assets/e144bae4-c132-427b-b3d6-76291d77c3c6)

**Step 3. Extract the downloaded zip file at any directory which is not privileged (does not require admin rights)**

![step 3](https://github.com/user-attachments/assets/9e244af8-6ed0-437c-aaf0-3c801b30c05c)

**Step 4. Navigate to inside the folder Fresh Random Civ Picker and run FRCP_X.X.X.exe**

![step 4](https://github.com/user-attachments/assets/42a86057-364c-429c-9fe3-a32fef87ff94)

**Step 5. Click More info**  

![step 5](https://github.com/user-attachments/assets/99dbf317-7ff7-4a3f-bfa8-b53eaecf522f)

**Step 6. Click Run anyway**  

![step 6](https://github.com/user-attachments/assets/005fbcbd-5c77-4ccf-af1c-76c5a4488cbd)


### Why does Microsoft Defender want to protect my PC against this program?
When you run the executable, Microsoft Defender Smart Screen will give you a popup telling you your PC has been protected. This doesn’t mean that it has scanned the executable, in fact you can go ahead and scan it with Microsoft Defender by right-clicking the executable and clicking "Scan with Microsoft Defender...". The popup only means that it has found that the application application is not digitally signed with a Microsoft-approved certificate. The smart screen popup is warning you that I have not acquired a certificate under the Microsoft Authenticode programme which requires some hundreds of euros in digital signature services per year which I cannot afford at this point in time.

## System requirements
OS: Windows x64 (Lite version supports x86)

Drive space: at least 71.3 megabytes available (Lite version requires only 500kb)

## Project Milestones Achieved
- v1.0: GUI with simple log
- v1.1: Customisable civ pool and user options
- v1.2: Persistent feature states and automatically logged current civ pool over program lifetimes and sessions
- v1.3: Ctrl+Z/Ctrl+X undraw/redraw feature, ability to save and load presets, and to join AoE2DE lobbies using lobby code


## Why was Fresh Random Civ Picker created?
I got the idea to create this application when I was playing the game one day and got to play the same civilisation twice throughout five matches, despite the fact that I picked random every time and the fact that the game has 45 civilisations. This made me feel frustrated and that I was wasting my time. As a full-time student and with other priorities in life I do not have the time to play the game every day. I might only have the opportunity to play five matches in a month. I want to play random civilisations and I also want to minimise the repetition of some factors like the civilisation played. The frustrating experience of having to replay the same civilisation during my precious few five matches in a game that has 45(!) civilisations gave me the motivation to create a solution for this problem. Thus, the idea, work, and birth of this program came to.


## Acknowledgements

- hszemi, Anda, exterkamp, paulirish, lalitpatel, Jineapple, TriRem, pip, and NkoDragaš over at [Siege Engineers](https://siegeengineers.org/) for their https://aoe2techtree.net/ website which my application links to for tech tree.
  
- [Survivalist](https://www.youtube.com/channel/UCmFJUeU8Oubp5Neldbq0JRA) for his https://aoe2-de-tools.herokuapp.com/ webapp which my application links to for production sustainability calculations.


## Regarding usage of Microsoft assets in this project
Age of Empires II © Microsoft Corporation. Fresh Random Civ Picker was created under Microsoft's "Game Content Usage Rules" using assets from Age of Empires II: Age of Kings, Age of Empires II: HD Edition, and Age of Empires II: Definitive Edition, and it is not endorsed by or affiliated with Microsoft. https://www.xbox.com/en-us/developers/rules
