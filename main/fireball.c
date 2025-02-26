/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.  
COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/
/*
 * $Source: f:/miner/source/main/rcs/fireball.c $
 * $Revision: 2.2 $
 * $Author: john $
 * $Date: 1995/03/21 14:39:57 $
 * 
 * Code for rendering & otherwise dealing with explosions
 * 
 * $Log: fireball.c $
 * Revision 2.2  1995/03/21  14:39:57  john
 * Ifdef'd out the NETWORK code.
 * 
 * Revision 2.1  1995/03/20  18:15:47  john
 * Added code to not store the normals in the segment structure.
 * 
 * Revision 2.0  1995/02/27  11:30:34  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 * 
 * Revision 1.200  1995/02/22  13:18:41  allender
 * remove anonymous unions from object structure
 * 
 * Revision 1.199  1995/02/14  19:58:32  mike
 * comment out "something bad has happened" int3.
 * 
 * Revision 1.198  1995/02/09  13:11:01  mike
 * remove an annoying mprintf and Int3().
 * 
 * Revision 1.197  1995/02/08  17:10:14  mike
 * don't drop cloaks if one nearby.
 * 
 * Revision 1.196  1995/02/08  13:27:14  rob
 * Give keys dropped by robots 0 velocity in coop game.
 * 
 * Revision 1.195  1995/02/08  11:57:40  mike
 * determine whether debris object failed to create because buffer was
 * exhausted or because limit was hit.
 * 
 * Revision 1.194  1995/02/08  11:37:58  mike
 * Check for failures in call to obj_create.
 * 
 * Revision 1.193  1995/02/07  21:09:41  mike
 * only replace weapon with energy 1/2 time.
 * 
 * Revision 1.192  1995/01/30  18:21:52  rob
 * Replace extra life powerups in multiplayer to invul when
 * dropped by robots.
 * 
 * Revision 1.191  1995/01/28  17:40:59  mike
 * fix stupidity in converting quad lasers to energy.
 * 
 * Revision 1.190  1995/01/27  15:05:59  rob
 * Trying to fix a bug with damaging robots with player badass explosions.
 * 
 * Revision 1.189  1995/01/26  18:59:04  rob
 * Powerups were flying too far in robot-cooperative games.
 * 
 * Revision 1.188  1995/01/25  10:53:35  mike
 * make badass damage go through grates.
 * 
 * Revision 1.187  1995/01/25  09:37:23  mike
 * fix objects containing robots, worked for powerups, bad {} placement.
 * 
 * Revision 1.186  1995/01/23  22:51:20  mike
 * drop energy instead of primary weapon if you already have primary weapon.
 * 
 * Revision 1.185  1995/01/20  16:56:37  mike
 * Cut damage done by badass weapons.
 * 
 * Revision 1.184  1995/01/19  17:44:57  mike
 * damage_force removed, that information coming from strength field.
 * 
 * Revision 1.183  1995/01/16  21:06:54  mike
 * Move function pick_random_point_in_segment from fireball.c to gameseg.c.
 * 
 * Revision 1.182  1995/01/16  19:24:04  mike
 * If a gated-in robot and going to drop energy powerup, don't!
 * 
 * Revision 1.181  1995/01/15  20:48:03  mike
 * drop energy in place of quad lasers if player already has quad lasers.
 * 
 * Revision 1.180  1995/01/14  19:32:19  rob
 * Fixed an error.
 * 
 * Revision 1.179  1995/01/14  18:50:55  rob
 * Make robot egg creation suitable for mutliplayer situations.
 * 
 * Revision 1.178  1995/01/14  14:55:07  rob
 * Make weapons/keys/etc never disappear in network mode.
 */


#pragma off (unreferenced)
static char rcsid[] = "$Id: fireball.c 2.2 1995/03/21 14:39:57 john Exp $";
#pragma on (unreferenced)

#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "3d.h"

#include "inferno.h"
#include "object.h"
#include "vclip.h"
#include "game.h"
#include "mono.h"
#include "polyobj.h"
#include "sounds.h"
#include "player.h"
#include "gauges.h"
#include "powerup.h"
#include "bm.h"
#include "ai.h"
#include "weapon.h"
#include "fireball.h"
#include "collide.h"
#include "newmenu.h"
#include "network.h"
#include "gameseq.h"
#include "physics.h"
#include "scores.h"
#include "laser.h"
#include "wall.h"
#include "multi.h"
#include "endlevel.h"
#include "timer.h"
#include "fuelcen.h"
#include "gameseg.h"

#define EXPLOSION_SCALE fl2f(2.5)		//explosion is the obj size times this  

//--unused-- ubyte	Frame_processed[MAX_OBJECTS];

object *object_create_explosion_sub(object *objp, short segnum, vms_vector * position, fix size, int vclip_type, fix maxdamage, fix maxdistance, fix maxforce, int parent )
{
	int objnum;
	object *obj;

	objnum = obj_create( OBJ_FIREBALL,vclip_type,segnum,position,&vmd_identity_matrix,size,
					CT_EXPLOSION,MT_NONE,RT_FIREBALL);

	if (objnum < 0 ) {
		mprintf((1, "Can't create object in object_create_explosion_sub.\n"));
		return NULL;
	}

	obj = &Objects[objnum];

	//mprintf( 0, "Fireball created at %d, %d, %d\n", obj->pos.x, obj->pos.y, obj->pos.z );

	//now set explosion-specific data

	obj->lifeleft = Vclip[vclip_type ].play_time;
	obj->ctype.expl_info.spawn_time = -1;
	obj->ctype.expl_info.delete_objnum = -1;
	obj->ctype.expl_info.delete_time = -1;

