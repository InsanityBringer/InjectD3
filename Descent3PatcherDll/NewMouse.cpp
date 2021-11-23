/*-----------------------------------------------------------------------------
*
 *  Copyright (c) 2021 SaladBadger
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Win32App.h"
#include "NewMouse.h"
#include "psclass.h"
#include "Configuration.h"
#include "GameOffsets.h"

mses_state* pDDIO_mouse_state;
bool* pDDIO_mouse_init;
bool rawInputOpened = false;

bool handlerCalled = false;

FILE* hack = nullptr;

float MouseScalar = 0.5f;

typedef struct t_mse_button_info
{
	bool is_down[N_MSEBTNS];
	uint8_t down_count[N_MSEBTNS];
	uint8_t up_count[N_MSEBTNS];
	DWORD time_down[N_MSEBTNS];	// in milliseconds windows ticks
	DWORD time_up[N_MSEBTNS];
}
t_mse_button_info;

typedef struct t_mse_event
{
	short btn;
	short state;
}
t_mse_event;

t_mse_button_info* pDIM_buttons;
tQueue<t_mse_event, 16>* pMB_queue;

//Normally mouse events use ticks, attempting to use timer_GetTime which has more accuracy to smooth over bug with mouse buttons dropping. 
float localDownStart[N_MSEBTNS];
float localUpStart[N_MSEBTNS];

#define MOUSE_ZMIN		0						// mouse wheel z min and max (increments of 120 = 10 units)
#define MOUSE_ZMAX		1200
#define N_DIMSEBTNS		4						// # of REAL mouse buttons
#define MSEBTN_WHL_UP	(N_DIMSEBTNS)		// button index for mouse wheel up
#define MSEBTN_WHL_DOWN	(N_DIMSEBTNS+1)	// button index for mouse wheel down

int wheelAccum = 0;

//These are needed to keep the mouse smooth in UIs
float xCoord, yCoord;

// taken from winuser.h
#ifndef WHEEL_DELTA
#define WHEEL_DELTA	120
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL	0x20a
#endif

//-----------------------------------------------------------------------------
// NOTE: The following code is from the Descent 3 editor source and is
// therefore under Outrage's code license
//-----------------------------------------------------------------------------
void DDIOShowCursor(BOOL show)
{
	if (show) 
	{
		if (pDDIO_mouse_state->cursor_count == -1) 
		{
			ShowCursor(TRUE);
		}
		pDDIO_mouse_state->cursor_count = 0;
	}
	else 
	{
		if (pDDIO_mouse_state->cursor_count == 0) 
		{
			ShowCursor(FALSE);
		}
		pDDIO_mouse_state->cursor_count = -1;
	}
}

void ddio_MouseMode(int mode)
{
	if (mode == MOUSE_EXCLUSIVE_MODE) 
	{
		DDIOShowCursor(FALSE);
	}
	else if (mode == MOUSE_STANDARD_MODE) 
	{
		DDIOShowCursor(TRUE);
	}
	else 
	{
		//Int3();
		return;
	}

	pDDIO_mouse_state->mode = mode;
}

void ddio_MouseQueueFlush()
{
	memset(pDIM_buttons, 0, sizeof(*pDIM_buttons));
	pMB_queue->flush();

	//Need to clear the new arrays, since otherwise the game will think you're holding down a button when leaving a UI screen.
	memset(localDownStart, 0, sizeof(localDownStart));
	memset(localUpStart, 0, sizeof(localUpStart));
}

void ddio_MouseReset()
{
	/*tWin32AppInfo appi;

	DInputData.app->get_info(&appi);
	pDDIO_mouse_state->cx = appi.wnd_x + (appi.wnd_w / 2);
	pDDIO_mouse_state->cy = appi.wnd_y + (appi.wnd_h / 2);*/

	SetRect(&pDDIO_mouse_state->brect, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	pDDIO_mouse_state->zmin = MOUSE_ZMIN;
	pDDIO_mouse_state->zmax = MOUSE_ZMAX;

	//	set up new coordinates for mouse pointer.
	pDDIO_mouse_state->btn_mask = 0;
	pDDIO_mouse_state->dx = 0;
	pDDIO_mouse_state->dy = 0;
	pDDIO_mouse_state->dz = 0;
	pDDIO_mouse_state->imm_dz = 0;
	pDDIO_mouse_state->x = (pDDIO_mouse_state->brect.right - pDDIO_mouse_state->brect.left) / 2;
	pDDIO_mouse_state->y = (pDDIO_mouse_state->brect.bottom - pDDIO_mouse_state->brect.top) / 2;
	pDDIO_mouse_state->z = (pDDIO_mouse_state->zmax = pDDIO_mouse_state->zmin) / 2;
	pDDIO_mouse_state->cz = 0;
	pDDIO_mouse_state->x_aspect = 1.0f;
	pDDIO_mouse_state->y_aspect = 1.0f;

	xCoord = pDDIO_mouse_state->x;
	yCoord = pDDIO_mouse_state->y;

	// reset button states
	ddio_MouseQueueFlush();
}

