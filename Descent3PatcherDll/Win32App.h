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

#define MAX_MSG_FUNCTIONS			64

typedef int (*tOEWin32MsgCallback)(HWND, unsigned, unsigned, long);

typedef struct oeWin32Application
{
	void* vtbl;
	uint32_t unkn;
	bool m_WasCreated;						// Tells us if this app created the window handle or not.

	int m_NumMsgFn;							// Number of message functions.

	struct
	{										// assign functions to messages.
		unsigned msg;
		tOEWin32MsgCallback fn;
	}
	m_MsgFn[MAX_MSG_FUNCTIONS];

	bool m_NTFlag;								// Are we in NT?

	void (*m_DeferFunc)(bool);				// function to call when deffering to OS (OnIdle for instance)

	char m_WndName[64];						// name of window.

	//static bool os_initialized;			// is the OS check initialized?
	//static bool first_time;					// first time init?

	HWND m_hWnd;							// handles created by the system
	HINSTANCE m_hInstance;
	unsigned m_Flags;
	int m_X, m_Y, m_W, m_H;				// window dimensions.
};

extern oeWin32Application* app;

bool app_add_handler(oeWin32Application* self, unsigned msg, tOEWin32MsgCallback fn);

//Timer here for now
extern float (*timer_GetTime)();
