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
 * $Source: f:/miner/source/main/editor/rcs/kdefs.h $
 * $Revision: 2.1 $
 * $Author: yuan $
 * $Date: 1995/03/08 16:07:10 $
 *
 * Prototypes for functions called from keypresses or buttons.
 *
 * $Log: kdefs.h $
 * Revision 2.1  1995/03/08  16:07:10  yuan
 * Added segment sizing default functions.
 *
 * Revision 2.0  1995/02/27  11:34:34  john
 * Version 2.0! No anonymous unions, Watcom 10.0, with no need
 * for bitmaps.tbl.
 *
 * Revision 1.97  1995/01/12  12:10:22  yuan
 * Added coop object capability.
 *
 * Revision 1.96  1994/10/27  10:06:33  mike
 * kill macro stuff.
 *
 * Revision 1.95  1994/09/26  23:22:59  matt
 * Added functions to keep player's starting position from getting messed up
 *
 * Revision 1.94  1994/09/24  14:15:24  mike
 * Custom colored object support.
 *
 * Revision 1.93  1994/09/23  18:03:52  yuan
 * Finished wall checking code.
 *
 * Revision 1.92  1994/09/14  16:50:49  yuan
 * Added load mine only function
 *
 * Revision 1.91  1994/08/16  18:11:03  yuan
 * Maded C place you in the center of a segment.
 *
 * Revision 1.90  1994/08/15  17:47:55  yuan
 * Added external walls.
 *
 * Revision 1.89  1994/08/03  10:32:10  mike
 * Texture map propagation without uv assignment.
 *
 * Revision 1.88  1994/08/02  14:18:01  mike
 * Add Object dialog.
 *
 * Revision 1.87  1994/07/22  17:19:15  yuan
 * Working on dialog box for refuel/repair/material/control centers.
 *
 * Revision 1.86  1994/07/21  17:26:49  matt
 * When new mine created, the default save filename is now reset
 *
 * Revision 1.85  1994/07/21  12:47:26  mike
 * *** empty log message ***
 *
 * Revision 1.84  1994/07/14  14:49:19  yuan
 * Added prototype
 *
 * Revision 1.83  1994/07/14  14:43:09  yuan
 * Added new rotation functions
 *
 * Revision 1.82  1994/07/01  17:57:04  john
 * First version of not-working hostage system
 *
 *
 * Revision 1.81  1994/06/21  12:57:27  yuan
 * Remove center from segment function added to menu.
 *
 * Revision 1.80  1994/06/17  16:05:20  mike
 * Prototype set_average_light_on_all_quick.
 *
 * Revision 1.79  1994/05/31  16:43:24  john
 * Added hooks to create materialization centers.
 *
 * Revision 1.78  1994/05/27  10:34:40  yuan
 * Added new Dialog boxes for Walls and Triggers.
 *
 * Revision 1.77  1994/05/25  18:08:44  yuan
 * Revamping walls and triggers interface.
 * Wall interface complete, but triggers are still in progress.
 *
 * Revision 1.76  1994/05/09  23:34:04  mike
 * SubtractFromGroup, CreateSloppyAdjacentJointsGroup, ClearFoundList
 *
 * Revision 1.75  1994/05/03  18:31:17  mike
 * Add PerturbCurside.
 *
 * Revision 1.74  1994/05/03  11:04:39  mike
 * Add prototypes for new segment sizing functions.
 *
 * Revision 1.73  1994/04/29  10:32:04  yuan
 * Added door 8... Door typing system should be replaced soon.
 *
 */

// In khelp.c
int DoHelp(void);

// In kcurve.c
int InitCurve(void);
int GenerateCurve(void);
int DecreaseR4(void);
int IncreaseR4(void);
int DecreaseR1(void);
int IncreaseR1(void);
int DeleteCurve(void);
int SetCurve(void);

// In kmine.c
int SaveMine(void);
int LoadMine();
int MineMenu(void);
int CreateNewMine(void);
int LoadOldMine();

int SaveSituation(void);
int LoadSituation(void);

// In kgame.c
int SetPlayerPosition(void);
int SaveGameData(void);
int LoadGameData(void);
int LoadMineOnly();
void ResetFilename(void);