	if (maxdamage > 0) {
		fix dist, force;
		vms_vector pos_hit, vforce;
		fix damage;
		int i;
		object * obj0p = &Objects[0];
					  
		// -- now legal for badass explosions on a wall. Assert(objp != NULL);

		for (i=0; i<=Highest_object_index; i++ )	{
			//	Weapons used to be affected by badass explosions, but this introduces serious problems.
			//	When a smart bomb blows up, if one of its children goes right towards a nearby wall, it will
			//	blow up, blowing up all the children.  So I remove it.  MK, 09/11/94
			if ( (obj0p->type == OBJ_CNTRLCEN) || (obj0p->type==OBJ_PLAYER) || ((obj0p->type==OBJ_ROBOT) && ((Objects[parent].type != OBJ_ROBOT) || (Objects[parent].id != obj0p->id)))) {
				dist = vm_vec_dist_quick( &obj0p->pos, &obj->pos );
				// Make damage be from 'maxdamage' to 0.0, where 0.0 is 'maxdistance' away;
				if ( dist < maxdistance ) {
					if (object_to_object_visibility(obj, obj0p, FQ_TRANSWALL)) {
						damage = maxdamage - fixmuldiv( dist, maxdamage, maxdistance );
						force = maxforce - fixmuldiv( dist, maxforce, maxdistance );
						
						// Find the force vector on the object
						vm_vec_sub( &vforce, &obj0p->pos, &obj->pos );
						vm_vec_normalize_quick(&vforce);
						vm_vec_scale(&vforce, force );
	
						// Find where the point of impact is... ( pos_hit )
						vm_vec_scale(vm_vec_sub(&pos_hit, &obj->pos, &obj0p->pos), fixdiv(obj0p->size, obj0p->size + dist));
	
						switch ( obj0p->type )	{
							case OBJ_ROBOT:
								phys_apply_force(obj0p,&vforce);

								//	When a robot gets whacked by a badass force, he looks towards it because robots tend to get blasted from behind.
								{
									vms_vector neg_vforce; 
									neg_vforce.x = vforce.x * -2 * (7 - Difficulty_level)/8;
									neg_vforce.y = vforce.y * -2 * (7 - Difficulty_level)/8;
									neg_vforce.z = vforce.z * -2 * (7 - Difficulty_level)/8;
									phys_apply_rot(obj0p,&neg_vforce);
								}
								if ( obj0p->shields >= 0 ) {
									if (apply_damage_to_robot(obj0p, damage, parent))
										if ((objp != NULL) && (parent == Players[Player_num].objnum))
											add_points_to_score(Robot_info[obj0p->id].score_value);
								}
								break;
							case OBJ_CNTRLCEN:
								if ( obj0p->shields >= 0 ) {
									apply_damage_to_controlcen(obj0p, damage, parent );
								}
								break;
							case OBJ_PLAYER:	{
								object * killer=NULL; 
								vms_vector	vforce2;
								if ((objp != NULL) && (Game_mode & GM_MULTI) && (objp->type == OBJ_PLAYER)) {
//									mprintf((0, "Damaged by player %d's explosion.\n", objp->id));
									killer = objp;
								}
								vforce2 = vforce;
								if (parent > -1 ) {
									killer = &Objects[parent];
									if (killer != ConsoleObject)		// if someone else whacks you, cut force by 2x
										vforce2.x /= 2;	vforce2.y /= 2;	vforce2.z /= 2;
								}
								vforce2.x /= 2;	vforce2.y /= 2;	vforce2.z /= 2;

								phys_apply_force(obj0p,&vforce);
								phys_apply_rot(obj0p,&vforce2);
								if ( obj0p->shields >= 0 )
									apply_damage_to_player(obj0p, killer, damage );
							}
								break;

							default:
								Int3();	//	Illegal object type
						}	// end switch
					} else {
						; // mprintf((0, "No badass: robot=%2i, dist=%7.3f, maxdistance=%7.3f .\n", i, f2fl(dist), f2fl(maxdistance)));
					}	// end if (object_to_object_visibility...
				}	// end if (dist < maxdistance)
			}
			obj0p++;
		}	// end for
	}	// end if (maxdamage...

//	mprintf(0, "\n");

	return obj;

}


object *object_create_muzzle_flash(short segnum, vms_vector * position, fix size, int vclip_type )
{
	return object_create_explosion_sub(NULL, segnum, position, size, vclip_type, 0, 0, 0, -1 );
}

object *object_create_explosion(short segnum, vms_vector * position, fix size, int vclip_type )
{
	return object_create_explosion_sub(NULL, segnum, position, size, vclip_type, 0, 0, 0, -1 );
}

object *object_create_badass_explosion(object *objp, short segnum, vms_vector * position, fix size, int vclip_type, fix maxdamage, fix maxdistance, fix maxforce, int parent )
{
	object	*rval;

	rval = object_create_explosion_sub(objp, segnum, position, size, vclip_type, maxdamage, maxdistance, maxforce, parent );

	if (objp != NULL)
		create_smart_children(objp);

	return rval;
}

//blows up a badass weapon, creating the badass explosion
//return the explosion object
object *explode_badass_weapon(object *obj)
{
	weapon_info *wi = &Weapon_info[obj->id];

	Assert(wi->damage_radius);

	digi_link_sound_to_object(SOUND_BADASS_EXPLOSION, obj-Objects, 0, F1_0);

	return object_create_badass_explosion( obj, obj->segnum, &obj->pos, 
					wi->impact_size, 
					wi->robot_hit_vclip, 
					wi->strength[Difficulty_level], 
					wi->damage_radius,wi->strength[Difficulty_level],
					obj->ctype.laser_info.parent_num );

}

//blows up the player with a badass explosion
//return the explosion object
object *explode_badass_player(object *objp)
{
	object 	*rval;

	rval = object_create_badass_explosion(objp, objp->segnum, &objp->pos, objp->size,
					get_explosion_vclip(objp, 0),
					F1_0*50, F1_0*40, F1_0*150, 
					objp-Objects);
	if (rval)
		digi_link_sound_to_object(SOUND_BADASS_EXPLOSION, rval-Objects, 0, F1_0);
	return (rval);
}


#define DEBRIS_LIFE (f1_0 * 2)		//lifespan in seconds

object *object_create_debris(object *parent, int subobj_num)
{
	int objnum;
	object *obj;
	polymodel *po;

	Assert((parent->type == OBJ_ROBOT) || (parent->type == OBJ_PLAYER)  );

	objnum = obj_create(OBJ_DEBRIS,0,parent->segnum,&parent->pos,
				&parent->orient,Polygon_models[parent->rtype.pobj_info.model_num].submodel_rads[subobj_num],
				CT_DEBRIS,MT_PHYSICS,RT_POLYOBJ);

