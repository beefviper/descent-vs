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
 * $Source: f:/miner/source/main/rcs/gamemine.c $
 * $Revision: 2.2 $
 * $Author: john $
 * $Date: 1995/03/06 15:23:14 $
 * 
 * Functions for loading mines in the game
 * 
 * $Log: gamemine.c $
 * Revision 2.2  1995/03/06  15:23:14  john
 * New screen techniques.
 * 
 * Revision 2.1  1995/02/27  13:13:37  john
 * Removed floating point.
 * 
 * Revision 2.0  1995/02/27  11:27:45  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 * 
 * Revision 1.70  1995/02/13  20:35:09  john
 * Lintized
 * 
 * Revision 1.69  1995/02/07  17:12:03  rob
 * Added ifdef's for Editor.
 * 
 * Revision 1.68  1995/02/07  16:51:48  mike
 * fix gray rock josh problem.
 * 
 * Revision 1.67  1995/02/01  15:46:26  yuan
 * Fixed matcen_nums.
 * 
 * Revision 1.66  1995/01/19  15:19:28  mike
 * new super-compressed registered file format.
 * 
 * Revision 1.65  1994/12/10  16:44:59  matt
 * Added debugging code to track down door that turns into rock
 * 
 * Revision 1.64  1994/12/10  14:58:24  yuan
 * *** empty log message ***
 * 
 * Revision 1.63  1994/12/08  17:19:10  yuan
 * Cfiling stuff.
 * 
 * Revision 1.62  1994/12/07  14:05:33  yuan
 * Fixed wall assert problem... Bashed highest_segment
 * _index before WALL_IS_DOORWAY check.
 * 
 * Revision 1.61  1994/11/27  23:14:52  matt
 * Made changes for new mprintf calling convention
 * 
 * Revision 1.60  1994/11/27  18:05:20  matt
 * Compile out LVL reader when editor compiled out
 * 
 * Revision 1.59  1994/11/26  22:51:45  matt
 * Removed editor-only fields from segment structure when editor is compiled
 * out, and padded segment structure to even multiple of 4 bytes.
 * 
 * Revision 1.58  1994/11/26  21:48:02  matt
 * Fixed saturation in short light value 
 * 
 * Revision 1.57  1994/11/20  22:11:49  mike
 * comment out an apparently unnecessary call to fuelcen_reset().
 * 
 * Revision 1.56  1994/11/18  21:56:42  john
 * Added a better, leaner pig format.
 * 
 * Revision 1.55  1994/11/17  20:09:18  john
 * Added new compiled level format.
 * 
 * Revision 1.54  1994/11/17  15:40:17  mike
 * Comment out mprintf which was causing important information to scroll away.
 * 
 * Revision 1.53  1994/11/17  14:56:37  mike
 * moved segment validation functions from editor to main.
 * 
 * Revision 1.52  1994/11/17  11:39:35  matt
 * Ripped out code to load old mines
 * 
 * Revision 1.51  1994/11/14  20:47:53  john
 * Attempted to strip out all the code in the game 
 * directory that uses any ui code.
 * 
 * Revision 1.50  1994/11/14  16:05:38  matt
 * Fixed, maybe, again, errors when can't find texture during remap
 * 
 * Revision 1.49  1994/11/14  14:34:03  matt
 * Fixed up handling when textures can't be found during remap
 * 
 * Revision 1.48  1994/11/14  13:01:55  matt
 * Added Int3() when can't find texture
 * 
 * Revision 1.47  1994/10/30  14:12:21  mike
 * rip out local segments stuff.
 * 
 * Revision 1.46  1994/10/27  19:43:07  john
 * Disable the piglet option.
 * 
 * Revision 1.45  1994/10/27  18:51:42  john
 * Added -piglet option that only loads needed textures for a 
 * mine.  Only saved ~1MB, and code still doesn't free textures
 * before you load a new mine.
 * 
 * Revision 1.44  1994/10/20  12:47:22  matt
 * Replace old save files (MIN/SAV/HOT) with new LVL files
 * 
 * Revision 1.43  1994/10/19  16:46:40  matt
 * Made tmap overrides for robots remap texture numbers
 * 
 * Revision 1.42  1994/10/03  23:37:01  mike
 * Adapt to changed fuelcen_activate parameters.
 * 
 * Revision 1.41  1994/09/23  22:14:49  matt
 * Took out obsolete structure fields
 * 
 * Revision 1.40  1994/08/01  11:04:11  yuan
 * New materialization centers.
 * 
 * Revision 1.39  1994/07/21  19:01:47  mike
 * Call Lsegment stuff.
 * 
 * 
 */