// return mouse button down time.
// This function has been hacked to use timer_GetTime which can be much more accurate. 
float ddio_MouseBtnDownTime(int btn)
{
	//DWORD ticks, curticks = GetTickCount();
	float time, curtime = timer_GetTime();

	//ASSERT(btn >= 0 && btn < N_MSEBTNS);

	if (pDIM_buttons->is_down[btn])
	{
		time = curtime - localDownStart[btn];
		pDIM_buttons->time_down[btn] = (int)(curtime*1000);
		localDownStart[btn] = curtime;
	}
	else 
	{
		time = localUpStart[btn] - localDownStart[btn];
		localUpStart[btn] = localDownStart[btn] = 0;
		pDIM_buttons->time_down[btn] = pDIM_buttons->time_up[btn] = 0;
	}

	pDIM_buttons->is_down[MSEBTN_WHL_UP] = false;
	pDIM_buttons->is_down[MSEBTN_WHL_DOWN] = false;

	return time;
}

//Mouse prescaling needs to be handled here, otherwise deltas frequently become 0 since they're very small individually (often 1), but come in large batches. 
int ddio_MouseGetState(int* x, int* y, int* dx, int* dy, int* z, int* dz)
{
	//	update mouse timer.
	int btn_mask = pDDIO_mouse_state->btn_mask;

	pDDIO_mouse_state->timer = timer_GetTime();

	//	get return values.
	if (x) *x = pDDIO_mouse_state->x;
	if (y) *y = pDDIO_mouse_state->y;
	if (z) *z = pDDIO_mouse_state->z;
	//TODO: Is there a cleaner way of doing this?
	//This hack fixes low prescalars causing a "dead zone" where small motions weren't picked up. 
	if (dx) *dx = (int)ceil(abs(pDDIO_mouse_state->dx) * (double)MouseScalar) * (pDDIO_mouse_state->dx < 0 ? -1 : 1);
	if (dy) *dy = (int)ceil(abs(pDDIO_mouse_state->dy) * (double)MouseScalar) * (pDDIO_mouse_state->dy < 0 ? -1 : 1);
	if (dz) *dz = pDDIO_mouse_state->dz;

	pDDIO_mouse_state->dx = 0;
	pDDIO_mouse_state->dy = 0;
	pDDIO_mouse_state->dz = 0;
	pDDIO_mouse_state->btn_mask = 0;

	pDIM_buttons->is_down[MSEBTN_WHL_UP] = false;
	pDIM_buttons->is_down[MSEBTN_WHL_DOWN] = false;

	return btn_mask;
}

void ddio_InternalMouseFrame()
{
	int btn_mask = 0;

	//These need to be continually maintained, since a small number of inputs rely on it being set every frame.
	if (pDIM_buttons->is_down[0]) btn_mask |= MOUSE_LB;
	if (pDIM_buttons->is_down[1]) btn_mask |= MOUSE_RB;
	if (pDIM_buttons->is_down[2]) btn_mask |= MOUSE_CB;
	if (pDIM_buttons->is_down[3]) btn_mask |= MOUSE_B4;
	if (pDIM_buttons->is_down[4]) btn_mask |= MOUSE_B5;
	if (pDIM_buttons->is_down[5]) btn_mask |= MOUSE_B6;
	if (pDIM_buttons->is_down[6]) btn_mask |= MOUSE_B7;
	if (pDIM_buttons->is_down[7]) btn_mask |= MOUSE_B8;

	pDDIO_mouse_state->btn_mask = btn_mask;
}