	if ((objnum < 0 ) && (Highest_object_index >= MAX_OBJECTS-1)) {
		mprintf((1, "Can't create object in object_create_debris.\n"));
		Int3();
		return NULL;
	}
	if ( objnum < 0 )
		return NULL;				// Not enough debris slots!
	obj = &Objects[objnum];

	Assert(subobj_num < 32);

	//Set polygon-object-specific data 

	obj->rtype.pobj_info.model_num = parent->rtype.pobj_info.model_num;
	obj->rtype.pobj_info.subobj_flags = 1<<subobj_num;
	obj->rtype.pobj_info.tmap_override = parent->rtype.pobj_info.tmap_override;

	//Set physics data for this object

	po = &Polygon_models[obj->rtype.pobj_info.model_num];

	obj->mtype.phys_info.velocity.x = RAND_MAX/2 - rand();
	obj->mtype.phys_info.velocity.y = RAND_MAX/2 - rand();
	obj->mtype.phys_info.velocity.z = RAND_MAX/2 - rand();
	vm_vec_normalize_quick(&obj->mtype.phys_info.velocity);
	vm_vec_scale(&obj->mtype.phys_info.velocity,i2f(10 + (30 * rand() / RAND_MAX)));

	vm_vec_add2(&obj->mtype.phys_info.velocity,&parent->mtype.phys_info.velocity);

	vm_vec_make(&obj->mtype.phys_info.rotvel,10*0x2000/3,10*0x4000/3,10*0x7000/3);
	vm_vec_zero(&obj->mtype.phys_info.rotthrust);

	obj->lifeleft = DEBRIS_LIFE;

	obj->mtype.phys_info.mass = fixmuldiv(parent->mtype.phys_info.mass,obj->size,parent->size);
	obj->mtype.phys_info.drag = 0; //fl2f(0.2);		//parent->mtype.phys_info.drag;

	return obj;

}

void draw_fireball(object *obj)
{
	//mprintf( 0, "[Drawing obj %d type %d fireball size %x]\n", obj-Objects, obj->id, obj->size );

	if ( obj->lifeleft > 0 )
		draw_vclip_object(obj,obj->lifeleft,0, obj->id);

}

// --------------------------------------------------------------------------------------------------------------------
//	Return true if there is a door here and it is openable
//	It is assumed that the player has all keys.
int door_is_openable_by_player(segment *segp, int sidenum)
{
	int	wall_num, wall_type;

	wall_num = segp->sides[sidenum].wall_num;
	wall_type = Walls[wall_num].type;

	if (wall_num == -1)
		return 0;						//	no wall here.

	//	Can't open locked doors.
	if (((wall_type == WALL_DOOR) && (Walls[wall_num].flags & WALL_DOOR_LOCKED)) || (wall_type == WALL_CLOSED))
		return 0;

	return 1;

}

#define	QUEUE_SIZE	64

// --------------------------------------------------------------------------------------------------------------------
//	Return a segment %i segments away from initial segment.
//	Returns -1 if can't find a segment that distance away.
int pick_connected_segment(object *objp, int max_depth)
{
	int		i;
	int		cur_depth;
	int		start_seg;
	int		head, tail;
	int		seg_queue[QUEUE_SIZE*2];
	byte		visited[MAX_SEGMENTS];
	byte		depth[MAX_SEGMENTS];
	byte		side_rand[MAX_SIDES_PER_SEGMENT];

//	mprintf((0, "Finding a segment %i segments away from segment %i: ", max_depth, objp->segnum));

	start_seg = objp->segnum;
	head = 0;
	tail = 0;
	seg_queue[head++] = start_seg;

	memset(visited, 0, Highest_segment_index+1);
	memset(depth, 0, Highest_segment_index+1);
	cur_depth = 0;

	for (i=0; i<MAX_SIDES_PER_SEGMENT; i++)
		side_rand[i] = i;

	while (tail != head) {
		int		sidenum;
		segment	*segp;
		byte		ind1, ind2, temp;

		if (cur_depth >= max_depth) {
//			mprintf((0, "selected segment %i\n", seg_queue[tail]));
			return seg_queue[tail];
		}

		segp = &Segments[seg_queue[tail++]];
		tail &= QUEUE_SIZE-1;

		//	to make random, switch a pair of entries in side_rand.
		ind1 = (rand() * MAX_SIDES_PER_SEGMENT) >> 15;
		ind2 = (rand() * MAX_SIDES_PER_SEGMENT) >> 15;
		temp = side_rand[ind1];
		side_rand[ind1] = side_rand[ind2];
		side_rand[ind2] = temp;

		for (sidenum=0; sidenum<MAX_SIDES_PER_SEGMENT; sidenum++) {
			int	snrand = side_rand[sidenum];
			int	wall_num = segp->sides[snrand].wall_num;

			if (((wall_num == -1) && (segp->children[snrand] > -1)) || door_is_openable_by_player(segp, snrand)) {
				if (visited[segp->children[snrand]] == 0) {
					seg_queue[head++] = segp->children[snrand];
					visited[segp->children[snrand]] = 1;
					depth[segp->children[snrand]] = cur_depth+1;
					head &= QUEUE_SIZE-1;
					if (head > tail) {
						if (head == tail + QUEUE_SIZE-1)
							Int3();	//	queue overflow.  Make it bigger!
					} else
						if (head+QUEUE_SIZE == tail + QUEUE_SIZE-1)
							Int3();	//	queue overflow.  Make it bigger!
				}
			}
		}
		if ((seg_queue[tail] < 0) || (seg_queue[tail] > Highest_segment_index)) {
			// -- Int3();	//	Something bad has happened.  Queue is trashed.  --MK, 12/13/94
			return -1;
		}
		cur_depth = depth[seg_queue[tail]];
	}

	mprintf((0, "...failed at depth %i, returning -1\n", cur_depth));
	return -1;
}

#define	BASE_NET_DROP_DEPTH	10