#pragma off (unreferenced)
static char rcsid[] = "$Id: gamemine.c 2.2 1995/03/06 15:23:14 john Exp $";
#pragma on (unreferenced)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "mono.h"

#include "inferno.h"
#include "segment.h"
#include "textures.h"
#include "wall.h"
#include "object.h"
#include "gamemine.h"
#include "error.h"
#include "gameseg.h"
#include "switch.h"

#include "game.h"
#include "newmenu.h"

#ifdef EDITOR
#include "editor\editor.h"
#endif

#include "cfile.h"		
#include "fuelcen.h"

#include "hash.h"
#include "key.h"
#include "piggy.h"

#define REMOVE_EXT(s)  (*(strchr( (s), '.' ))='\0')

struct mtfi mine_top_fileinfo;    // Should be same as first two fields below...
struct mfi mine_fileinfo;
struct mh mine_header;
struct me mine_editor;

// Function Prototypes
int load_mine_data_compiled_new(CFILE* LoadFile);

int CreateDefaultNewSegment();

#ifdef EDITOR

static char old_tmap_list[MAX_TEXTURES][13];
short tmap_xlate_table[MAX_TEXTURES];
static short tmap_times_used[MAX_TEXTURES];

// -----------------------------------------------------------------------------
//loads from an already-open file
// returns 0=everything ok, 1=old version, -1=error
int load_mine_data(CFILE *LoadFile)
{
	int   i, j;
	short tmap_xlate;
	int 	translate;
	char 	*temptr;
	int	mine_start = cftell(LoadFile);

	fuelcen_reset();

	for (i=0; i<MAX_TEXTURES; i++ )
		tmap_times_used[i] = 0;
	
	#ifdef EDITOR
	// Create a new mine to initialize things.
	//texpage_goto_first();
	create_new_mine();
	#endif

	//===================== READ FILE INFO ========================

	// These are the default values... version and fileinfo_sizeof
	// don't have defaults.
	mine_fileinfo.header_offset     =   -1;
	mine_fileinfo.header_size       =   sizeof(mine_header);
	mine_fileinfo.editor_offset     =   -1;
	mine_fileinfo.editor_size       =   sizeof(mine_editor);
	mine_fileinfo.vertex_offset     =   -1;
	mine_fileinfo.vertex_howmany    =   0;
	mine_fileinfo.vertex_sizeof     =   sizeof(vms_vector);
	mine_fileinfo.segment_offset    =   -1;
	mine_fileinfo.segment_howmany   =   0;
	mine_fileinfo.segment_sizeof    =   sizeof(segment);
	mine_fileinfo.newseg_verts_offset     =   -1;
	mine_fileinfo.newseg_verts_howmany    =   0;
	mine_fileinfo.newseg_verts_sizeof     =   sizeof(vms_vector);
	mine_fileinfo.group_offset		  =	-1;
	mine_fileinfo.group_howmany	  =	0;
	mine_fileinfo.group_sizeof		  =	sizeof(group);
	mine_fileinfo.texture_offset    =   -1;
	mine_fileinfo.texture_howmany   =   0;
	mine_fileinfo.texture_sizeof    =   13;  // num characters in a name
 	mine_fileinfo.walls_offset		  =	-1;
	mine_fileinfo.walls_howmany	  =	0;
	mine_fileinfo.walls_sizeof		  =	sizeof(wall);  
 	mine_fileinfo.triggers_offset	  =	-1;
	mine_fileinfo.triggers_howmany  =	0;
	mine_fileinfo.triggers_sizeof	  =	sizeof(trigger);  
	mine_fileinfo.object_offset		=	-1;
	mine_fileinfo.object_howmany		=	1;
	mine_fileinfo.object_sizeof		=	sizeof(object);  

	// Read in mine_top_fileinfo to get size of saved fileinfo.
	
	memset( &mine_top_fileinfo, 0, sizeof(mine_top_fileinfo) );

	if (cfseek( LoadFile, mine_start, SEEK_SET ))
		Error( "Error moving to top of file in gamemine.c" );

	if (cfread( &mine_top_fileinfo, sizeof(mine_top_fileinfo), 1, LoadFile )!=1)
		Error( "Error reading mine_top_fileinfo in gamemine.c" );

	if (mine_top_fileinfo.fileinfo_signature != 0x2884)
		return -1;

	// Check version number
	if (mine_top_fileinfo.fileinfo_version < COMPATIBLE_VERSION )
		return -1;

	// Now, Read in the fileinfo
	if (cfseek( LoadFile, mine_start, SEEK_SET ))
		Error( "Error seeking to top of file in gamemine.c" );

	if (cfread( &mine_fileinfo, mine_top_fileinfo.fileinfo_sizeof, 1, LoadFile )!=1)
		Error( "Error reading mine_fileinfo in gamemine.c" );

	//===================== READ HEADER INFO ========================

	// Set default values.
	mine_header.num_vertices        =   0;
	mine_header.num_segments        =   0;

	if (mine_fileinfo.header_offset > -1 )
	{
		if (cfseek( LoadFile, mine_fileinfo.header_offset, SEEK_SET ))
			Error( "Error seeking to header_offset in gamemine.c" );
	
		if (cfread( &mine_header, mine_fileinfo.header_size, 1, LoadFile )!=1)
			Error( "Error reading mine_header in gamemine.c" );
	}

	//===================== READ EDITOR INFO ==========================

	// Set default values
	mine_editor.current_seg         =   0;
	mine_editor.newsegment_offset   =   -1; // To be written
	mine_editor.newsegment_size     =   sizeof(segment);
	mine_editor.Curside             =   0;
	mine_editor.Markedsegp          =   -1;
	mine_editor.Markedside          =   0;

	if (mine_fileinfo.editor_offset > -1 )
	{
		if (cfseek( LoadFile, mine_fileinfo.editor_offset, SEEK_SET ))
			Error( "Error seeking to editor_offset in gamemine.c" );
	
		if (cfread( &mine_editor, mine_fileinfo.editor_size, 1, LoadFile )!=1)
			Error( "Error reading mine_editor in gamemine.c" );
	}

	//===================== READ TEXTURE INFO ==========================

	if ( (mine_fileinfo.texture_offset > -1) && (mine_fileinfo.texture_howmany > 0))
	{
		if (cfseek( LoadFile, mine_fileinfo.texture_offset, SEEK_SET ))
			Error( "Error seeking to texture_offset in gamemine.c" );

		for (i=0; i< mine_fileinfo.texture_howmany; i++ )
		{
			if (cfread( &old_tmap_list[i], mine_fileinfo.texture_sizeof, 1, LoadFile )!=1)
				Error( "Error reading old_tmap_list[i] in gamemine.c" );
		}
	}

	//=============== GENERATE TEXTURE TRANSLATION TABLE ===============

	translate = 0;
	
	Assert (NumTextures < MAX_TEXTURES);

	{
		hashtable ht;
	
		hashtable_init( &ht, NumTextures );
	
		// Remove all the file extensions in the textures list
	
		for (i=0;i<NumTextures;i++)	{
			temptr = strchr(TmapInfo[i].filename, '.');
			if (temptr) *temptr = '\0';
			hashtable_insert( &ht, TmapInfo[i].filename, i );
		}
	
		// For every texture, search through the texture list
		// to find a matching name.
		for (j=0;j<mine_fileinfo.texture_howmany;j++) 	{
			// Remove this texture name's extension
			temptr = strchr(old_tmap_list[j], '.');
			if (temptr) *temptr = '\0';
	
			tmap_xlate_table[j] = hashtable_search( &ht,old_tmap_list[j]);
			if (tmap_xlate_table[j]	< 0 )	{
				//tmap_xlate_table[j] = 0;
				// mprintf( (0, "Couldn't find texture '%s'\n", old_tmap_list[j] ));
				;
			}
			if (tmap_xlate_table[j] != j ) translate = 1;
			if (tmap_xlate_table[j] >= 0)
				tmap_times_used[tmap_xlate_table[j]]++;
		}
	
		{
			int count = 0;
			for (i=0; i<MAX_TEXTURES; i++ )
				if (tmap_times_used[i])
					count++;
			mprintf( (0, "This mine has %d unique textures in it (~%d KB)\n", count, (count*4096) /1024 ));
		}
	
		mprintf( (0, "Translate=%d\n", translate ));
	
		hashtable_free( &ht );
	}

	//====================== READ VERTEX INFO ==========================

	// New check added to make sure we don't read in too many vertices.
	if ( mine_fileinfo.vertex_howmany > MAX_VERTICES )
		{
		mprintf((0, "Num vertices exceeds maximum.  Loading MAX %d vertices\n", MAX_VERTICES));
		mine_fileinfo.vertex_howmany = MAX_VERTICES;
		}

	if ( (mine_fileinfo.vertex_offset > -1) && (mine_fileinfo.vertex_howmany > 0))
	{
		if (cfseek( LoadFile, mine_fileinfo.vertex_offset, SEEK_SET ))
			Error( "Error seeking to vertex_offset in gamemine.c" );

		for (i=0; i< mine_fileinfo.vertex_howmany; i++ )
		{
			// Set the default values for this vertex
			Vertices[i].x = 1;
			Vertices[i].y = 1;
			Vertices[i].z = 1;

			if (cfread( &Vertices[i], mine_fileinfo.vertex_sizeof, 1, LoadFile )!=1)
				Error( "Error reading Vertices[i] in gamemine.c" );
		}
	}

	//==================== READ SEGMENT INFO ===========================

	// New check added to make sure we don't read in too many segments.
	if ( mine_fileinfo.segment_howmany > MAX_SEGMENTS ) {
		mprintf((0, "Num segments exceeds maximum.  Loading MAX %d segments\n", MAX_SEGMENTS));
		mine_fileinfo.segment_howmany = MAX_SEGMENTS;
	}

	// [commented out by mk on 11/20/94 (weren't we supposed to hit final in October?) because it looks redundant.  I think I'll test it now...]  fuelcen_reset();

	if ( (mine_fileinfo.segment_offset > -1) && (mine_fileinfo.segment_howmany > 0))	{

		if (cfseek( LoadFile, mine_fileinfo.segment_offset,SEEK_SET ))

			Error( "Error seeking to segment_offset in gamemine.c" );

		Highest_segment_index = mine_fileinfo.segment_howmany-1;

		for (i=0; i< mine_fileinfo.segment_howmany; i++ ) {
			segment v16_seg;

			// Set the default values for this segment (clear to zero )
			//memset( &Segments[i], 0, sizeof(segment) );

			if (mine_top_fileinfo.fileinfo_version >= 16) {

				Assert(mine_fileinfo.segment_sizeof == sizeof(v16_seg));

				if (cfread( &v16_seg, mine_fileinfo.segment_sizeof, 1, LoadFile )!=1)
					Error( "Error reading segments in gamemine.c" );

			}				
			else 
				Error("Invalid mine version");

			Segments[i] = v16_seg;

			Segments[i].objects = -1;
			#ifdef EDITOR
			Segments[i].group = -1;
			#endif

			if (mine_top_fileinfo.fileinfo_version < 15) {	//used old uvl ranges
				int sn,uvln;

				for (sn=0;sn<MAX_SIDES_PER_SEGMENT;sn++)
					for (uvln=0;uvln<4;uvln++) {
						Segments[i].sides[sn].uvls[uvln].u /= 64;
						Segments[i].sides[sn].uvls[uvln].v /= 64;
						Segments[i].sides[sn].uvls[uvln].l /= 32;
					}
			}

			fuelcen_activate( &Segments[i], Segments[i].special );

			if (translate == 1)
				for (j=0;j<MAX_SIDES_PER_SEGMENT;j++) {
					unsigned short orient;
					tmap_xlate = Segments[i].sides[j].tmap_num;
					Segments[i].sides[j].tmap_num = tmap_xlate_table[tmap_xlate];
					if ((WALL_IS_DOORWAY(&Segments[i],j) & WID_RENDER_FLAG))
						if (Segments[i].sides[j].tmap_num < 0)	{
							mprintf( (0, "Couldn't find texture '%s' for Segment %d, side %d\n", old_tmap_list[tmap_xlate],i,j));
							Int3();
							Segments[i].sides[j].tmap_num = 0;
						}
					tmap_xlate = Segments[i].sides[j].tmap_num2 & 0x3FFF;
					orient = Segments[i].sides[j].tmap_num2 & (~0x3FFF);
					if (tmap_xlate != 0) {
						int xlated_tmap = tmap_xlate_table[tmap_xlate];

						if ((WALL_IS_DOORWAY(&Segments[i],j) & WID_RENDER_FLAG))
							if (xlated_tmap <= 0)	{
								mprintf( (0, "Couldn't find texture '%s' for Segment %d, side %d\n", old_tmap_list[tmap_xlate],i,j));
								Int3();
								Segments[i].sides[j].tmap_num2 = 0;
							}
						Segments[i].sides[j].tmap_num2 = xlated_tmap | orient;
					}
				}
		}
	}

	//===================== READ NEWSEGMENT INFO =====================

	#ifdef EDITOR

	{		// Default segment created.
		vms_vector	sizevec;
		med_create_new_segment(vm_vec_make(&sizevec,DEFAULT_X_SIZE,DEFAULT_Y_SIZE,DEFAULT_Z_SIZE));		// New_segment = Segments[0];
		//memset( &New_segment, 0, sizeof(segment) );
	}

	if (mine_editor.newsegment_offset > -1)
	{
		if (cfseek( LoadFile, mine_editor.newsegment_offset,SEEK_SET ))
			Error( "Error seeking to newsegment_offset in gamemine.c" );
		if (cfread( &New_segment, mine_editor.newsegment_size,1,LoadFile )!=1)
			Error( "Error reading new_segment in gamemine.c" );
	}

	if ( (mine_fileinfo.newseg_verts_offset > -1) && (mine_fileinfo.newseg_verts_howmany > 0))
	{
		if (cfseek( LoadFile, mine_fileinfo.newseg_verts_offset, SEEK_SET ))
			Error( "Error seeking to newseg_verts_offset in gamemine.c" );
		for (i=0; i< mine_fileinfo.newseg_verts_howmany; i++ )
		{
			// Set the default values for this vertex
			Vertices[NEW_SEGMENT_VERTICES+i].x = 1;
			Vertices[NEW_SEGMENT_VERTICES+i].y = 1;
			Vertices[NEW_SEGMENT_VERTICES+i].z = 1;
			
			if (cfread( &Vertices[NEW_SEGMENT_VERTICES+i], mine_fileinfo.newseg_verts_sizeof,1,LoadFile )!=1)
				Error( "Error reading Vertices[NEW_SEGMENT_VERTICES+i] in gamemine.c" );

			New_segment.verts[i] = NEW_SEGMENT_VERTICES+i;
		}
	}

	#endif
															
	//========================= UPDATE VARIABLES ======================

	#ifdef EDITOR

	// Setting to Markedsegp to NULL ignores Curside and Markedside, which
	// we want to do when reading in an old file.
	
 	Markedside = mine_editor.Markedside;
	Curside = mine_editor.Curside;
	for (i=0;i<10;i++)
		Groupside[i] = mine_editor.Groupside[i];

	if ( mine_editor.current_seg != -1 )
		Cursegp = mine_editor.current_seg + Segments;
	else
 		Cursegp = NULL;

	if (mine_editor.Markedsegp != -1 ) 
		Markedsegp = mine_editor.Markedsegp + Segments;
	else
		Markedsegp = NULL;

	num_groups = 0;
	current_group = -1;

	#endif

	Num_vertices = mine_fileinfo.vertex_howmany;
	Num_segments = mine_fileinfo.segment_howmany;
	Highest_vertex_index = Num_vertices-1;
	Highest_segment_index = Num_segments-1;

	reset_objects(1);		//one object, the player

	#ifdef EDITOR
	Highest_vertex_index = MAX_SEGMENT_VERTICES-1;
	Highest_segment_index = MAX_SEGMENTS-1;
	set_vertex_counts();
	Highest_vertex_index = Num_vertices-1;
	Highest_segment_index = Num_segments-1;

	warn_if_concave_segments();
	#endif

	#ifdef EDITOR
		validate_segment_all();
	#endif

	//create_local_segment_data();

	//gamemine_find_textures();

	if (mine_top_fileinfo.fileinfo_version < MINE_VERSION )
		return 1;		//old version
	else
		return 0;

}
#endif