//-----------------------------------------------------------------------------
// End Outrage-licensed code.
//-----------------------------------------------------------------------------

void MouseError()
{
	MessageBoxA(nullptr, "Failed to init raw input for mouse", "Error", MB_ICONERROR);
}

int RawInputHandler(HWND hWnd, unsigned int msg, unsigned int wParam, long lParam)
{
	uint32_t buttons;
	t_mse_event ev;
	float curtime = timer_GetTime();

	if (pDDIO_mouse_state->suspended) 
	{
		pDDIO_mouse_state->btn_mask = 0;
		pDDIO_mouse_state->dx = 0;
		pDDIO_mouse_state->dy = 0;
		pDDIO_mouse_state->dz = 0;
		return 0;
	}

	HRAWINPUT rawinputHandle = (HRAWINPUT)lParam;
	UINT size = 0;
	UINT result = GetRawInputData(rawinputHandle, RID_INPUT, 0, &size, sizeof(RAWINPUTHEADER));
	if (result == 0 && size > 0)
	{
		void* buf = malloc(size);
		if (!buf)
		{
			return 0;
		}
		result = GetRawInputData(rawinputHandle, RID_INPUT, buf, &size, sizeof(RAWINPUTHEADER));

		if (result >= 0)
		{
			RAWINPUT* rawinput = (RAWINPUT*)buf;
			if (rawinput->header.dwType == RIM_TYPEMOUSE)
			{
				buttons = rawinput->data.mouse.ulButtons;
				if (buttons & RI_MOUSE_LEFT_BUTTON_DOWN && !pDIM_buttons->is_down[0])
				{
					pDIM_buttons->down_count[0]++;
					pDIM_buttons->time_down[0] = GetTickCount();
					localDownStart[0] = curtime;
					pDIM_buttons->is_down[0] = true;
					pDDIO_mouse_state->btn_mask |= MOUSE_LB;
					ev.btn = 0;
					ev.state = true;
					pMB_queue->send(ev);
					/*if (hack)
					{
						fprintf(hack, "MB1 down at %f\n", timer_GetTime());
					}*/
				}
				else if (buttons & RI_MOUSE_LEFT_BUTTON_UP && pDIM_buttons->is_down[0])
				{
					pDIM_buttons->up_count[0]++;
					pDIM_buttons->is_down[0] = false;
					pDIM_buttons->time_up[0] = GetTickCount();
					localUpStart[0] = curtime;
					pDDIO_mouse_state->btn_mask &= ~MOUSE_LB;
					ev.btn = 0;
					ev.state = false;
					pMB_queue->send(ev);
					/*if (hack)
					{
						fprintf(hack, "MB1 up at %f\n", timer_GetTime());
					}*/
				}
				if (buttons & RI_MOUSE_RIGHT_BUTTON_DOWN && !pDIM_buttons->is_down[1])
				{
					pDIM_buttons->down_count[1]++;
					pDIM_buttons->time_down[1] = GetTickCount();
					localDownStart[1] = curtime;
					pDIM_buttons->is_down[1] = true;
					pDDIO_mouse_state->btn_mask |= MOUSE_RB;
					ev.btn = 1;
					ev.state = true;
					pMB_queue->send(ev);
				}
				else if (buttons & RI_MOUSE_RIGHT_BUTTON_UP && pDIM_buttons->is_down[1])
				{
					pDIM_buttons->up_count[1]++;
					pDIM_buttons->is_down[1] = false;
					pDIM_buttons->time_up[1] = GetTickCount();
					localUpStart[1] = curtime;
					pDDIO_mouse_state->btn_mask &= ~MOUSE_RB;
					ev.btn = 1;
					ev.state = false;
					pMB_queue->send(ev);
				}
				if (buttons & RI_MOUSE_MIDDLE_BUTTON_DOWN && !pDIM_buttons->is_down[2])
				{
					pDIM_buttons->down_count[2]++;
					pDIM_buttons->time_down[2] = GetTickCount();
					localDownStart[2] = curtime;
					pDIM_buttons->is_down[2] = true;
					pDDIO_mouse_state->btn_mask |= MOUSE_CB;
					ev.btn = 2;
					ev.state = true;
					pMB_queue->send(ev);
				}
				else if (buttons & RI_MOUSE_MIDDLE_BUTTON_UP && pDIM_buttons->is_down[2])
				{
					pDIM_buttons->up_count[2]++;
					pDIM_buttons->is_down[2] = false;
					pDIM_buttons->time_up[2] = GetTickCount();
					localUpStart[2] = curtime;
					pDDIO_mouse_state->btn_mask &= ~MOUSE_CB;
					ev.btn = 2;
					ev.state = false;
					pMB_queue->send(ev);
				}

				if (buttons & RI_MOUSE_WHEEL)
				{
					wheelAccum += (int)(short)rawinput->data.mouse.usButtonData;
					if (wheelAccum >= WHEEL_DELTA)
					{
						pDIM_buttons->down_count[MSEBTN_WHL_UP]++;
						pDIM_buttons->up_count[MSEBTN_WHL_UP]++;
						pDIM_buttons->is_down[MSEBTN_WHL_UP] = true;
						pDIM_buttons->time_down[MSEBTN_WHL_UP] = GetTickCount();
						pDIM_buttons->time_up[MSEBTN_WHL_UP] = GetTickCount()+100;
						localDownStart[MSEBTN_WHL_UP] = curtime;
						localUpStart[MSEBTN_WHL_UP] = curtime+.1f;
						wheelAccum = 0;
					}
					else if (wheelAccum <= -WHEEL_DELTA)
					{
						pDIM_buttons->down_count[MSEBTN_WHL_DOWN]++;
						pDIM_buttons->up_count[MSEBTN_WHL_DOWN]++;
						pDIM_buttons->is_down[MSEBTN_WHL_DOWN] = true;
						pDIM_buttons->time_down[MSEBTN_WHL_DOWN] = GetTickCount();
						pDIM_buttons->time_up[MSEBTN_WHL_DOWN] = GetTickCount()+100;
						localDownStart[MSEBTN_WHL_DOWN] = curtime;
						localUpStart[MSEBTN_WHL_DOWN] = curtime+.1f;
						wheelAccum = 0;
					}
				}

				pDDIO_mouse_state->dx += rawinput->data.mouse.lLastX;
				pDDIO_mouse_state->dy += rawinput->data.mouse.lLastY;
				//pDDIO_mouse_state->btn_mask = buttons;
			}

			//if (pDDIO_mouse_state->mode == MOUSE_STANDARD_MODE) 
			{
				// if in standard mode, don't use x,y,z retreived from dimouse_GetDeviceData
				xCoord += (float)rawinput->data.mouse.lLastX;
				yCoord += (float)rawinput->data.mouse.lLastY;

				pDDIO_mouse_state->x = (int)xCoord;
				pDDIO_mouse_state->y = (int)yCoord;
				pDDIO_mouse_state->z = 0;

				//	check bounds of mouse cursor.
				if (pDDIO_mouse_state->x < pDDIO_mouse_state->brect.left)
					xCoord = pDDIO_mouse_state->x = (short)pDDIO_mouse_state->brect.left;
				if (pDDIO_mouse_state->x >= pDDIO_mouse_state->brect.right)
					xCoord = pDDIO_mouse_state->x = (short)pDDIO_mouse_state->brect.right - 1;
				if (pDDIO_mouse_state->y < pDDIO_mouse_state->brect.top)
					yCoord = pDDIO_mouse_state->y = (short)pDDIO_mouse_state->brect.top;
				if (pDDIO_mouse_state->y >= pDDIO_mouse_state->brect.bottom)
					yCoord = pDDIO_mouse_state->y = (short)pDDIO_mouse_state->brect.bottom - 1;
				if (pDDIO_mouse_state->z > pDDIO_mouse_state->zmax)
					pDDIO_mouse_state->z = (short)pDDIO_mouse_state->zmax;
				if (pDDIO_mouse_state->z < pDDIO_mouse_state->zmin)
					pDDIO_mouse_state->z = (short)pDDIO_mouse_state->zmin;
			}
		}

		free(buf);
	}
	return 0;
}