//	------------------------------------------------------------------------------------------------------
//	Choose segment to drop a powerup in.
//	For all active net players, try to create a N segment path from the player.  If possible, return that
//	segment.  If not possible, try another player.  After a few tries, use a random segment.
//	Don't drop if control center in segment.
int choose_drop_segment(void)
{
	int	pnum = 0;
	int	segnum = -1;
	int	initial_drop_depth = BASE_NET_DROP_DEPTH + ((rand() * 10) >> 15);
	int	cur_drop_depth = initial_drop_depth;
	int	count;

	srand(timer_get_fixed_seconds());

	while ((segnum == -1) && (cur_drop_depth > BASE_NET_DROP_DEPTH/2)) {
		pnum = (rand() * N_players) >> 15;
		count = 0;
		while ((Players[pnum].connected == 0) && (count < N_players)) {
			pnum = (pnum+1)%N_players;
			count++;
		}

		if (count == N_players) {
			mprintf((1, "Warning: choose_drop_segment: Couldn't find legal drop segment because no connected players.\n"));
			return (rand() * Highest_segment_index) >> 15;
		}

		segnum = pick_connected_segment(&Objects[Players[pnum].objnum], cur_drop_depth);
		if (Segments[segnum].special == SEGMENT_IS_CONTROLCEN)
			segnum = -1;
		cur_drop_depth--;
	}

	if (segnum == -1) {
		mprintf((1, "Warning: Unable to find a connected segment.  Picking a random one.\n"));
		return (rand() * Highest_segment_index) >> 15;
	} else
		return segnum;

}

#ifdef NETWORK
//	------------------------------------------------------------------------------------------------------
//	Drop cloak powerup if in a network game.
void maybe_drop_net_powerup(int powerup_type)
{
	if ((Game_mode & GM_MULTI) && !(Game_mode & GM_MULTI_COOP)) {
		int	segnum, objnum;
		vms_vector	new_pos;

		if (Fuelcen_control_center_destroyed || Endlevel_sequence)
			return;

		segnum = choose_drop_segment();
//--old-- 		segnum = (rand() * Highest_segment_index) >> 15;
//--old-- 		Assert((segnum >= 0) && (segnum <= Highest_segment_index));
//--old-- 		if (segnum < 0)
//--old-- 			segnum = -segnum;
//--old-- 		while (segnum > Highest_segment_index)
//--old-- 			segnum /= 2;

		Net_create_loc = 0;
		objnum = call_object_create_egg(&Objects[Players[Player_num].objnum], 1, OBJ_POWERUP, powerup_type);

		if (objnum < 0)
			return;

#ifndef SHAREWARE
		pick_random_point_in_seg(&new_pos, segnum);
#else
		compute_segment_center(&new_pos, &Segments[segnum]);
#endif

		multi_send_create_powerup(powerup_type, segnum, objnum, &new_pos);

		Objects[objnum].pos = new_pos;
		vm_vec_zero(&Objects[objnum].mtype.phys_info.velocity);
		obj_relink(objnum, segnum);

		object_create_explosion(segnum, &new_pos, i2f(5), VCLIP_POWERUP_DISAPPEARANCE );
//		mprintf(0, "Creating net powerup in segment %i at %7.3f %7.3f %7.3f\n", segnum, f2fl(new_pos.x), f2fl(new_pos.y), f2fl(new_pos.z));
	}
}
#endif

//	------------------------------------------------------------------------------------------------------
//	Return true if current segment contains some object.
int segment_contains_object(int obj_type, int obj_id, int segnum)
{
	int	objnum;

	if (segnum == -1)
		return 0;

	objnum = Segments[segnum].objects;

	while (objnum != -1)
		if ((Objects[objnum].type == obj_type) && (Objects[objnum].id == obj_id))
			return 1;
		else
			objnum = Objects[objnum].next;

	return 0;
}

//	------------------------------------------------------------------------------------------------------
int object_nearby_aux(int segnum, int object_type, int object_id, int depth)
{
	int	i;

	if (depth == 0)
		return 0;

	if (segment_contains_object(object_type, object_id, segnum))
		return 1;

	for (i=0; i<MAX_SIDES_PER_SEGMENT; i++) {
		int	seg2 = Segments[segnum].children[i];

		if (seg2 != -1)
			if (object_nearby_aux(seg2, object_type, object_id, depth-1))
				return 1;
	}

	return 0;
}


//	------------------------------------------------------------------------------------------------------
//	Return true if some powerup is nearby (within 3 segments).
int weapon_nearby(object *objp, int weapon_id)
{
	return object_nearby_aux(objp->segnum, OBJ_POWERUP, weapon_id, 3);
}

//	------------------------------------------------------------------------------------------------------
void maybe_replace_powerup_with_energy(object *del_obj)
{
	int	weapon_index=-1;

	if (del_obj->contains_type != OBJ_POWERUP)
		return;

	if (del_obj->contains_id == POW_CLOAK) {
		if (weapon_nearby(del_obj, del_obj->contains_id)) {
			mprintf((0, "Bashing cloak into nothing because there's one nearby.\n"));
			del_obj->contains_count = 0;
		}
		return;
	}
	switch (del_obj->contains_id) {
		case POW_VULCAN_WEAPON:			weapon_index = VULCAN_INDEX;		break;
		case POW_SPREADFIRE_WEAPON:	weapon_index = SPREADFIRE_INDEX;	break;
#ifndef SHAREWARE
		case POW_PLASMA_WEAPON:			weapon_index = PLASMA_INDEX;		break;
		case POW_FUSION_WEAPON:			weapon_index = FUSION_INDEX;		break;
#endif
	}

	//	Don't drop vulcan ammo if player maxed out.
	if (((weapon_index == VULCAN_INDEX) || (del_obj->contains_id == POW_VULCAN_AMMO)) && (Players[Player_num].primary_ammo[VULCAN_INDEX] >= VULCAN_AMMO_MAX))
		del_obj->contains_count = 0;
	else if (weapon_index != -1) {
		if ((player_has_weapon(weapon_index, 0) & HAS_WEAPON_FLAG) || weapon_nearby(del_obj, del_obj->contains_id)) {
			if (rand() > 16384) {
				del_obj->contains_count = 1;
				del_obj->contains_type = OBJ_POWERUP;
				if (weapon_index == VULCAN_INDEX) {
					del_obj->contains_id = POW_VULCAN_AMMO;
				} else {
					del_obj->contains_id = POW_ENERGY;
				}
			} else
				del_obj->contains_count = 0;
		}
	} else if (del_obj->contains_id == POW_QUAD_FIRE)
		if ((Players[Player_num].flags & PLAYER_FLAGS_QUAD_LASERS) || weapon_nearby(del_obj, del_obj->contains_id)) {
			if (rand() > 16384) {
				del_obj->contains_count = 1;
				del_obj->contains_type = OBJ_POWERUP;
				del_obj->contains_id = POW_ENERGY;
			} else
				del_obj->contains_count = 0;
		}

	//	If this robot was gated in by the boss and it now contains energy, make it contain nothing,
	//	else the room gets full of energy.
	if ( (del_obj->matcen_creator == BOSS_GATE_MATCEN_NUM) && (del_obj->contains_id == POW_ENERGY) && (del_obj->contains_type == OBJ_POWERUP) ) {
		mprintf((0, "Converting energy powerup to nothing because robot %i gated in by boss.\n", del_obj-Objects));
		del_obj->contains_count = 0;
	}

	// Change multiplayer extra-lives into invulnerability
	if ((Game_mode & GM_MULTI) && (del_obj->contains_id == POW_EXTRA_LIFE))
	{
		del_obj->contains_id = POW_INVULNERABILITY;
	}
}

