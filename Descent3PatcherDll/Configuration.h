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
//This will eventually hold user-configuration for the hack.
#pragma once

//Which patches to apply
#define SCREENMODE_ORIGINAL 0
#define SCREENMODE_WINDOWED 1
#define SCREENMODE_BORDERLESS 2
extern int PatchScreenMode;
extern bool PatchMouseLibrary;
extern bool PatchSoundSystem;
extern bool PatchRegistryRoot;

//Configuration switches
extern bool ConfigForce32BitMode;
extern bool ConfigForceZ32;
extern int ConfigMultisampleCount;
extern bool ConfigUseReverbs;

//Automatically determined options
extern bool AutoUseSubBuffer;
extern bool AutoPatchOpenGL;

//Stores the number of the monitor to center the window on
extern int DisplayNum;
//Stores the FOV to replace the default with.
extern float DefaultFov;

//Bypass Katmai checks and always use SSE features.
extern bool AlwaysKatmai;

//Not really configuration, but need it everywhere

enum class LogLevel
{
	Info,
	Warning,
	Error
};

void LoadConfig();

void OpenLog(const char* filename);
void PutLog(LogLevel level, const char* fmt, ...);
//like PutLog, but doesn't call timer_GetTime.
void PutLogInit(LogLevel level, const char* fmt, ...);
void CloseLog();
