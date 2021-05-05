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

#define MOUSE_ZMIN		0						// mouse wheel z min and max (increments of 120 = 10 units)
#define MOUSE_ZMAX		1200
#define N_DIMSEBTNS		4						// # of REAL mouse buttons
#define MSEBTN_WHL_UP	(N_DIMSEBTNS)		// button index for mouse wheel up
#define MSEBTN_WHL_DOWN	(N_DIMSEBTNS+1)	// button index for mouse wheel down

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

	// reset button states
	//ddio_MouseQueueFlush();
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
	/*if (!handlerCalled)
	{
		MessageBoxA(hWnd, "Alright, bring it on!", "Ara ara~", MB_OK);
		handlerCalled = true;
	}*/

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
					pDIM_buttons->is_down[0] = true;
					pDDIO_mouse_state->btn_flags |= MOUSE_LB;
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
					pDDIO_mouse_state->btn_flags &= ~MOUSE_LB;
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
					pDIM_buttons->is_down[1] = true;
					pDDIO_mouse_state->btn_flags |= MOUSE_RB;
					ev.btn = 0;
					ev.state = true;
					pMB_queue->send(ev);
				}
				else if (buttons & RI_MOUSE_RIGHT_BUTTON_UP && pDIM_buttons->is_down[1])
				{
					pDIM_buttons->up_count[1]++;
					pDIM_buttons->is_down[1] = false;
					pDIM_buttons->time_up[1] = GetTickCount();
					pDDIO_mouse_state->btn_flags &= ~MOUSE_RB;
					ev.btn = 0;
					ev.state = false;
					pMB_queue->send(ev);
				}
				if (buttons & RI_MOUSE_MIDDLE_BUTTON_DOWN && !pDIM_buttons->is_down[2])
				{
					pDIM_buttons->down_count[2]++;
					pDIM_buttons->time_down[2] = GetTickCount();
					pDIM_buttons->is_down[2] = true;
					pDDIO_mouse_state->btn_flags |= MOUSE_CB;
					ev.btn = 0;
					ev.state = true;
					pMB_queue->send(ev);
				}
				else if (buttons & RI_MOUSE_MIDDLE_BUTTON_UP && pDIM_buttons->is_down[2])
				{
					pDIM_buttons->up_count[2]++;
					pDIM_buttons->is_down[2] = false;
					pDIM_buttons->time_up[2] = GetTickCount();
					pDDIO_mouse_state->btn_flags &= ~MOUSE_CB;
					ev.btn = 0;
					ev.state = false;
					pMB_queue->send(ev);
				}

				pDDIO_mouse_state->dx += rawinput->data.mouse.lLastX;
				pDDIO_mouse_state->dy += rawinput->data.mouse.lLastY;
				//pDDIO_mouse_state->btn_mask = buttons;
			}

			//if (pDDIO_mouse_state->mode == MOUSE_STANDARD_MODE) 
			{
				// if in standard mode, don't use x,y,z retreived from dimouse_GetDeviceData

				pDDIO_mouse_state->x += (pDDIO_mouse_state->dx >> 2); //The default values are way too extreme in my experience
				pDDIO_mouse_state->y += (pDDIO_mouse_state->dy >> 2);
				pDDIO_mouse_state->z = 0;

				//	check bounds of mouse cursor.
				if (pDDIO_mouse_state->x < pDDIO_mouse_state->brect.left)
					pDDIO_mouse_state->x = (short)pDDIO_mouse_state->brect.left;
				if (pDDIO_mouse_state->x >= pDDIO_mouse_state->brect.right)
					pDDIO_mouse_state->x = (short)pDDIO_mouse_state->brect.right - 1;
				if (pDDIO_mouse_state->y < pDDIO_mouse_state->brect.top)
					pDDIO_mouse_state->y = (short)pDDIO_mouse_state->brect.top;
				if (pDDIO_mouse_state->y >= pDDIO_mouse_state->brect.bottom)
					pDDIO_mouse_state->y = (short)pDDIO_mouse_state->brect.bottom - 1;
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

		//If the device registered properly, register a handler for WM_INPUT messages
		//MessageBoxA(app->m_hWnd, "Is this hWnd any good?", "Ara ara~", MB_OK);

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
		pDDIO_mouse_state->nbtns = 3;
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