//	------------------------------------------------------------------------------------------------------
//	Returns created object number.
int object_create_egg(object *objp)
{
	int		objnum;
	object	*obj;
	int		powerup_type, powerup_id, count;
	vms_vector	new_velocity, new_pos;
	fix		old_mag;

	powerup_type = objp->contains_type;
	powerup_id = objp->contains_id;

//	maybe_replace_powerup_with_energy(objp);

//	if (Game_mode & GM_NETWORK) 
//	{
//		char	type_str[16], id_str[16];
//		if (objp->contains_type == OBJ_POWERUP) {
//			if (objp->contains_count > 1)			strcpy(type_str, "powerups");		else			strcpy(type_str, "powerup");
//			strcpy(id_str, Powerup_names[objp->contains_id]);
//		} else if (objp->contains_type == OBJ_ROBOT) {
//			if (objp->contains_count > 1)			strcpy(type_str, "robots");		else			strcpy(type_str, "robot");
//				strcpy(id_str, "something-or-other");
//		} else {
//			strcpy(type_str, "unknown objects");
//			strcpy(id_str, "mysterious");
//		}
//		mprintf(0, "Dropping %i %s %s.\n", objp->contains_count, id_str, type_str);
//	}

	switch (objp->contains_type) {
		case OBJ_POWERUP:
			for (count=0; count<objp->contains_count; count++) {
				int	rand_scale;
				new_velocity = objp->mtype.phys_info.velocity;
				old_mag = vm_vec_mag_quick(&objp->mtype.phys_info.velocity);

				//	We want powerups to move more in network mode.
				if ((Game_mode & GM_MULTI) && !(Game_mode & GM_MULTI_ROBOTS)) {
					rand_scale = 4;
					//	extra life powerups are converted to invulnerability in multiplayer, for what is an extra life, anyway?
					if (objp->contains_id == POW_EXTRA_LIFE)
						objp->contains_id = POW_INVULNERABILITY;
				} else
					rand_scale = 2;

				new_velocity.x += fixmul(old_mag+F1_0*32, rand()*rand_scale - 16384*rand_scale);
				new_velocity.y += fixmul(old_mag+F1_0*32, rand()*rand_scale - 16384*rand_scale);
				new_velocity.z += fixmul(old_mag+F1_0*32, rand()*rand_scale - 16384*rand_scale);

				// Give keys zero velocity so they can be tracked better in multi

				if ((Game_mode & GM_MULTI) && (objp->contains_id >= POW_KEY_BLUE) && (objp->contains_id <= POW_KEY_GOLD))
					vm_vec_zero(&new_velocity);

				new_pos = objp->pos;
//				new_pos.x += (rand()-16384)*8;
//				new_pos.y += (rand()-16384)*8;
//				new_pos.z += (rand()-16384)*8;

				#ifdef NETWORK
				if (Game_mode & GM_MULTI)
				{	
					if (Net_create_loc >= MAX_NET_CREATE_OBJECTS)
					{
					 	mprintf( (0, "Not enough slots to drop all powerups!\n" ));
						return (-1);
					}
				}
				#endif

				objnum = obj_create( objp->contains_type, objp->contains_id, objp->segnum, &new_pos, &vmd_identity_matrix, Powerup_info[objp->contains_id].size, CT_POWERUP, MT_PHYSICS, RT_POWERUP);

				if (objnum < 0 ) {
					mprintf((1, "Can't create object in object_create_egg.  Aborting.\n"));
					Int3();
					return objnum;
				}

				#ifdef NETWORK
				if (Game_mode & GM_MULTI)
				{
					Net_create_objnums[Net_create_loc++] = objnum;
				}
				#endif

				obj = &Objects[objnum];

				obj->mtype.phys_info.velocity = new_velocity;

//				mprintf(0, "Created powerup, object #%i, velocity = %7.3f %7.3f %7.3f\n", objnum, f2fl(new_velocity.x), f2fl(new_velocity.y), f2fl(new_velocity.z));
//				mprintf(0, "                             pos = x=%d y=%d z=%d\n", obj->pos.x, obj->pos.y, obj->pos.z);

				obj->mtype.phys_info.drag = 512;	//1024;
				obj->mtype.phys_info.mass = F1_0;

				obj->mtype.phys_info.flags = PF_BOUNCE;

				obj->rtype.vclip_info.vclip_num = Powerup_info[obj->id].vclip_num;
				obj->rtype.vclip_info.frametime = Vclip[obj->rtype.vclip_info.vclip_num].frame_time;
				obj->rtype.vclip_info.framenum = 0;

				switch (obj->id) {
					case POW_MISSILE_1:
					case POW_MISSILE_4:
					case POW_SHIELD_BOOST:
					case POW_ENERGY:
						obj->lifeleft = (rand() + F1_0*3) * 64;		//	Lives for 3 to 3.5 binary minutes (a binary minute is 64 seconds)
						if (Game_mode & GM_MULTI)
							obj->lifeleft /= 2;
						break;
					default:
//						if (Game_mode & GM_MULTI)
//							obj->lifeleft = (rand() + F1_0*3) * 64;		//	Lives for 5 to 5.5 binary minutes (a binary minute is 64 seconds)
						break;
				}
			}
			break;

		case OBJ_ROBOT:
			for (count=0; count<objp->contains_count; count++) {
				int	rand_scale;
				new_velocity = objp->mtype.phys_info.velocity;
				old_mag = vm_vec_mag_quick(&objp->mtype.phys_info.velocity);

				vm_vec_normalize_quick(&new_velocity);

				//	We want powerups to move more in network mode.
//				if (Game_mode & GM_MULTI)
//					rand_scale = 4;
//				else
					rand_scale = 2;

				new_velocity.x += (rand()-16384)*2;
				new_velocity.y += (rand()-16384)*2;
				new_velocity.z += (rand()-16384)*2;

				vm_vec_normalize_quick(&new_velocity);
				vm_vec_scale(&new_velocity, (F1_0*32 + old_mag) * rand_scale);
				new_pos = objp->pos;
				//	This is dangerous, could be outside mine.
//				new_pos.x += (rand()-16384)*8;
//				new_pos.y += (rand()-16384)*7;
//				new_pos.z += (rand()-16384)*6;

				objnum = obj_create(OBJ_ROBOT, objp->contains_id, objp->segnum, &new_pos, &vmd_identity_matrix, Polygon_models[Robot_info[ObjId[objp->contains_type]].model_num].rad, CT_AI, MT_PHYSICS, RT_POLYOBJ);

mprintf((0, "Object %2i, Scatter vector = [%7.3f %7.3f %7.3f]\n", objnum, f2fl(new_velocity.x), f2fl(new_velocity.y), f2fl(new_velocity.z)));

				if ( objnum < 0 ) {
					mprintf((1, "Can't create object in object_create_egg, robots.  Aborting.\n"));
					Int3();
					return objnum;
				}

				#ifdef NETWORK
				if (Game_mode & GM_MULTI)
				{
					Net_create_objnums[Net_create_loc++] = objnum;
				}
				#endif

				obj = &Objects[objnum];

				//Set polygon-object-specific data 

				obj->rtype.pobj_info.model_num = Robot_info[obj->id].model_num;
				obj->rtype.pobj_info.subobj_flags = 0;

				//set Physics info
		
				obj->mtype.phys_info.velocity = new_velocity;

				obj->mtype.phys_info.mass = Robot_info[obj->id].mass;
				obj->mtype.phys_info.drag = Robot_info[obj->id].drag;

				obj->mtype.phys_info.flags |= (PF_LEVELLING);

				obj->shields = Robot_info[obj->id].strength;

				obj->ctype.ai_info.behavior = AIB_NORMAL;
				Ai_local_info[obj-Objects].player_awareness_type = PA_WEAPON_ROBOT_COLLISION;
				Ai_local_info[obj-Objects].player_awareness_time = F1_0*3;
				obj->ctype.ai_info.CURRENT_STATE = AIS_LOCK;
				obj->ctype.ai_info.GOAL_STATE = AIS_LOCK;
				obj->ctype.ai_info.REMOTE_OWNER = -1;
			}

			break;

		default:
			Error("Error: Illegal type (%i) in object spawning.\n", objp->contains_type);
	}

	return objnum;
}

