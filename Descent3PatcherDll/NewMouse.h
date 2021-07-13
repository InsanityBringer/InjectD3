/*
 THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF OUTRAGE
 ENTERTAINMENT, INC. ("OUTRAGE").  OUTRAGE, IN DISTRIBUTING THE CODE TO
 END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
 ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
 IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
 SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
 FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
 CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
 AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
 COPYRIGHT 1996-2000 OUTRAGE ENTERTAINMENT, INC.  ALL RIGHTS RESERVED.
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

// use these extensions to set exclusive or standard mouse modes
#define MOUSE_STANDARD_MODE		1			// uses absolute coordinates and simple buttons
#define MOUSE_EXCLUSIVE_MODE		2			// uses relative coordinates and advanced button information

#define MOUSE_LB		1							// mouse button masks
#define MOUSE_RB		2
#define MOUSE_CB		4
#define MOUSE_B4		8
#define MOUSE_B5		16
#define MOUSE_B6		32
#define MOUSE_B7		64
#define MOUSE_B8		128

#define N_MSEBTNS		8

struct mses_state
{
	void* lpdimse; //Useless
	RECT brect;							// limit rectangle of absolute mouse coords
	short x, y, z;						// current x,y,z in absolute mouse coords
	short cx, cy, cz;					// prior values of x,y,z from last mouse frame
	short zmin, zmax;					// 3 dimensional mouse devices use this
	int btn_mask, btn_flags;		// btn_flags are the avaiable buttons on this device in mask form.
	float timer;						// done to keep track of mouse polling.
	bool emulated;						// are we emulating direct input?
	bool acquired;
	bool suspended;
	int8_t cursor_count;
	float x_aspect, y_aspect;		// used in calculating coordinates returned from ddio_MouseGetState
	HANDLE hmseevt;					// signaled if mouse input is awaiting.
	short dx, dy, dz, imm_dz;
	short mode;							// mode of mouse operation.
	short nbtns, naxis;				// device caps.
};

bool InitNewMouse();
float ddio_MouseBtnDownTime(int btn);
void ddio_MouseQueueFlush();
void ddio_MouseReset();
