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

// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <math.h>
#include <memoryapi.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "Win32App.h"
#include "NewMouse.h"
#include "Memory.h"
#include "Rend.h"
#include "Configuration.h"
#include "Sound.h"
#include "GameOffsets.h"

FILE* outputFile;

bool PatchRegistryRoot = false;

void PatchMemory(uintptr_t address, uint8_t* data, size_t len);
void CreateCallTo(uintptr_t srcAddress, uintptr_t funcAddress);
void CreateJmpTo(uintptr_t srcAddress, uintptr_t funcAddress);
void NOPMemory(uintptr_t address, size_t count);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    uint8_t ret = 0xC3;

    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        uint8_t windowedMode = 1;

        //Open a log
        OpenLog("InjectD3Output.txt");
        LoadConfig();
        GameVersion version = DetectVersion();
        if (InitPatchPoints(version))
        {
            //Get some function pointers.
            timer_GetTime = (float(*)())GetPatchPoint(PatchPoint::TimerGetTimeFunc);
            mem_malloc_sub = (void* (*)(int, char*, int))GetPatchPoint(PatchPoint::MemMallocSubFunc);

            //needs to be calculated after reading config
            double doubleFov = DefaultFov;
            float zoom = tan(DefaultFov * .5 * 0.01744444);

            //Prototype patch #1: Change the window title without rewriting the original buffer.
            //Patch WinMain to load our random title string
            //void* bignameptr = (void*)&NewLargeName;
            //PatchMemory(0x4f7c6c, (uint8_t*)&bignameptr, sizeof(void*));

            //Prototype patch #2: Patch init_reg call with a simple function
            //uint32_t offset = (uint32_t)&Die - (0x4f7b49 + 4); //patch init_reg
            //PatchMemory(0x4f7b49, (uint8_t*)&offset, sizeof(offset));

            if (PatchRegistryRoot)
            {
                HKEY hack = HKEY_CURRENT_USER;
                PutLogInit(LogLevel::Info, "Patching registry root to HKEY_CURRENT_USER.");
                PatchMemory(GetPatchPoint(PatchPoint::RegistryRoot), (uint8_t*)&hack, sizeof(HKEY));
            }

            if (PatchMouseLibrary)
            {
                //Mouse library patch:
                PutLogInit(LogLevel::Info, "Patching mouse library.");
                //Patch DDIO startup function with new raw input mouse startup. 
                CreateCallTo(GetPatchPoint(PatchPoint::InitMouseCall), (uintptr_t)&InitNewMouse);
                //Thunk ddio_MouseBtnDownTime
                CreateJmpTo(GetPatchPoint(PatchPoint::MouseBtnDownTimeThunk), (uintptr_t)&ddio_MouseBtnDownTime);
                //Thunk ddio_MouseReset
                CreateJmpTo(GetPatchPoint(PatchPoint::MouseResetThunk), (uintptr_t)&ddio_MouseQueueFlush);
            }

            //Windowed mode patch:
            if (PatchScreenMode == SCREENMODE_WINDOWED)
            {
                PutLogInit(LogLevel::Info, "Patching windowed mode.");
                //Patch WinMain to change oeWin32Application constructor flag param to OEAPP_WINDOWED
                PatchMemory(GetPatchPoint(PatchPoint::OeAppWindowedParam), &windowedMode, sizeof(windowedMode));
                //Patch oeWin32Application::init() to accept OEAPP_WINDOWED.
                //TODO: This simply hijacks OEAPP_CONSOLE, used for the dedi server. This needs to be tweaked, what I could do is reverse the check so it runs the other code on flags equaling OEAPP_FULLSCREEN
                PatchMemory(GetPatchPoint(PatchPoint::OeAppInitWindowParams), &windowedMode, sizeof(windowedMode));
            }

            if (AutoPatchOpenGL)
            {
                //OpenGL renderer patch:
                PutLogInit(LogLevel::Info, "Patching OpenGL renderer.");
                //Patch rend_Init
                CreateCallTo(GetPatchPoint(PatchPoint::RendInitrGLInitCall), (uintptr_t)&rGL_Init);
                //Patch rGL_SetMode
                CreateCallTo(GetPatchPoint(PatchPoint::RendSetModerGLInitCall), (uintptr_t)&rGL_Init);
                //Patch rGL_Flip //5323c6
                CreateJmpTo(GetPatchPoint(PatchPoint::RendFliprGLCall), (uintptr_t)&rGL_Flip);
            }

            //High FOV patch: This needs to patch both the FOV angle and the zoom value actually used for projection. 
            PutLogInit(LogLevel::Info, "Patching FOV constants.");
            //Patch Render_Fov variable
            PatchMemory(GetPatchPoint(PatchPoint::FovVariable), (uint8_t*)&DefaultFov, sizeof(DefaultFov));
            //Patch DoEffects
            PatchMemory(GetPatchPoint(PatchPoint::FovDoEffects1), (uint8_t*)&DefaultFov, sizeof(DefaultFov));
            PatchMemory(GetPatchPoint(PatchPoint::FovDoEffects2), (uint8_t*)&doubleFov, sizeof(doubleFov)); //constant used in FADD
            //Patch UndoPlayerZoom
            PatchMemory(GetPatchPoint(PatchPoint::FovUndoPlayerZoom), (uint8_t*)&DefaultFov, sizeof(DefaultFov));
            //Patch DoPlayerZoom
            PatchMemory(GetPatchPoint(PatchPoint::FovDoPlayerZoom), (uint8_t*)&DefaultFov, sizeof(DefaultFov));
            //zoom patches:
            //Patch Zoom variable
            PatchMemory(GetPatchPoint(PatchPoint::ZoomVariable), (uint8_t*)&zoom, sizeof(zoom));
            //Patch ClearGameEvents
            PatchMemory(GetPatchPoint(PatchPoint::ZoomClearGameEffects), (uint8_t*)&zoom, sizeof(zoom));
            //Patch DoRespawnZoomEffect
            PatchMemory(GetPatchPoint(PatchPoint::ZoomDoRespawnZoomEffect), (uint8_t*)&zoom, sizeof(zoom));
            //Patch init_level
            PatchMemory(GetPatchPoint(PatchPoint::ZoomInitLevel), (uint8_t*)&zoom, sizeof(zoom));

            if (PatchSoundSystem)
            {
                //Sound system patch. This is the biggest hack on planet earth, and should never be done.
                PutLogInit(LogLevel::Info, "Patching sound system.");
                uintptr_t cppFuncIntermediary;
                uint8_t* cppFuncPtr;
                size_t llsOpenALSize = sizeof(llsOpenAL);

                //Patch size of new operator
                PatchMemory(GetPatchPoint(PatchPoint::LLSSizeOf), (uint8_t*)&llsOpenALSize, sizeof(llsOpenALSize));

                //Use diabolical hack to patch constructor
                llsOpenAL* (llsOpenAL:: * llsOpenALConstructor)() = &llsOpenAL::llsOpenAL_PatchConstructor;
                cppFuncIntermediary = (uintptr_t)&llsOpenALConstructor;
                cppFuncPtr = *((uint8_t**)cppFuncIntermediary);
                CreateCallTo(GetPatchPoint(PatchPoint::LLSConstructor), (uintptr_t)cppFuncPtr);
            }

            PutLogInit(LogLevel::Info, "Patching complete, game starting.");
        }
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        //Close the log to hopefully ensure its flushed.
        PutLog(LogLevel::Info, "Patcher DLL detaching.");
        CloseLog();
    }
    return TRUE;
}

