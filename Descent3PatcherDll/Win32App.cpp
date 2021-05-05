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

#include "Win32App.h"

oeWin32Application* app;

//Functions to replicate oeWin32Application C++ methods. 

bool app_add_handler(oeWin32Application *self, unsigned msg, tOEWin32MsgCallback fn)
{
	int i = 0;

	//	search for redundant callbacks.
	for (i = 0; i < MAX_MSG_FUNCTIONS; i++)
	{
		if (self->m_MsgFn[i].msg == msg && self->m_MsgFn[i].fn == fn)
			return true;
	}

	for (i = 0; i < MAX_MSG_FUNCTIONS; i++)
	{
		if (self->m_MsgFn[i].fn == NULL) 
		{
			self->m_MsgFn[i].msg = msg;
			self->m_MsgFn[i].fn = fn;
			return true;
		}
	}

	DebugBreak();							// We have reached the max number of message functions!

	return false;
}

// real defer code.
#define DEFER_PROCESS_ACTIVE			1		// process is still active
#define DEFER_PROCESS_INPUT_IDLE		2		// process input from os not pending.


int defer_block(oeWin32Application *self)
{
	MSG msg;
	//	static int n_iterations = 0;
	//
	//	if ((n_iterations % 200)==0) {
	//		mprintf((0, "System timer at %f\n", timer_GetTime()));
	//	}
	//	n_iterations++;

	if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
	{
		/*if (msg.message == WM_QUIT) 
		{
			//	QUIT APP.
			exit(1);
		}*/
		TranslateMessage(&msg);
		DispatchMessageA(&msg);

		return DEFER_PROCESS_ACTIVE;
	}
	/*else 
	{
		// IDLE PROCESSING
		if (self->m_DeferFunc)
			(*self->m_DeferFunc)(self->active());

		if (self->active()) 
		{
			return (DEFER_PROCESS_ACTIVE + DEFER_PROCESS_INPUT_IDLE);
		}
		else
		{
			//JEFF - Commented out because we don't want to wait until
			//a message is available, else we won't be able to do any 
			//multiplayer game background processing.
			//
			//WaitMessage();
			//mprintf((0, "Waiting...\n"));
			return (DEFER_PROCESS_INPUT_IDLE);
		}
	}*/

	DebugBreak();
	return (0);
}

float (*timer_GetTime)();

