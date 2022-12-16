# Game-Mod-Q2-Fall2022

Michael O'Hanlon\
IT266 Game Mod Development Fall 2022\
Quake 2 Call of Duty Zombies Mod

This project takes the source code for Quake 2 and modifies it to create a Call of Duty Zombies mode in Quake.
Goals for this project are:
- [x] Create a desktop shortcut that automatically launches the mod
- [x] Mod in Seperate Folder
- [x] README.md that explains the mod and its features
- [x] Modified Heads Up Display (HUD) reflecting a key feature of the mod\
  -Added a function to display, using the console, the wave number and point count of the player.\
  -Function is called whenever the player kills an enemy or based on a timer to counter console spam.
- [x] In-Game Help Screen explaining mod changes\
  -Modified the existing Help Screen to reflect the Zombies mod.
- [ ] Wave Based System
- [x] Rethemed Weapons\
  -All 10 weapons have been modified to change the way each weapon behaves.
- [x] 5 Different Perks\
  -Juggernog: Increase max health from 100 to 200.\
  -PhD Flopper: Player takes no damage from explosions.\
  -Fire Ring: One enemy that is standing close to the player takes ticking damage to their health.\
  -Double Tap: All weapons fire twice per mouse click instead of once, doubles damage output.\
  -Quick Revive: If player dies, restore them to 50 health and lose all perks.
- [x] 5 Different Drops / Item Pickups / Powerups\
  -Double Points: Player earns 200 points for a kill instead of 100 for 10 seconds.\
  -Max Ammo: Instantly max out the ammo for all ammo types.\
  -Fire Sale: Reduce the price of all items in the shop by 500 points.\
  -Perk Power: Give the player all 5 perks for 10 seconds. Restore player's original perks after.\
  -Insta-Kill: Dramatically increase damage of all weapons so enemies die from any source of damage.\
  -Added system to prevent overload of powerup drops.\
  -A powerup can drop with a 10% chance and if no other powerups are active or spawned.
- [x] Point Based System / Buy Menu\
  -Buy Menu accessible using TAB. Displays all weapons and perks that are purchasable.\
  -Player earns 100 points for every zombie killed.