#define COMPILED_MINE_VERSION 0

int	New_file_format_load = 1;

int load_mine_data_compiled(CFILE *LoadFile)
{
	int i,segnum,sidenum;
	ubyte version;
	short temp_short;
	ushort temp_ushort;

#ifndef SHAREWARE
	if (New_file_format_load)
		return load_mine_data_compiled_new(LoadFile);
#endif

	//	For compiled levels, textures map to themselves, prevent tmap_override always being gray,
	//	bug which Matt and John refused to acknowledge, so here is Mike, fixing it.
	// 
	// Although in a cloud of arrogant glee, he forgot to ifdef it on EDITOR!
	// (Matt told me to write that!)
#ifdef EDITOR
	for (i=0; i<MAX_TEXTURES; i++)
		tmap_xlate_table[i] = i;
#endif
//	memset( Segments, 0, sizeof(segment)*MAX_SEGMENTS );
	fuelcen_reset();

	//=============================== Reading part ==============================
	cfread( &version, sizeof(ubyte), 1, LoadFile );						// 1 byte = compiled version
	Assert( version==COMPILED_MINE_VERSION );
	cfread( &Num_vertices, sizeof(int), 1, LoadFile );					// 4 bytes = Num_vertices
	Assert( Num_vertices <= MAX_VERTICES );
	cfread( &Num_segments, sizeof(int), 1, LoadFile );					// 4 bytes = Num_segments
	Assert( Num_segments <= MAX_SEGMENTS );
	cfread( Vertices, sizeof(vms_vector), Num_vertices, LoadFile );

	for (segnum=0; segnum<Num_segments; segnum++ )	{
		#ifdef EDITOR
		Segments[segnum].segnum = segnum;
		Segments[segnum].group = 0;
		#endif

		// Read short Segments[segnum].children[MAX_SIDES_PER_SEGMENT]
 		cfread( Segments[segnum].children, sizeof(short), MAX_SIDES_PER_SEGMENT, LoadFile );
		// Read short Segments[segnum].verts[MAX_VERTICES_PER_SEGMENT]
		cfread( Segments[segnum].verts, sizeof(short), MAX_VERTICES_PER_SEGMENT, LoadFile );
		Segments[segnum].objects = -1;

		// Read ubyte	Segments[segnum].special
		cfread( &Segments[segnum].special, sizeof(ubyte), 1, LoadFile );
		// Read byte	Segments[segnum].matcen_num
		cfread( &Segments[segnum].matcen_num, sizeof(ubyte), 1, LoadFile );
		// Read short	Segments[segnum].value
		cfread( &Segments[segnum].value, sizeof(short), 1, LoadFile );

		// Read fix	Segments[segnum].static_light (shift down 5 bits, write as short)
		cfread( &temp_ushort, sizeof(temp_ushort), 1, LoadFile );
		Segments[segnum].static_light	= ((fix)temp_ushort) << 4;
		//cfread( &Segments[segnum].static_light, sizeof(fix), 1, LoadFile );
	
		// Read the walls as a 6 byte array
		for (sidenum=0; sidenum<MAX_SIDES_PER_SEGMENT; sidenum++ )	{
			ubyte byte_wallnum;
			Segments[segnum].sides[sidenum].pad = 0;
			cfread( &byte_wallnum, sizeof(ubyte), 1, LoadFile );
			if ( byte_wallnum == 255 )			
				Segments[segnum].sides[sidenum].wall_num = -1;
			else		
				Segments[segnum].sides[sidenum].wall_num = byte_wallnum;
		}

		for (sidenum=0; sidenum<MAX_SIDES_PER_SEGMENT; sidenum++ )	{

			if ( (Segments[segnum].children[sidenum]==-1) || (Segments[segnum].sides[sidenum].wall_num!=-1) )	{
				// Read short Segments[segnum].sides[sidenum].tmap_num;
				cfread( &Segments[segnum].sides[sidenum].tmap_num, sizeof(short), 1, LoadFile );
				// Read short Segments[segnum].sides[sidenum].tmap_num2;
				cfread( &Segments[segnum].sides[sidenum].tmap_num2, sizeof(short), 1, LoadFile );
				// Read uvl Segments[segnum].sides[sidenum].uvls[4] (u,v>>5, write as short, l>>1 write as short)
				for (i=0; i<4; i++ )	{
					cfread( &temp_short, sizeof(short), 1, LoadFile );
					Segments[segnum].sides[sidenum].uvls[i].u = ((fix)temp_short) << 5;
					cfread( &temp_short, sizeof(short), 1, LoadFile );
					Segments[segnum].sides[sidenum].uvls[i].v = ((fix)temp_short) << 5;
					cfread( &temp_ushort, sizeof(temp_ushort), 1, LoadFile );
					Segments[segnum].sides[sidenum].uvls[i].l = ((fix)temp_ushort) << 1;
					//cfread( &Segments[segnum].sides[sidenum].uvls[i].l, sizeof(fix), 1, LoadFile );
				}	
			} else {
				Segments[segnum].sides[sidenum].tmap_num = 0;
				Segments[segnum].sides[sidenum].tmap_num2 = 0;
				for (i=0; i<4; i++ )	{
					Segments[segnum].sides[sidenum].uvls[i].u = 0;
					Segments[segnum].sides[sidenum].uvls[i].v = 0;
					Segments[segnum].sides[sidenum].uvls[i].l = 0;
				}	
			}
		}
	}

	Highest_vertex_index = Num_vertices-1;
	Highest_segment_index = Num_segments-1;

	validate_segment_all();			// Fill in side type and normals.

	// Activate fuelcentes
	for (i=0; i< Num_segments; i++ ) {
		fuelcen_activate( &Segments[i], Segments[i].special );
	}

	reset_objects(1);		//one object, the player

	return 0;
}