// In group.c
int LoadGroup(void);
int SaveGroup(void);
int PrevGroup(void);
int NextGroup(void);
int CreateGroup(void);
int SubtractFromGroup(void);
int DeleteGroup(void);
int MarkGroupSegment(void);
int MoveGroup(void);
int CopyGroup(void);
int AttachSegmentNew(void);
int UngroupSegment(void);
int GroupSegment(void);
int Degroup(void);
int RotateGroup(void);

// In segment.c
int ToggleBottom(void);
void make_curside_bottom_side();

// In editor.c
int UndoCommand(void);

// In kview.c
int ZoomOut(void);
int ZoomIn(void);
int MoveAway(void);
int MoveCloser(void);
int ToggleChaseMode(void);

// In kbuild.c
int CreateBridge(void);
int FormJoint(void);
int CreateAdjacentJoint(void);
int CreateAdjacentJointsSegment(void);
int CreateAdjacentJointsAll(void);
int CreateSloppyAdjacentJoint(void);
int CreateSloppyAdjacentJointsGroup(void);

// In ksegmove.c
int DecreaseHeading(void);
int IncreaseHeading(void);
int DecreasePitch(void);
int IncreasePitch(void);
int DecreaseBank(void);
int IncreaseBank(void);

// In ksegsel.c
int SelectCurrentSegForward(void);
int SelectCurrentSegBackward(void);
int SelectNextSide(void);
int SelectPrevSide(void);
int CopySegToMarked(void);
int SelectBottom(void);
int SelectFront(void);
int SelectTop(void);
int SelectBack(void);
int SelectLeft(void);
int SelectRight(void);

// In ksegsize.c
int IncreaseSegLength(void);
int DecreaseSegLength(void);
int DecreaseSegWidth(void);
int IncreaseSegWidth(void);
int IncreaseSegHeight(void);
int DecreaseSegHeight(void);
int ToggleSegSizeMode(void);
int PerturbCurside(void);
int PerturbCursideBig(void);

int IncreaseSegLengthBig(void);
int DecreaseSegLengthBig(void);
int DecreaseSegWidthBig(void);
int IncreaseSegWidthBig(void);
int IncreaseSegHeightBig(void);
int DecreaseSegHeightBig(void);

int IncreaseSegLengthDefault(void);
int DecreaseSegLengthDefault(void);
int IncreaseSegWidthDefault(void);
int DecreaseSegWidthDefault(void);
int IncreaseSegHeightDefault(void);
int DecreaseSegHeightDefault(void);

//	In ktmap.c
int AssignTexture(void);
int AssignTexture2(void);
int ClearTexture2(void);
int PropagateTextures(void);
int PropagateTexturesMove(void);
int PropagateTexturesMoveUVs(void);
int PropagateTexturesUVs(void);
int PropagateTexturesSelected(void);

//--//// In macro.c
//--//int MacroMenu();
//--//int MacroPlayFast();
//--//int MacroPlayNormal();
//--//int MacroRecordAll();
//--//int MacroRecordKeys();
//--//int MacroSave();
//--//int MacroLoad();

// In editor.c
int medlisp_update_screen(void);
int medlisp_delete_segment(void);
int medlisp_scale_segment(void);
int medlisp_rotate_segment(void);
int medlisp_add_segment(void);
int AttachSegment(void);
int DeleteSegment();
int DosShell(void);
int CallLisp();
int ExitEditor(void);
int ShowAbout(void);
int ExchangeMarkAndCurseg(void);
int med_keypad_goto_prev(void);
int med_keypad_goto_next(void);
int med_keypad_goto(void);
int med_increase_tilings();
int med_decrease_tilings();
int ToggleAutosave(void);
int MarkStart(void);
int MarkEnd(void);

//	Texture.c
int	TexFlipX(void);
int	TexFlipY(void);
int	TexSlideUp(void);
int	TexSlideLeft(void);
int	TexSetDefault(void);
int	TexSlideRight(void);
int	TexRotateLeft(void);
int	TexSlideDown(void);
int	TexRotateRight(void);
int	TexSelectActiveEdge(void);
int	TexRotate90Degrees(void);
int	TexIncreaseTiling(void);
int	TexDecreaseTiling(void);
int	TexSlideUpBig(void);
int	TexSlideLeftBig(void);
int	TexSlideRightBig(void);
int	TexRotateLeftBig(void);
int	TexSlideDownBig(void);
int	TexRotateRightBig(void);
int	TexStretchDown(void);
int	TexStretchUp(void);