void CreateCallTo(uintptr_t srcAddress, uintptr_t funcAddress)
{
    uint8_t callByte = 0xE8;
    uintptr_t offset = (funcAddress)- (srcAddress + 5);

    PatchMemory(srcAddress, &callByte, sizeof(callByte));
    PatchMemory(srcAddress + 1, (uint8_t*)&offset, sizeof(offset));
}

void CreateJmpTo(uintptr_t srcAddress, uintptr_t funcAddress)
{
    uint8_t callByte = 0xE9;
    uintptr_t offset = (funcAddress)-(srcAddress + 5);

    PatchMemory(srcAddress, &callByte, sizeof(callByte));
    PatchMemory(srcAddress + 1, (uint8_t*)&offset, sizeof(offset));
}

void PatchMemory(uintptr_t address, uint8_t* data, size_t len) 
{
    DWORD oldProtect = 0;

    VirtualProtect((LPVOID)address, len, PAGE_EXECUTE_READWRITE, &oldProtect);
    for (uint32_t i = 0; i < len; i++)
    {
        *(uint8_t*)(address+(i)) = data[i];
    }
    VirtualProtect((LPVOID)address, len, oldProtect, &oldProtect);
}

void NOPMemory(uintptr_t address, size_t count)
{
    DWORD oldProtect = 0;

    VirtualProtect((LPVOID)address, count, PAGE_EXECUTE_READWRITE, &oldProtect);
    for (uint32_t i = 0; i < count; i++)
    {
        *(uint8_t*)(address + (i)) = 0x90;
    }
    VirtualProtect((LPVOID)address, count, oldProtect, &oldProtect);
}
