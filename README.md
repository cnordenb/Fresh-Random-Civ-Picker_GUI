
![social-preview](https://github.com/user-attachments/assets/5ae32863-a995-4922-9a32-4850bed951d9)

# Fresh Random Civ Picker

**This is the Fresh Random Civ Picker.**  
![demo gif](https://github.com/user-attachments/assets/04f98b24-e096-4030-825b-c0cd147f80bf)

### Table of contents (click to jump)  
- Getting Started
  - [Installation](#How-to-install-and-run-a-release)
  - [Compilation](#How-to-compile-in-IDE)  
- Usage
  - [Using the Fresh Random Civ Picker](#How-do-I-use-this-application)
  - [Tips and tricks](#Tips-and-tricks)
- About  
  - [Purpose](#Purpose)  
  - [Application description](#What-is-this-application)
  - [Proof of concept](#Proof-of-concept)
  - [Features](#Features)  
  - [GUI description](#GUI-description)  
  - [System requirements](#System-requirements)  
  - [Project milestones](#Project-milestones-achieved)  
  - [Origin story](#Why-was-it-created)  
  - [Known bugs](#Known-bugs-as-of-2025-05-08)  
  - [Contact](#Contact)  
  - [Acknowledgements](#Acknowledgements)  
  - [Microsoft assets](#Regarding-usage-of-Microsoft-assets-in-this-project)  

## Purpose
The user can randomly iterate through the entire civ pool or a user-defined civ pool without drawing the same civ twice.

## What is this application?
This - the Fresh Random Civ Picker or FRCP for short - is an application which generates a randomly drawn civilisation out of the civilisations available to play in a computer game called [Age of Empires II: Definitive Edition](https://store.steampowered.com/app/813780/Age_of_Empires_II_Definitive_Edition/) which is a real-time strategy game. What my application offers which the random civ generator in the game itself does not is the "freshness" which is that a civilisation is not repeated until all the civilisations have been iterated through.  

## Proof of concept
In this youtube video (10 minutes) I explain the need for fresh random and demonstrate how it's different from the game's random. Click the image below to go the video.  

[![YouTube video describing the need for and use of fresh random](https://img.youtube.com/vi/mDj5id8GE2g/0.jpg)](https://www.youtube.com/watch?v=mDj5id8GE2g)

## Features
- **Draw a fresh random civ!**  
*Never draw the same civ twice within the civ pool. Click the Draw button or press the spacebar.*

- **Freshness even over civ pools!**  
*Ensure continuous freshness once you've iterated through the whole civ pool. Enable continuous freshness setting in options to hardcode the program to exclude recently drawn civs. Adjustable to count anything from just one previous to previous half of current civ pool length.*

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
*Works the same as Windows run prompt but only accepts AoE2DE lobby code inputs. To open lobby code prompt, press Ctrl+F or click Join Lobby from the Lobbies menu. It's also possible to spectate games with game ID using this function.*

- **Undraw a drawn civ!**  
*Go back and undraw the civ you drew with Ctrl+Z. If you change your mind again right away, redraw the undrawn civ with Ctrl+X. If you have automatic logging enabled, you can also undo a reset.*

- **Read the histories!**  
*Browse the histories of the civs and see any civ's pool status as well as the time and game edition it was added to the game. Both search bar and dropdown menu available. Click the history button or press H while on Draw tab.*

- **Use hotkeys!**  
*Most of the program is entirely controllable with key bindings. To see which hotkeys there are and what they do, navigate to Hotkeys from either Options or the Help menu.*

## GUI description  
The program has a graphical user interface which is a window. This window contains three tabs: "Draw Civ", "Log", and "Civ Pool".  
![tabs gif](https://github.com/user-attachments/assets/97fc3d9d-7ffe-4455-9a60-3cb42a53002f)

On the Draw tab the user can draw a fresh random civ and see its icon and hear its jingle. There is also a tech tree button which links to the tech tree of the currently drawn civ. 
![draw demo gif](https://github.com/user-attachments/assets/66808895-0ee1-4d99-8677-20baf540a3ff)


On the "Log" tab, the user can view the log of drawn civs as well as of remaining civs in the pool.  
![log demo gif](https://github.com/user-attachments/assets/0994a260-4cd9-47ac-8e6a-9562c4059237)


On the "Civ Pool" tab, the user can customise which civs are part of the civ pool.  
![civpool demo gif](https://github.com/user-attachments/assets/66256e41-59f8-4cc6-9961-2dca608e435e)


## How to compile in IDE
Instructions for Visual Studio on Windows
1. Clone this repository
2. Open FRCP_GUI.cpp
3. Select FRCP_GUI as single startup project in Solution Property Pages
4. Run or debug

## How to install and run a release
**Step 1. Go to [Releases](https://github.com/cnordenb/Fresh-Random-Civ-Picker_GUI/releases), located on this page in the right-hand sidebar below About and above Packages**

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
When you run the executable, Microsoft Defender Smart Screen will give you a popup telling you your PC has been protected. This doesn’t mean that it has scanned the executable, in fact you can go ahead and scan it with Microsoft Defender by right-clicking the executable and clicking "Scan with Microsoft Defender...". The popup only means that it has found that the application application is not digitally signed with a Microsoft-approved certificate. The smart screen popup is warning you that I have not acquired a certificate under the [Microsoft Authenticode](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/authenticode) programme which requires some [hundreds of euros](https://www.sslshopper.com/microsoft-authenticode-certificates.html) in digital signature services per year which I cannot afford at this point in time.



## How do I use this application?  
After you've started FRCP (see [Installation](#How-to-install-and-run-a-release)), simply press spacebar or click the Draw button to receive your fresh random civ. You can track the drawn and remaining civs in the log tab or alter the civ pool in the custom civ pool tab. To reset - undrawing all drawn civs - press enter or click the Reset button. Draw and Reset cannot be used while in the custom civ pool tab.

## Tips and tricks
There's more to FRCP than simply drawing fresh random civs.
### Quick access
I personally like to have FRCP pinned to the taskbar in the first (leftmost spot). This means I can open FRCP at any time (such as in-game) simply by pressing Windows Key + 1. Tell FRCP to scram simply by pressing escape. It will shut down completely yet still pick things back up effortlessly next time you call it with Windows Key + 1 (unless you toggle off automatic civ pool logging in options). You can also have FRCP to draw a civ for you automatically on startup if you want. That is togglable in options.
### Lobby joining
AoE2DE has a nifty lobby joining feature which is the lobby code. Windows's run prompt (Windows Key + R) offers a quick and easy way to execute a lobby code and joining a lobby. FRCP offers the same function with Ctrl + F with the added benefit that it does not require the aoe2de://0/ prefix (unlike Windows's run prompt). This comes in handy if your friend just gave you the bare code or if you are copying the lobby code from a third party lobby browser which only gives it without prefix (such as aoe2insights.com).
### Saving and loading
FRCP allows you to save and load any amount of presets of civ pool and drawn civs. With the default automatic civ pool logging, FRCP remembers what civs you have enabled and drawn since last time you used it. You can save separate presets manually by Save Preset (from File dropdown menu top left or by pressing Ctrl + S). FRCP also has a quicksave feature, you can use it by pressing F5 and load the quicksave by pressing F9.
### Change your mind
Accidentally reset? Don't like the civ you drew? Ctrl + Z has your back! Double-change your mind? Redraw an undrawn civ with Ctrl + X. Reset can only be undone if automatic civ pool logging is enabled but undrawing and redrawing civs works regardless.
### Use FRCP as a quiz app
Did you know you can toggle icon, jingle, or/and civ name on and off whenever you want? Either in options or with a hotkey (hotkeys can be viewed with F2 or from the Help dropdown menu), you can toggle eg. the civ name label off. Drawing a civ would then only give you the jingle and icon. You and your friends can pass the time by guessing which civ the jingle and emblem belongs to.
### Enjoy the civ jingles
Without needing to redraw a civ, you can always listen to its jingle again by clicking the civ icon. If it's a legacy civ (eg. Spanish), you can switch between the definitive and classic jingles in the Jingle Type dropdown menu in options.
### Use FRCP as a shortcut to Survivalist's sustainability app
Sometimes we just need to know how many villagers are needed on what. FRCP helps you get to Survivalist's nifty app quickly. There is a dedicated button in the Draw tab which will take you to the https://aoe2-de-tools.herokuapp.com/villagers-required/ webpage. You can also use hotkey R (when in draw tab) or Ctrl + T. 
### Orient yourself with the civilisations of Age of Empires II using FRCP
There are now half a hundred civs in the game which have been released in a dozen different editions and DLC's in the past 26 years. Keeping track of them all can be daunting, especially for someone new or returning. The Civ Pool tab allows you to easily overview which civs belong to which DLC and edition. See instantly how many civs which edition has and batch toggle any DLC or edition for your civ pool. FRCP also offers shortcut to the tech tree.

![civ pool gif](https://github.com/user-attachments/assets/ac8ce463-8829-4eaa-b9dd-44d8a7ff2806)



## System requirements
OS: Windows x64 (Lite version supports x86)

Drive space: at least 160mb available (Lite version requires only 500kb)

## Project milestones achieved  
- GUI with simple log ([v1.0.0](https://github.com/cnordenb/Fresh-Random-Civ-Picker_GUI/releases/tag/v1.0.0), January 2025)  
- Customisable civ pool, user options, persistent settings and optional logging ([v1.1.0](https://github.com/cnordenb/Fresh-Random-Civ-Picker_GUI/releases/tag/v1.1.0), February 2025)  
- Ctrl+Z/Ctrl+X undraw/redraw feature, ability to save and load presets, and to join AoE2DE lobbies using lobby code ([v1.2.0](https://github.com/cnordenb/Fresh-Random-Civ-Picker_GUI/releases/tag/v1.2.0), March 2025)  
- Quicksave and quickload feature, shortcut button to sustainability app ([v1.3.0](https://github.com/cnordenb/Fresh-Random-Civ-Picker_GUI/releases/tag/v1.3.0), April 2025)  
- Support for Three Kingdoms DLC, undo reset feature ([v1.3.5](https://github.com/cnordenb/Fresh-Random-Civ-Picker_GUI/releases/tag/v1.3.5), May 2025)
- Continuous freshness setting, history window, spectate option in lobby joining feature ([v1.4.0](https://github.com/cnordenb/Fresh-Random-Civ-Picker_GUI/releases/tag/v1.4.0), June 2025)  


## Why was Fresh Random Civ Picker created?
I got the idea to create this application when I was playing the game one day and got to play the same civilisation twice throughout five matches, despite the fact that I picked random every time and the fact that the game has 50 civilisations. This made me feel frustrated and that I was wasting my time. As a full-time student and with other priorities in life I do not have the time to play the game every day. I might only have the opportunity to play five matches in a month. I want to play random civilisations and I also want to minimise the repetition of some factors like the civilisation played. The frustrating experience of having to replay the same civilisation during my precious few five matches in a game that has 50(!) civilisations gave me the motivation to create a solution for this problem. Thus, the idea, work, and birth of this program came to.

## Known bugs as of 2025-05-08
  
Tracked: Sound playback works poorly on slower computers
  
Solved: The tech tree button for Maya and Inca not working  
  
Solved: Icons and jingles of Maya and Inca not working

## Contact  
If you have inquiries about this application, you can contact me at @Hjoerleif (Telegram/Discord) or cnordenb@abo.fi (email).

You can also submit bug reports or feature requests by creating an issue [here](https://github.com/cnordenb/Fresh-Random-Civ-Picker_GUI/issues).

Join the [the Fresh Random Civ Picker discord server](https://discord.gg/rcFqrBdVUN) to receive notifications about updates and new releases.


## Acknowledgements

- hszemi, Anda, exterkamp, paulirish, lalitpatel, Jineapple, TriRem, pip, and NkoDragaš over at [Siege Engineers](https://siegeengineers.org/) for their https://aoe2techtree.net/ website which my application links to for tech tree.
  
- [Survivalist](https://www.youtube.com/channel/UCmFJUeU8Oubp5Neldbq0JRA) for his https://aoe2-de-tools.herokuapp.com/ webapp which my application links to for production sustainability calculations.


## Regarding usage of Microsoft assets in this project
Age of Empires II © Microsoft Corporation. Fresh Random Civ Picker was created under Microsoft's "Game Content Usage Rules" using assets from Age of Empires II: Age of Kings, Age of Empires II: HD Edition, and Age of Empires II: Definitive Edition, and it is not endorsed by or affiliated with Microsoft. https://www.xbox.com/en-us/developers/rules
