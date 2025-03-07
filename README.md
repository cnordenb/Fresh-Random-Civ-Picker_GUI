# Fresh Random Civ Picker

**This is the C++ GUI version of the Fresh Random Civ Picker.**

![frcp demo5](https://github.com/user-attachments/assets/e620fc7c-3e40-491b-8390-7e072b12e63e)



## Purpose
Purpose of Fresh Random Civ Picker: the user can randomly iterate through the entire civ pool or a user-defined civ pool without drawing the same civ twice.

## GUI Description
The program has a graphical user interface which is a window. This window contains three tabs: "Draw", "Log", and "Civ Pool".

On the Draw tab the user can draw a fresh random civ and see its icon and hear its jingle. There is also a tech tree button which links to the tech tree of the currently drawn civ.
![frcp draw tab](https://github.com/user-attachments/assets/af2074d3-1618-422b-8c3c-2fc88f2dc47b)

On the "Log" tab, the user can view the log of drawn civs as well as of remaining civs in the pool.
![frcp log tab](https://github.com/user-attachments/assets/f8cc8aa6-112e-4ca6-a0ed-ad029f3c30b3)

On the "Civ Pool", the user can customise which civs are part of the civ pool.
![frcp pool tab](https://github.com/user-attachments/assets/90b70859-7c95-41f3-b079-19970e79160e)

## How do I use this application?
How to use: the user can click the "Draw" button (or press spacebar) to have the centre label display a fresh random civ. There is an indicator at the bottom right which tells how many civs have been drawn so far.
![frcp demo6](https://github.com/user-attachments/assets/951b35e6-fe8b-4df4-a487-d6c85444bdba)

If the user wishes to see the tech tree of the drawn civ, they can click the Tech Tree button near the bottom right corner of the window.
![frcp demo7](https://github.com/user-attachments/assets/741be987-c490-4729-affc-95734e91765b)


Once all civs have been drawn, the next draw will reset the program and all civs are once again available to draw. The user can at any time reset the program manually by clicking the "Reset" button (or pressing Return). The user can at any time navigate to the "Log" tab (by either clicking it or pressing Tab) to view the history of drawn civs. The user can at any time navigate to the "Civ Pool" tab to customise which civs are enabled in the pool. Disabled civs are never drawn.

## What is this application?
This is an application which generates a randomly drawn civilisation name out of the civilisations available to play in a computer game called "Age of Empires II: Definitive Edition" which is a real-time strategy game. What my application offers which the random civ name generator in the game itself doesn't is the "freshness" which is that a civilisation is not repeated until all the civilisations have been iterated through.

## Why was it created?
I got the idea to create this application when I was playing the game one day and got to play the same civilisation twice throughout five matches, despite the fact that I picked random every time and the fact that the game has 45 civilisations. This made me feel frustrated and that I was wasting my time. As a full-time student and with other priorities in life I do not have the time to play the game every day. I might only have the opportunity to play five matches in a month. I want to play random civilisations and I also want to minimise the repetition of some factors like the civilisation played. The frustrating experience of having to replay the same civilisation during my precious few five matches in a game that has 45(!) civilisations gave me the motivation to create a solution for this problem. Thus, the idea, work, and birth of this program came to.

## Project Milestones Achieved
- v1.0: GUI with simple log
- v1.1: Customisable civ pool and user options
- v1.2: Persistent feature states over program lifetimes and sessions


### Regarding usage of Microsoft assets in this project
Age of Empires II © Microsoft Corporation. Fresh Random Civ Picker was created under Microsoft's "Game Content Usage Rules" using assets from Age of Empires II: Age of Kings, Age of Empires II: HD Edition, and Age of Empires II: Definitive Edition, and it is not endorsed by or affiliated with Microsoft. https://www.xbox.com/en-us/developers/rules