#ifndef SHAREWARE
int load_mine_data_compiled_new(CFILE *LoadFile)
{
	int		i,segnum,sidenum;
	ubyte		version;
	short		temp_short;
	ushort	temp_ushort;
	ubyte		bit_mask;

	//	For compiled levels, textures map to themselves, prevent tmap_override always being gray,
	//	bug which Matt and John refused to acknowledge, so here is Mike, fixing it.
#ifdef EDITOR
	for (i=0; i<MAX_TEXTURES; i++)
		tmap_xlate_table[i] = i;
#endif

//	memset( Segments, 0, sizeof(segment)*MAX_SEGMENTS );
	fuelcen_reset();

	//=============================== Reading part ==============================
	cfread( &version, sizeof(ubyte), 1, LoadFile );						// 1 byte = compiled version
	Assert( version==COMPILED_MINE_VERSION );

	cfread( &temp_ushort, sizeof(ushort), 1, LoadFile );					// 2 bytes = Num_vertices
	Num_vertices = temp_ushort;
	Assert( Num_vertices <= MAX_VERTICES );

	cfread( &temp_ushort, sizeof(ushort), 1, LoadFile );					// 2 bytes = Num_segments
	Num_segments = temp_ushort;
	Assert( Num_segments <= MAX_SEGMENTS );

	cfread( Vertices, sizeof(vms_vector), Num_vertices, LoadFile );

	for (segnum=0; segnum<Num_segments; segnum++ )	{
		int	bit;

		#ifdef EDITOR
		Segments[segnum].segnum = segnum;
		Segments[segnum].group = 0;
		#endif

 		cfread( &bit_mask, sizeof(ubyte), 1, LoadFile );

		for (bit=0; bit<MAX_SIDES_PER_SEGMENT; bit++) {
			if (bit_mask & (1 << bit))
		 		cfread( &Segments[segnum].children[bit], sizeof(short), 1, LoadFile );
			else
				Segments[segnum].children[bit] = -1;
		}

		// Read short Segments[segnum].verts[MAX_VERTICES_PER_SEGMENT]
		cfread( Segments[segnum].verts, sizeof(short), MAX_VERTICES_PER_SEGMENT, LoadFile );
		Segments[segnum].objects = -1;

		if (bit_mask & (1 << MAX_SIDES_PER_SEGMENT)) {
			// Read ubyte	Segments[segnum].special
			cfread( &Segments[segnum].special, sizeof(ubyte), 1, LoadFile );
			// Read byte	Segments[segnum].matcen_num
			cfread( &Segments[segnum].matcen_num, sizeof(ubyte), 1, LoadFile );
			// Read short	Segments[segnum].value
			cfread( &Segments[segnum].value, sizeof(short), 1, LoadFile );
		} else {
			Segments[segnum].special = 0;
			Segments[segnum].matcen_num = -1;
			Segments[segnum].value = 0;
		}

		// Read fix	Segments[segnum].static_light (shift down 5 bits, write as short)
		cfread( &temp_ushort, sizeof(temp_ushort), 1, LoadFile );
		Segments[segnum].static_light	= ((fix)temp_ushort) << 4;
		//cfread( &Segments[segnum].static_light, sizeof(fix), 1, LoadFile );
	
		// Read the walls as a 6 byte array
		for (sidenum=0; sidenum<MAX_SIDES_PER_SEGMENT; sidenum++ )	{
			Segments[segnum].sides[sidenum].pad = 0;
		}

 		cfread( &bit_mask, sizeof(ubyte), 1, LoadFile );
		for (sidenum=0; sidenum<MAX_SIDES_PER_SEGMENT; sidenum++) {
			ubyte byte_wallnum;

			if (bit_mask & (1 << sidenum)) {
				cfread( &byte_wallnum, sizeof(ubyte), 1, LoadFile );
				if ( byte_wallnum == 255 )			
					Segments[segnum].sides[sidenum].wall_num = -1;
				else		
					Segments[segnum].sides[sidenum].wall_num = byte_wallnum;
			} else
					Segments[segnum].sides[sidenum].wall_num = -1;
		}

		for (sidenum=0; sidenum<MAX_SIDES_PER_SEGMENT; sidenum++ )	{

			if ( (Segments[segnum].children[sidenum]==-1) || (Segments[segnum].sides[sidenum].wall_num!=-1) )	{
				// Read short Segments[segnum].sides[sidenum].tmap_num;
				cfread( &temp_ushort, sizeof(ushort), 1, LoadFile );

				Segments[segnum].sides[sidenum].tmap_num = temp_ushort & 0x7fff;

				if (!(temp_ushort & 0x8000))
					Segments[segnum].sides[sidenum].tmap_num2 = 0;
				else {
					// Read short Segments[segnum].sides[sidenum].tmap_num2;
					cfread( &Segments[segnum].sides[sidenum].tmap_num2, sizeof(short), 1, LoadFile );
				}

				// Read uvl Segments[segnum].sides[sidenum].uvls[4] (u,v>>5, write as short, l>>1 write as short)
				for (i=0; i<4; i++ )	{
					cfread( &temp_short, sizeof(short), 1, LoadFile );
					Segments[segnum].sides[sidenum].uvls[i].u = ((fix)temp_short) << 5;
					cfread( &temp_short, sizeof(short), 1, LoadFile );
					Segments[segnum].sides[sidenum].uvls[i].v = ((fix)temp_short) << 5;
					cfread( &temp_ushort, sizeof(temp_ushort), 1, LoadFile );
					Segments[segnum].sides[sidenum].uvls[i].l = ((fix)temp_ushort) << 1;
					//cfread( &Segments[segnum].sides[sidenum].uvls[i].l, sizeof(fix), 1, LoadFile );
				}	
			} else {
				Segments[segnum].sides[sidenum].tmap_num = 0;
				Segments[segnum].sides[sidenum].tmap_num2 = 0;
				for (i=0; i<4; i++ )	{
					Segments[segnum].sides[sidenum].uvls[i].u = 0;
					Segments[segnum].sides[sidenum].uvls[i].v = 0;
					Segments[segnum].sides[sidenum].uvls[i].l = 0;
				}	
			}
		}
	}

	Highest_vertex_index = Num_vertices-1;
	Highest_segment_index = Num_segments-1;

	validate_segment_all();			// Fill in side type and normals.

	// Activate fuelcentes
	for (i=0; i< Num_segments; i++ ) {
		fuelcen_activate( &Segments[i], Segments[i].special );
	}

	reset_objects(1);		//one object, the player

	return 0;
}

#endif