//	-------------------------------------------------------------------------------------------------------
//	Put count objects of type type (eg, powerup), id = id (eg, energy) into *objp, then drop them!  Yippee!
//	Returns created object number.
int call_object_create_egg(object *objp, int count, int type, int id)
{
	if (count > 0) {
		objp->contains_count = count;
		objp->contains_type = type;
		objp->contains_id = id;
		return object_create_egg(objp);
	}

	return -1;
}

//what vclip does this explode with?
int get_explosion_vclip(object *obj,int stage)
{
	if (obj->type==OBJ_ROBOT) {

		if (stage==0 && Robot_info[obj->id].exp1_vclip_num>-1)
				return Robot_info[obj->id].exp1_vclip_num;
		else if (stage==1 && Robot_info[obj->id].exp2_vclip_num>-1)
				return Robot_info[obj->id].exp2_vclip_num;

	}
	else if (obj->type==OBJ_PLAYER && Player_ship->expl_vclip_num>-1)
			return Player_ship->expl_vclip_num;

	return VCLIP_SMALL_EXPLOSION;		//default
}

//blow up a polygon model
void explode_model(object *obj)
{
	Assert(obj->render_type == RT_POLYOBJ);

	if (Dying_modelnums[obj->rtype.pobj_info.model_num] != -1)
		obj->rtype.pobj_info.model_num = Dying_modelnums[obj->rtype.pobj_info.model_num];

	if (Polygon_models[obj->rtype.pobj_info.model_num].n_models > 1) {
		int i;

		for (i=1;i<Polygon_models[obj->rtype.pobj_info.model_num].n_models;i++)
			object_create_debris(obj,i);

		//make parent object only draw center part
		obj->rtype.pobj_info.subobj_flags=1;
	}
}

//if the object has a destroyed model, switch to it.  Otherwise, delete it.
void maybe_delete_object(object *del_obj)
{
	if (Dead_modelnums[del_obj->rtype.pobj_info.model_num] != -1) {
		del_obj->rtype.pobj_info.model_num = Dead_modelnums[del_obj->rtype.pobj_info.model_num];
		del_obj->flags |= OF_DESTROYED;
	}
	else {		//normal, multi-stage explosion
		if (del_obj->type == OBJ_PLAYER)
			del_obj->render_type = RT_NONE;
		else
			del_obj->flags |= OF_SHOULD_BE_DEAD;
	}
}

