// The MIT License
// 
// Copyright (c) 2011 Dylan Smith
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

// Store information on and manipulate, collide, create and destroy
// objects in the game.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ctfserv.h"
#include "ctfmessage.h"

// Rotation/direction lookup table. Clockwise from north, with
// 16 steps around the circle.
struct mvlookup vectbl[] = {
	{0, -16}, {6, -15}, {11, -11}, {15, -6},
	{16, 0}, {15, 6}, {11, 11}, {6, 15},
	{0, 16}, {-6, 15}, {-11, 11}, {-15, 6},
	{-16, 0}, {-15, -6}, {-11, -11}, {-6, -15}
};

// Master object list
// While a linked list would be more memory efficient (and
// not have a hard limit), the array position makes for
// a simple 8 bit object id which the client can very
// rapidly look up (in other words, it makes the code much
// simpler)
Object *objlist[MAXOBJS];

// Player list
Player *players[MAXCLIENTS];

// Set all object entries to null, clear viewports etc.
void initObjList() {
	memset(objlist, 0, sizeof(objlist));
	memset(players, 0, sizeof(players));
}

// Compare two viewports to see if they are the same.
bool viewPortEquals(Viewport *lhs, Viewport *rhs) {
	if(lhs->tx == rhs->tx &&
		 lhs->ty == rhs->ty &&
		 lhs->bx == rhs->bx &&
		 lhs->by == rhs->by)
		return TRUE;
	return FALSE;
}

// Add an object to the list of objects currently in the game.
void addObject(Object *obj) {
	int i;
	for(i=0; i < MAXOBJS; i++) {
		if(objlist[i] == NULL) {
			objlist[i]=obj;
			return;
		}
	}
	fprintf(stderr, "addObject: Object list is full!\n");
}

// Remove an object from the list (not: does not free the object
// from memory)
int deleteObject(Object *obj) {
	int i;
	for(i=0; i < MAXOBJS; i++) {
		if(objlist[i] == obj) {
			objlist[i]=NULL;
			return;
		}
	}
	fprintf(stderr, "deleteObject: object not found\n");
}

// This is called before the game starts. We just initialize
// the player object and return it, where the id can be sent back
// to the client. The user on the client can then select various
// things like their team etc.
// When the game is started, the various bits of data in the 
// Player struct will be filled in, adn the player's object also
// filled in with things like initial X and Y positions and
// the object added to the object list.
// Returns a NULL pointer if a player couldn't be added.
Player *makeNewPlayer(int clientid, char *playerName) {
	int playeridx;
	Player *p;
	Object *tank;

	p=(Player *)malloc(sizeof(Player));
	tank=(Object *)malloc(sizeof(Object));
	players[clientid]=p;
	if(!p || !tank) {
		perror("makeNewPlayer: malloc");
		return NULL;
	}

	memset(p, 0, sizeof(Player));
	memset(tank, 0, sizeof(Object));
	strlcpy(p->name, playerName, MAXNAME);
	p->playerobj=tank;

	return p;
}

// startPlayer creates the initial starting spot for a player
// and the start message.
void startPlayer(int clientid) {
	MapXY spawn;
	Player *player=players[clientid];

	fprintf(stderr, "startPlayer for client %d\n", clientid);

	// TODO: something real
	spawn.mapx=100;
	spawn.mapy=100;
	player->playerobj->x=spawn.mapx*16;
	player->playerobj->y=spawn.mapy*16;

	// Add the player object to the object list
	addObject(player->playerobj);

	// Tell the client to initialize. The client will use the MapXY
	// to figure out where the viewport should be. The client will
	// then respond by telling the server the viewport.
	addInitGameMsg(clientid, &spawn);
	sendMessage(clientid);
}

// Get a player by id.
Player *getPlayer(int clientid) {
	return players[clientid];
}

// Make the sprite messages to update each player's display.
void makeSpriteUpdates(int clientid) {
	int objid;
	Object *obj;
	Player *player=players[clientid];

	for(int objid=0; objid < MAXOBJS; i++) {
		obj=objlist[objid];

		// We'll send a message if the object is within the player's viewport,
		// but only if the object moved or was destroyed or left the viewport.
		if(obj != NULL) {
			if(objIsInView(obj, &player->view)) {
				if(obj->flags & DESTROYED) {
					addDestructionMsg(clientid, objid, KILLED);
				}
				else if((obj->flags & HASMOVED) || (player->flags & NEWVIEWPORT)) {
					addSpriteMsg(clientid, obj, objid);
				}
			}
			else if(objWasInView(obj, &player->view)) {
				addDestructionMsg(clientid, objid, OFFSCREEN);
			}
		}
	}
}

bool objIsInView(Object *obj, Viewport *view) {
	// Remove the least significant 4 bits which are fractions of
	// a map position.
	int ox=obj->x >> 4;
	int oy=obj->y >> 4;

	if(ox >= view->tx && ox <= view->bx &&
			oy >= view->ty && oy <= view->by)
		return TRUE;
	return FALSE;
}

bool objWasInView(Object *obj, Viewport *view) {
	// Remove the least significant 4 bits which are fractions of
	// a map position.
	int ox=obj->prevx >> 4;
	int oy=obj->prevy >> 4;

	if(ox >= view->tx && ox <= view->bx &&
			oy >= view->ty && oy <= view->by)
		return TRUE;
	return FALSE;
}