//	object.c
int	ObjectPlaceObject(void);
int	ObjectMakeCoop(void);
int	ObjectPlaceObjectTmap(void);
int	ObjectDelete(void);
int	ObjectMoveForward(void);
int	ObjectMoveLeft(void);
int	ObjectSetDefault(void);
int	ObjectMoveRight(void);
int	ObjectMoveBack(void);
int	ObjectMoveDown(void);
int	ObjectMoveUp(void);
int	ObjectMoveNearer(void);
int	ObjectMoveFurther(void);
int	ObjectSelectNextinSegment(void);
int	ObjectSelectNextType();
int	ObjectDecreaseBank(void);
int	ObjectIncreaseBank(void);
int	ObjectDecreasePitch(void);
int	ObjectIncreasePitch(void);
int	ObjectDecreaseHeading(void);
int	ObjectIncreaseHeading(void);
int  	ObjectResetObject(void);


//	elight.c
int	LightSelectNextVertex(void);
int	LightSelectNextEdge(void);
int	LightCopyIntensity(void);
int	LightCopyIntensitySegment(void);
int	LightDecreaseLightVertex(void);
int	LightIncreaseLightVertex(void);
int	LightDecreaseLightSide(void);
int	LightIncreaseLightSide(void);
int	LightDecreaseLightSegment(void);
int	LightIncreaseLightSegment(void);
int	LightSetMaximum(void);
int	LightSetDefault(void);
int	LightSetDefaultAll(void);
int	LightAmbientLighting(void);

// seguvs.c
int fix_bogus_uvs_on_side(void);
int fix_bogus_uvs_all(void);
int set_average_light_on_curside(void);
int set_average_light_on_all(void);
int set_average_light_on_all_quick(void);

// Miscellaneous, please put in correct file if you have time
int IncreaseDrawDepth();
int DecreaseDrawDepth();
int GotoGame(void);
int GotoGameScreen(void);
int DropIntoDebugger(void);
int CreateDefaultNewSegment(void);
int CreateDefaultNewSegmentandAttach(void);
int ClearSelectedList(void);
int ClearFoundList(void);
int SortSelectedList();
int SetPlayerFromCurseg(void);
int SetPlayerFromCursegAndRotate(void);
int SetPlayerFromCursegMinusOne(void);
int FindConcaveSegs(void);
int SelectNextFoundSeg();
int SelectPreviousFoundSeg(void);
int do_reset_orient();
int GameZoomOut(void);
int GameZoomIn(void);

// John's temp page stuff
int medtmp_set_page();

// In objpage.c
int objpage_goto_next_object(void);

// In medsel.c
extern int SortSelectedList(void);
extern int SelectNextFoundSeg(void);
extern int SelectPreviousFoundSeg(void);

// In wall.c
extern int wall_add_blastable(void);
extern int wall_add_door(void);
extern int wall_add_closed_wall(void);
extern int wall_add_external_wall(void);
extern int wall_lock_door(void);
extern int wall_unlock_door(void);
extern int wall_automate_door(void);
extern int wall_deautomate_door(void);
extern int wall_add_illusion(void);
extern int wall_remove(void);
extern int wall_restore_all(void);
extern int wall_assign_door_1(void);
extern int wall_assign_door_2(void);
extern int wall_assign_door_3(void);
extern int wall_assign_door_4(void);
extern int wall_assign_door_5(void);
extern int wall_assign_door_6(void);
extern int wall_assign_door_7(void);
extern int wall_assign_door_8(void);
extern int do_wall_dialog(void);
extern int do_trigger_dialog(void);
extern int check_walls(void);
extern int delete_all_walls(void);

// In centers.c
extern int do_centers_dialog(void);

// In switch.c
//extern int trigger_add_damage(void);
//extern int trigger_add_blank(void);
//extern int trigger_add_exit(void);
//extern int trigger_add_repair(void);
//extern int trigger_control(void);
//extern int trigger_remove(void);
//extern int trigger_add_if_control_center_dead(void);
extern int bind_wall_to_control_trigger(void);

// In med.c
extern int fuelcen_create_from_curseg(void);
extern int repaircen_create_from_curseg(void);
extern int controlcen_create_from_curseg(void);
extern int robotmaker_create_from_curseg(void);
extern int fuelcen_reset_all(void);
extern int RestoreGameState(void);
extern int fuelcen_delete_from_curseg(void);

// In editor\robot.c
extern int do_robot_dialog(void);
extern int do_object_dialog(void);

// In editor\hostage.c
extern int do_hostage_dialog(void);