//	-------------------------------------------------------------------------------------------------------
//blow up an object.  Takes the object to destroy, and the point of impact
void explode_object(object *hitobj,fix delay_time)
{
	if (hitobj->flags & OF_EXPLODING) return;

	if (delay_time) {		//wait a little while before creating explosion
		int objnum;
		object *obj;

		//create a placeholder object to do the delay, with id==-1

		objnum = obj_create( OBJ_FIREBALL,-1,hitobj->segnum,&hitobj->pos,&vmd_identity_matrix,0,
						CT_EXPLOSION,MT_NONE,RT_NONE);
	
		if (objnum < 0 ) {
			maybe_delete_object(hitobj);		//no explosion, die instantly
			mprintf((1,"Couldn't start explosion, deleting object now\n"));
			Int3();
			return;
		}
	
		obj = &Objects[objnum];
	
		//now set explosion-specific data
	
		obj->lifeleft = delay_time;
		obj->ctype.expl_info.delete_objnum = hitobj-Objects;
#ifndef NDEBUG
		if (obj->ctype.expl_info.delete_objnum < 0)
		 Int3(); // See Rob!
#endif
		obj->ctype.expl_info.delete_time = -1;
		obj->ctype.expl_info.spawn_time = 0;

	}
	else {
		object *expl_obj;
		int vclip_num;

		vclip_num = get_explosion_vclip(hitobj,0);

		expl_obj = object_create_explosion(hitobj->segnum, &hitobj->pos, fixmul(hitobj->size,EXPLOSION_SCALE), vclip_num );
	
		if (! expl_obj) {
			maybe_delete_object(hitobj);		//no explosion, die instantly
			mprintf((0,"Couldn't start explosion, deleting object now\n"));
			return;
		}

		//don't make debris explosions have physics, because they often
		//happen when the debris has hit the wall, so the fireball is trying
		//to move into the wall, which shows off FVI problems.   	
		if (hitobj->type!=OBJ_DEBRIS && hitobj->movement_type==MT_PHYSICS) {
			expl_obj->movement_type = MT_PHYSICS;
			expl_obj->mtype.phys_info = hitobj->mtype.phys_info;
		}
	
		if (hitobj->render_type==RT_POLYOBJ && hitobj->type!=OBJ_DEBRIS)
			explode_model(hitobj);

		maybe_delete_object(hitobj);
	}

	hitobj->flags |= OF_EXPLODING;		//say that this is blowing up
	hitobj->control_type = CT_NONE;		//become inert while exploding

}


//do whatever needs to be done for this piece of debris for this frame
void do_debris_frame(object *obj)
{
	Assert(obj->control_type == CT_DEBRIS);

	if (obj->lifeleft < 0)
		explode_object(obj,0);

}

//do whatever needs to be done for this explosion for this frame
void do_explosion_sequence(object *obj)
{
	Assert(obj->control_type == CT_EXPLOSION);

	//mprintf( 0, "Object %d life left is %d\n", obj-Objects, obj->lifeleft );

	//See if we should die of old age
	if (obj->lifeleft <= 0 ) 	{	// We died of old age
		obj->flags |= OF_SHOULD_BE_DEAD;
		obj->lifeleft = 0;
	}

	//See if we should create a secondary explosion
	if (obj->lifeleft <= obj->ctype.expl_info.spawn_time) {
		object *expl_obj,*del_obj;
		int vclip_num;
		vms_vector *spawn_pos;

		if ((obj->ctype.expl_info.delete_objnum < 0) || (obj->ctype.expl_info.delete_objnum > Highest_object_index)) {
			mprintf((0, "Illegal value for delete_objnum in fireball.c\n"));
			Int3(); // get Rob, please... thanks
			return;
		}

		del_obj = &Objects[obj->ctype.expl_info.delete_objnum];

		spawn_pos = &del_obj->pos;

		Assert(del_obj->type==OBJ_ROBOT || del_obj->type==OBJ_CLUTTER || del_obj->type==OBJ_CNTRLCEN || del_obj->type == OBJ_PLAYER);
		Assert(del_obj->segnum != -1);

		vclip_num = get_explosion_vclip(del_obj,1);

		expl_obj = object_create_explosion( del_obj->segnum, spawn_pos, fixmul(del_obj->size, EXPLOSION_SCALE), vclip_num );

		if ((del_obj->contains_count > 0) && !(Game_mode & GM_MULTI)) { // Multiplayer handled outside of this code!!
			//	If dropping a weapon that the player has, drop energy instead, unless it's vulcan, in which case drop vulcan ammo.
			if (del_obj->contains_type == OBJ_POWERUP)
				maybe_replace_powerup_with_energy(del_obj);
			object_create_egg(del_obj);
		} else if ((del_obj->type == OBJ_ROBOT) && !(Game_mode & GM_MULTI)) { // Multiplayer handled outside this code!!
			robot_info	*robptr = &Robot_info[del_obj->id];
			if (robptr->contains_count) {
				if (((rand() * 16) >> 15) < robptr->contains_prob) {
					del_obj->contains_count = ((rand() * robptr->contains_count) >> 15) + 1;
					del_obj->contains_type = robptr->contains_type;
					del_obj->contains_id = robptr->contains_id;
					maybe_replace_powerup_with_energy(del_obj);
					object_create_egg(del_obj);
				}
			}
		}

		if ( Robot_info[del_obj->id].exp2_sound_num > -1 )
			digi_link_sound_to_pos( Robot_info[del_obj->id].exp2_sound_num, del_obj->segnum, 0, spawn_pos, 0, F1_0 );
			//PLAY_SOUND_3D( Robot_info[del_obj->id].exp2_sound_num, spawn_pos, del_obj->segnum  );

		// mprintf( 0, "Spawned an explosion of type %d\n", Robot_info[del_obj->id].exp2_vclip_num );

		//mprintf( 0, "Object %d spawned.\n", obj-Objects );
		//mprintf( 0, "Explosion at %d,%d,%d\n", obj->pos.x, obj->pos.y, obj->pos.z );
		//mprintf( 0, "Explosion at %d,%d,%d\n", obj->pos.x, obj->pos.y, obj->pos.z );
		//mprintf( 0, "Spawned exp at %d,%d,%d\n", expl_obj->pos.x, expl_obj->pos.y, expl_obj->pos.z );

		obj->ctype.expl_info.spawn_time = -1;

		//make debris
		if (del_obj->render_type==RT_POLYOBJ)
			explode_model(del_obj);		//explode a polygon model

		//set some parm in explosion
		if (expl_obj) {

			if (del_obj->movement_type == MT_PHYSICS) {
				expl_obj->movement_type = MT_PHYSICS;
				expl_obj->mtype.phys_info = del_obj->mtype.phys_info;
			}

			expl_obj->ctype.expl_info.delete_time = expl_obj->lifeleft/2;
			expl_obj->ctype.expl_info.delete_objnum = del_obj-Objects;
#ifndef NDEBUG
			if (obj->ctype.expl_info.delete_objnum < 0)
		  		Int3(); // See Rob!
#endif

		}
		else {
			maybe_delete_object(del_obj);
			mprintf((0,"Couldn't create secondary explosion, deleting object now\n"));
		}

	}

	//See if we should delete an object
	if (obj->lifeleft <= obj->ctype.expl_info.delete_time) {
		object *del_obj = &Objects[obj->ctype.expl_info.delete_objnum];

		obj->ctype.expl_info.delete_time = -1;

		maybe_delete_object(del_obj);
	}
}

