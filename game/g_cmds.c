/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "g_local.h"
#include "m_player.h"


char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	//Added for Zombies mod testing
	if (Q_stricmp(name, "points") == 0)
	{
		ent->client->pointCount = 100000;
		ent->client->valChanged = true;
		return;
	}

	/*
	* Logic for giving the player powerups here
	*/

	if (Q_stricmp(name, "max_ammo") == 0)
	{
		gi.centerprintf(ent, "Max Ammo!");
		ent->client->hasMaxAmmo = true;
		return;
	}

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

gi.cprintf(ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f(edict_t* ent)
{
	char* msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET))
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.cprintf(ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f(edict_t* ent)
{
	char* msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf(ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f(edict_t* ent)
{
	int			index;
	gitem_t* it;
	char* s;

	//Used to see if the player wants to buy a weapon they already own
	qboolean itemOwned = false;

	//Used for determining if the item to purchase is a weapon or perk
	qboolean weaponBool;

	//Used for giving the player a weapon they purchase
	edict_t* it_ent;

	s = gi.args();

	//Check if the player is purchasing an item
	if (ent->client->showbuymenu == true)
	{
		//Determine what item the player wants to purchase

		//NOTE: Make max ammos common because default ammo is not a lot

		it = FindItem(s);
		if (it != NULL)
		{
			//The item trying to purchase is a weapon
			index = ITEM_INDEX(it);
			if (ent->client->pers.inventory[index])
			{
				//Added extra new lines for nice formatting with the zombies UI
				itemOwned = true;
			}
			weaponBool = true;
		}
		else
		{
			//The item trying to purchase is a perk
			weaponBool = false;
		}

		//Player wants to buy a weapon
		if (weaponBool == true)
		{
			if ((Q_stricmp(s, "Shotgun") == 0))
			{
				//Want to buy the shotgun

				//Check if the player already has a shotgun
				if (itemOwned)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already own the shotgun.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase the shotgun

					//Check if the player can afford the shotgun
					if (ent->client->pointCount < ent->client->shotgunPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford the shotgun.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->shotgunPrice;
						ent->client->valChanged = true;

						//Actually give the player the shotgun here
						it_ent = G_Spawn();
						it_ent->classname = it->classname;
						SpawnItem(it_ent, it);
						Touch_Item(it_ent, ent, NULL, NULL);
						if (it_ent->inuse)
							G_FreeEdict(it_ent);

						gi.cprintf(ent, PRINT_HIGH, "Shotgun purchased for 1000 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "Super Shotgun") == 0))
			{
				//Want to buy the super shotgun

				//Check if the player already has a super shotgun
				if (itemOwned)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already own the super shotgun.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase the super shotgun

					//Check if the player can afford the super shotgun
					if (ent->client->pointCount < ent->client->supershotgunPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford the super shotgun.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->supershotgunPrice;
						ent->client->valChanged = true;

						//Actually give the player the super shotgun here
						it_ent = G_Spawn();
						it_ent->classname = it->classname;
						SpawnItem(it_ent, it);
						Touch_Item(it_ent, ent, NULL, NULL);
						if (it_ent->inuse)
							G_FreeEdict(it_ent);

						gi.cprintf(ent, PRINT_HIGH, "Super shotgun purchased for 3000 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "Machinegun") == 0))
			{
				//Want to buy the machinegun

				//Check if the player already has a machinegun
				if (itemOwned)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already own the machinegun.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase the machinegun

					//Check if the player can afford the machinegun
					if (ent->client->pointCount < ent->client->machinegunPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford the machinegun.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->machinegunPrice;
						ent->client->valChanged = true;

						//Actually give the player the machinegun here
						it_ent = G_Spawn();
						it_ent->classname = it->classname;
						SpawnItem(it_ent, it);
						Touch_Item(it_ent, ent, NULL, NULL);
						if (it_ent->inuse)
							G_FreeEdict(it_ent);

						gi.cprintf(ent, PRINT_HIGH, "Machinegun purchased for 2000 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "Chaingun") == 0))
			{
				//Want to buy the chaingun

				//Check if the player already has a chaingun
				if (itemOwned)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already own the chaingun.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase the chaingun

					//Check if the player can afford the chaingun
					if (ent->client->pointCount < ent->client->chaingunPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford the chaingun.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->chaingunPrice;
						ent->client->valChanged = true;

						//Actually give the player the chaingun here
						it_ent = G_Spawn();
						it_ent->classname = it->classname;
						SpawnItem(it_ent, it);
						Touch_Item(it_ent, ent, NULL, NULL);
						if (it_ent->inuse)
							G_FreeEdict(it_ent);

						gi.cprintf(ent, PRINT_HIGH, "Chaingun purchased for 4000 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "Grenade Launcher") == 0))
			{
				//Want to buy the grenade launcher

				//Check if the player already has a grenade launcher
				if (itemOwned)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already own the grenade launcher.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase the grenade launcher

					//Check if the player can afford the grenade launcher
					if (ent->client->pointCount < ent->client->grenadelauncherPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford the grenade launcher.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->grenadelauncherPrice;
						ent->client->valChanged = true;

						//Actually give the player the grenade launcher here
						it_ent = G_Spawn();
						it_ent->classname = it->classname;
						SpawnItem(it_ent, it);
						Touch_Item(it_ent, ent, NULL, NULL);
						if (it_ent->inuse)
							G_FreeEdict(it_ent);

						gi.cprintf(ent, PRINT_HIGH, "Grenade launcher purchased for 7000 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "Rocket Launcher") == 0))
			{
				//Want to buy the rocket launcher

				//Check if the player already has a rocket launcher
				if (itemOwned)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already own the rocket launcher.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase the rocket launcher

					//Check if the player can afford the rocket launcher
					if (ent->client->pointCount < ent->client->rocketlauncherPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford the rocket launcher.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->rocketlauncherPrice;
						ent->client->valChanged = true;

						//Actually give the player the rocket launcher here
						it_ent = G_Spawn();
						it_ent->classname = it->classname;
						SpawnItem(it_ent, it);
						Touch_Item(it_ent, ent, NULL, NULL);
						if (it_ent->inuse)
							G_FreeEdict(it_ent);

						gi.cprintf(ent, PRINT_HIGH, "Rocket launcher purchased for 5000 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "HyperBlaster") == 0))
			{
				//Want to buy the chaingun

				//Check if the player already has a chaingun
				if (itemOwned)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already own the chaingun.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase the chaingun

					//Check if the player can afford the chaingun
					if (ent->client->pointCount < ent->client->chaingunPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford the chaingun.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->chaingunPrice;
						ent->client->valChanged = true;

						//Actually give the player the chaingun here
						it_ent = G_Spawn();
						it_ent->classname = it->classname;
						SpawnItem(it_ent, it);
						Touch_Item(it_ent, ent, NULL, NULL);
						if (it_ent->inuse)
							G_FreeEdict(it_ent);

						gi.cprintf(ent, PRINT_HIGH, "Hyperblaster purchased for 6000 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "Railgun") == 0))
			{
				//Want to buy the railgun

				//Check if the player already has a railgun
				if (itemOwned)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already own the railgun.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase the railgun

					//Check if the player can afford the railgun
					if (ent->client->pointCount < ent->client->railgunPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford the railgun.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->railgunPrice;
						ent->client->valChanged = true;

						//Actually give the player the railgun here
						it_ent = G_Spawn();
						it_ent->classname = it->classname;
						SpawnItem(it_ent, it);
						Touch_Item(it_ent, ent, NULL, NULL);
						if (it_ent->inuse)
							G_FreeEdict(it_ent);

						gi.cprintf(ent, PRINT_HIGH, "Railgun purchased for 8000 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "BFG10K") == 0))
			{
				//Want to buy the bfg

				//Check if the player already has a bfg
				if (itemOwned)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already own the bfg.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase the bfg

					//Check if the player can afford the bfg
					if (ent->client->pointCount < ent->client->bfgPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford the bfg.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->bfgPrice;
						ent->client->valChanged = true;

						//Actually give the player the bfg here
						it_ent = G_Spawn();
						it_ent->classname = it->classname;
						SpawnItem(it_ent, it);
						Touch_Item(it_ent, ent, NULL, NULL);
						if (it_ent->inuse)
							G_FreeEdict(it_ent);

						gi.cprintf(ent, PRINT_HIGH, "BFG purchased for 9000 points.");
					}
				}
			}
		}
		else //Player wants to buy a perk
		{
			if ((Q_stricmp(s, "Juggernog") == 0))
			{
				//Want to buy juggernog perk

				//Check if the player already has juggernog
				if (ent->client->hasJuggernog == true)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already have Juggernog.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase juggernog

					//Check if the player can afford juggernog
					if (ent->client->pointCount < ent->client->juggernogPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford Juggernog.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->juggernogPrice;
						ent->client->valChanged = true;

						//Increase the amount of perks the player has
						ent->client->perkCount = ent->client->perkCount + 1;

						//Give the player juggernog
						ent->client->hasJuggernog = true;

						gi.cprintf(ent, PRINT_HIGH, "Juggernog purchased for 2500 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "PhD Flopper") == 0))
			{
				//Want to buy stamin-up perk

				//Check if the player already has stamin-up
				if (ent->client->hasPhDFlopper == true)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already have PhD Flopper.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase stamin-up

					//Check if the player can afford stamin-up
					if (ent->client->pointCount < ent->client->phdflopperPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford PhD Flopper.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->phdflopperPrice;
						ent->client->valChanged = true;

						//Increase the amount of perks the player has
						ent->client->perkCount = ent->client->perkCount + 1;

						//Give the player stamin-up
						ent->client->hasPhDFlopper = true;

						gi.cprintf(ent, PRINT_HIGH, "PhD Flopper purchased for 2000 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "Fire Ring") == 0))
			{
				//Want to buy ultra-jump perk

				//Check if the player already has ultra-jump
				if (ent->client->hasFireRing == true)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already have Fire Ring.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase ultra-jump

					//Check if the player can afford ultra-jump
					if (ent->client->pointCount < ent->client->fireringPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford Fire Ring.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->fireringPrice;
						ent->client->valChanged = true;

						//Increase the amount of perks the player has
						ent->client->perkCount = ent->client->perkCount + 1;

						//Give the player ultra-jump
						ent->client->hasFireRing = true;

						gi.cprintf(ent, PRINT_HIGH, "Fire Ring purchased for 1500 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "Double Tap") == 0))
			{
				//Want to buy double tap perk

				//Check if the player already has double tap
				if (ent->client->hasDoubleTap == true)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already have Double Tap.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase double tap

					//Check if the player can afford double tap
					if (ent->client->pointCount < ent->client->doubletapPrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford Double Tap.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->doubletapPrice;
						ent->client->valChanged = true;

						//Increase the amount of perks the player has
						ent->client->perkCount = ent->client->perkCount + 1;

						//Give the player double tap
						ent->client->hasDoubleTap = true;

						gi.cprintf(ent, PRINT_HIGH, "Double Tap purchased for 3000 points.");
					}
				}
			}
			else if ((Q_stricmp(s, "Quick Revive") == 0))
			{
				//Want to buy quick revive perk

				//Check if the player already has quick revive
				if (ent->client->hasQuickRevive == true)
				{
					gi.cprintf(ent, PRINT_HIGH, "\n\nYou already have Quick Revive.\n\n");

					//Add this line to give room for the message
					ent->client->timer = level.time + 1.0;
				}
				else
				{
					//Player can purchase quick revive

					//Check if the player can afford quick revive
					if (ent->client->pointCount < ent->client->quickrevivePrice)
					{
						gi.cprintf(ent, PRINT_HIGH, "\n\nYou cannot afford Quick Revive.\n\n");

						//Add this line to give room for the message
						ent->client->timer = level.time + 1.0;
					}
					else
					{
						//Decrease the player's point count
						ent->client->pointCount = ent->client->pointCount - ent->client->quickrevivePrice;
						ent->client->valChanged = true;

						//Increase the amount of perks the player has
						ent->client->perkCount = ent->client->perkCount + 1;

						//Give the player quick revive
						ent->client->hasQuickRevive = true;

						gi.cprintf(ent, PRINT_HIGH, "Quick Revive purchased for 500 points.");
					}
				}
			}
		}

		//Exit method here
		return;
	}

	it = FindItem(s);
	if (!it)
	{
		//Comment this out to not print if player presses a perk key outside of buy menu
		//gi.cprintf(ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi.cprintf(ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}

	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		//Added extra new lines for nice formatting with the zombies UI
		gi.cprintf (ent, PRINT_HIGH, "\n\nOut of item: %s\n\n", s);
		return;
	}

	it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		sprintf(st, "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}


/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}
	//Add buymenu command to draw the Buy Menu
	if (Q_stricmp(cmd, "buymenu") == 0)
	{
		Cmd_BuyMenu_f(ent);
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_stricmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_stricmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_stricmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
