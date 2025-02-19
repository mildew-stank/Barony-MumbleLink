/*-------------------------------------------------------------------------------

	BARONY
	File: gate.cpp
	Desc: implements all gate related code

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "main.hpp"
#include "game.hpp"
#include "stat.hpp"
#include "entity.hpp"
#include "engine/audio/sound.hpp"
#include "net.hpp"
#include "collision.hpp"
#include "player.hpp"
#include "paths.hpp"

void actGate(Entity* my)
{
	if ( !my )
	{
		return;
	}

	my->actGate();
}

void Entity::actGate()
{
	const bool oldPassable = flags[PASSABLE];
	if ( multiplayer != CLIENT )
	{
		if ( circuit_status == 0 )
		{
			return;    //Gate needs the mechanism powered state variable to be set.
		}

		if ( gateInverted == 0 )
		{
			// normal operation
			if ( circuit_status == CIRCUIT_ON )
			{
				//Raise gate if it's closed.
				if ( !gateStatus )
				{
					gateStatus = 1;
					playSoundEntity(this, 81, 64);
					serverUpdateEntitySkill(this, 3);
				}
			}
			else
			{
				//Close gate if it's open.
				if ( gateStatus )
				{
					gateStatus = 0;
					playSoundEntity(this, 82, 64);
					serverUpdateEntitySkill(this, 3);
				}
			}
		}
		else
		{
			// inverted operation
			if ( circuit_status == CIRCUIT_OFF )
			{
				//Raise gate if it's closed.
				if ( !gateStatus )
				{
					gateStatus = 1;
					playSoundEntity(this, 81, 64);
					serverUpdateEntitySkill(this, 3);
				}
			}
			else
			{
				//Close gate if it's open.
				if ( gateStatus )
				{
					gateStatus = 0;
					playSoundEntity(this, 82, 64);
					serverUpdateEntitySkill(this, 3);
				}
			}
		}
	}
	else
	{
		this->flags[NOUPDATE] = true;
	}
	if ( !gateInit )
	{
		gateInit = 1;
		gateStartHeight = this->z;
		if ( gateInverted )
		{
			this->z = gateStartHeight - 12;
		}
		this->scalex = 1.01;
		this->scaley = 1.01;
		this->scalez = 1.01;

		createWorldUITooltip();
	}

	// rightclick message
	if ( multiplayer != CLIENT )
	{
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if ( selectedEntity[i] == this || client_selected[i] == this )
			{
				if (inrange[i])
				{
					messagePlayer(i, MESSAGE_INTERACTION, Language::get(475));
				}
			}
		}
	}

	if ( !gateStatus )
	{
		//Closing gate.
		if ( this->z < gateStartHeight )
		{
			gateVelZ += .25;
			this->z = std::min(gateStartHeight, this->z + gateVelZ);
		}
		else
		{
			gateVelZ = 0;
		}
	}
	else
	{
		//Opening gate.
		if ( this->z > gateStartHeight - 12 )
		{
			this->z = std::max(gateStartHeight - 12, this->z - 0.25);

			// rattle the gate
			gateRattle = (gateRattle == 0);
			if ( gateRattle )
			{
				this->x += .05;
				this->y += .05;
			}
			else
			{
				this->x -= .05;
				this->y -= .05;
			}
		}
		else
		{
			// reset the gate's position
			if ( gateRattle )
			{
				gateRattle = 0;
				this->x -= .05;
				this->y -= .05;
			}
		}
	}

	//Setting collision
	node_t* node;
	bool somebodyinside = false;
	if ( this->z > gateStartHeight - 6 && this->flags[PASSABLE] )
	{
		std::vector<list_t*> entLists;
		if ( multiplayer == CLIENT )
		{
			entLists.push_back(map.entities); // clients use old map.entities method
		}
		else
		{
			entLists = TileEntityList.getEntitiesWithinRadiusAroundEntity(this, 1);
		}
		for ( std::vector<list_t*>::iterator it = entLists.begin(); it != entLists.end() && !somebodyinside; ++it )
		{
			list_t* currentList = *it;
			for ( node = currentList->first; node != nullptr; node = node->next )
			{
				Entity* entity = (Entity*)node->element;
				if ( entity == this || entity->flags[PASSABLE] || entity->behavior == &actDoorFrame )
				{
					continue;
				}
				if ( entityInsideEntity(this, entity) )
				{
					somebodyinside = true;
					break;
				}
			}
		}
		if ( !somebodyinside )
		{
			this->flags[PASSABLE] = false;
		}
	}
	else if ( this->z < gateStartHeight - 9 && !this->flags[PASSABLE] )
	{
		this->flags[PASSABLE] = true;
	}

	//if ( multiplayer != CLIENT )
	//{
	//	if ( flags[PASSABLE] != oldPassable )
	//	{
	//		updateGatePath(*this);
	//	}
	//}
}