bool InitNewMouse()
{
	int i;
	if (!rawInputOpened)
	{
		char buf[256];
		//Get the pointer to the DDIO_mouse_state
		//1.4 pointers
		app = *((oeWin32Application**)GetPatchPoint(PatchPoint::AppVar));
		pDDIO_mouse_state = (mses_state *)GetPatchPoint(PatchPoint::DDIOMouseStateVar);
		pDDIO_mouse_init = (bool*)GetPatchPoint(PatchPoint::DDIOMouseInitVar);
		pDIM_buttons = (t_mse_button_info*)GetPatchPoint(PatchPoint::DIMButtonsVar);
		pMB_queue = (tQueue<t_mse_event, 16> *)GetPatchPoint(PatchPoint::MBQueueVar);
		UINT nDevices;
		PRAWINPUTDEVICELIST pRawInputDeviceList, selectedDevice = nullptr;
		RID_DEVICE_INFO deviceInfo = {};
		UINT deviceInfoSize = deviceInfo.cbSize = sizeof(RID_DEVICE_INFO);
		RAWINPUTDEVICE rawInputDevice = {};
		/*if (GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)) != 0)
		{
			MessageBoxA(nullptr, "Can't get number of HIDs", "Error", MB_ICONERROR);
			return false;
		}
		if ((pRawInputDeviceList = (PRAWINPUTDEVICELIST)malloc(sizeof(RAWINPUTDEVICELIST) * nDevices)) == NULL)
		{
			MessageBoxA(nullptr, "Can't allocate buffer of HIDs", "Error", MB_ICONERROR);
			return false;
		}
		if (GetRawInputDeviceList(pRawInputDeviceList, &nDevices, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1)
		{
			free(pRawInputDeviceList);
			MessageBoxA(nullptr, "Can't get HID buffer", "Error", MB_ICONERROR);
			return false;
		}

		//From the list, find a mouse device
		for (UINT i = 0; i < nDevices; i++)
		{
			if (pRawInputDeviceList[i].dwType != RIM_TYPEMOUSE) //Not a mouse?
				continue; 

			//Well, we're not too choosy, let's just use the first one we found.
			//TODO: be choosier and use other ones
			if (GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICEINFO, (LPVOID)&deviceInfo, &deviceInfoSize) == (UINT)-1)
			{
				free(pRawInputDeviceList);
				MessageBoxA(nullptr, "Failed to get HID information", "Error", MB_ICONERROR);
				return false;
			}

			selectedDevice = &pRawInputDeviceList[i];
			pDDIO_mouse_state->nbtns = deviceInfo.mouse.dwNumberOfButtons;
			break;
		}

		if (!selectedDevice)
		{
			free(pRawInputDeviceList);
			MessageBoxA(nullptr, "Can't find a candidate mouse device", "Error", MB_ICONERROR);
			return false;
		}*/

		rawInputDevice.usUsage = 0x0002;
		rawInputDevice.usUsagePage = 0x0001;
		rawInputDevice.dwFlags = RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE;
		rawInputDevice.hwndTarget = app->m_hWnd;

		if (RegisterRawInputDevices(&rawInputDevice, 1, sizeof(rawInputDevice)) == FALSE)
		{
			//free(pRawInputDeviceList);
			snprintf(buf, 255, "HID Registration failed: %d", GetLastError());
			MessageBoxA(nullptr, buf, "Error", MB_ICONERROR);
			PutLog(LogLevel::Error, "HID Registration failed: %d.", GetLastError());
			return false;
		}

		//HACK: Need to flush messages for this to work.
		MSG msg;

		while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		app_add_handler(app, WM_INPUT, (tOEWin32MsgCallback)&RawInputHandler);

		//*pDDIO_mouse_init = true;
		pDDIO_mouse_state->suspended = false;
		pDDIO_mouse_state->timer = timer_GetTime();
		pDDIO_mouse_state->naxis = 2;
		pDDIO_mouse_state->nbtns = N_DIMSEBTNS + 2; //always have a mousewheel
		for (i = 0; i < pDDIO_mouse_state->nbtns; i++)
		{
			pDDIO_mouse_state->btn_flags |= (1 << i);
		}
		ddio_MouseMode(MOUSE_STANDARD_MODE);
		ddio_MouseReset();

		memset(pDIM_buttons, 0, sizeof(t_mse_button_info));
		PutLog(LogLevel::Info, "Raw input mouse library started successfully.");

		//diagonstics
		/*if (!hack)
		{
			hack = fopen("NewMouse.txt", "w");
		}

		if (hack)
		{
			fprintf(hack, "Mouse driver opened.\n");
		}*/

		//free(pRawInputDeviceList);
	}
	return true;
}