typedef struct expl_wall {
	int segnum,sidenum;
	fix time;
} expl_wall;

#define MAX_EXPLODING_WALLS 			10
#define EXPL_WALL_TIME					(f1_0)
#define EXPL_WALL_TOTAL_FIREBALLS	32
#define EXPL_WALL_FIREBALL_SIZE 		0x48000	//smallest size

expl_wall expl_wall_list[MAX_EXPLODING_WALLS];
//--unused-- int n_exploding_walls;

void init_exploding_walls()
{
	int i;

	for (i=0;i<MAX_EXPLODING_WALLS;i++)
		expl_wall_list[i].segnum = -1;
}

//explode the given wall
void explode_wall(int segnum,int sidenum)
{
	int i;
	vms_vector pos;

	//find a free slot

	for (i=0;i<MAX_EXPLODING_WALLS && expl_wall_list[i].segnum != -1;i++);

	if (i==MAX_EXPLODING_WALLS) {		//didn't find slot.
		mprintf((0,"Couldn't find free slot for exploding wall!\n"));
		Int3();
		return;
	}

	expl_wall_list[i].segnum	= segnum;
	expl_wall_list[i].sidenum	= sidenum;
	expl_wall_list[i].time		= 0;

	//play one long sound for whole door wall explosion
	compute_center_point_on_side(&pos,&Segments[segnum],sidenum);
	digi_link_sound_to_pos( SOUND_EXPLODING_WALL,segnum, sidenum, &pos, 0, F1_0 );

}

//handle walls for this frame
//note: this wall code assumes the wall is not triangulated
void do_exploding_wall_frame()
{
	int i;

	for (i=0;i<MAX_EXPLODING_WALLS;i++) {
		int segnum = expl_wall_list[i].segnum;

		if (segnum != -1) {
			int sidenum = expl_wall_list[i].sidenum;
			fix oldfrac,newfrac;
			int old_count,new_count,e;		//n,

			oldfrac = fixdiv(expl_wall_list[i].time,EXPL_WALL_TIME);

			expl_wall_list[i].time += FrameTime;
			if (expl_wall_list[i].time > EXPL_WALL_TIME)
				expl_wall_list[i].time = EXPL_WALL_TIME;

			if (expl_wall_list[i].time>(EXPL_WALL_TIME*3)/4) {
				segment *seg,*csegp;
				int cside,a,n;

				seg = &Segments[segnum];

				a = Walls[seg->sides[sidenum].wall_num].clip_num;
				n = WallAnims[a].num_frames;

				csegp = &Segments[seg->children[sidenum]];
				cside = find_connect_side(seg, csegp);

				wall_set_tmap_num(seg,sidenum,csegp,cside,a,n-1);
			}

			newfrac = fixdiv(expl_wall_list[i].time,EXPL_WALL_TIME);

			old_count = f2i(EXPL_WALL_TOTAL_FIREBALLS * fixmul(oldfrac,oldfrac));
			new_count = f2i(EXPL_WALL_TOTAL_FIREBALLS * fixmul(newfrac,newfrac));

			//n = new_count - old_count;

//			mprintf(0,"Creating %d new explosions\n",new_count-old_count);

			//now create all the next explosions

			for (e=old_count;e<new_count;e++) {
				short			vertnum_list[4];
				vms_vector	*v0,*v1,*v2;
				vms_vector	vv0,vv1,pos;
				fix			size;

				//calc expl position

				get_side_verts(vertnum_list,segnum,sidenum);

				v0 = &Vertices[vertnum_list[0]];
				v1 = &Vertices[vertnum_list[1]];
				v2 = &Vertices[vertnum_list[2]];

				vm_vec_sub(&vv0,v0,v1);
				vm_vec_sub(&vv1,v2,v1);

				vm_vec_scale_add(&pos,v1,&vv0,rand()*2);
				vm_vec_scale_add2(&pos,&vv1,rand()*2);

				size = EXPL_WALL_FIREBALL_SIZE + (2*EXPL_WALL_FIREBALL_SIZE * e / EXPL_WALL_TOTAL_FIREBALLS);

				//fireballs start away from door, with subsequent ones getting closer
				#ifdef COMPACT_SEGS	
					{
					vms_vector _vn;
					get_side_normal(&Segments[segnum], sidenum, 0, &_vn );
					vm_vec_scale_add2(&pos,&_vn,size*(EXPL_WALL_TOTAL_FIREBALLS-e)/EXPL_WALL_TOTAL_FIREBALLS);
					}
				#else
					vm_vec_scale_add2(&pos,&Segments[segnum].sides[sidenum].normals[0],size*(EXPL_WALL_TOTAL_FIREBALLS-e)/EXPL_WALL_TOTAL_FIREBALLS);
				#endif

				if (e & 3)		//3 of 4 are normal
					object_create_explosion(expl_wall_list[i].segnum,&pos,size,VCLIP_SMALL_EXPLOSION);
				else
					object_create_badass_explosion( NULL, expl_wall_list[i].segnum, &pos, 
					size, 
					VCLIP_SMALL_EXPLOSION,
					i2f(4),		// damage strength
					i2f(20),		//	damage radius
					i2f(50),		//	damage force
					-1		//	parent id
					);


			} 

			if (expl_wall_list[i].time >= EXPL_WALL_TIME)
				expl_wall_list[i].segnum = -1;	//flag this slot as free

		}
	}

}


