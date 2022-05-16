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
#include <stdarg.h>
#include <fstream>

#include "Win32App.h"
#include "Configuration.h"
#include "NewMouse.h"

int PatchScreenMode = SCREENMODE_WINDOWED;
bool PatchMouseLibrary = true;
bool PatchSoundSystem = true;
bool PatchOpenGLSpecular = false;

bool ConfigForce32BitMode = true;
bool ConfigForceZ32 = true;
bool ConfigFogHint = true;
int ConfigMultisampleCount = 1;

int DisplayNum = 0;
float DefaultFov = 72.0f;

bool AutoPatchOpenGL = false;
bool AutoUseSubBuffer = false;
float UIFrameRate = 20.0f;

bool AlwaysKatmai = false;

extern float SoundDopplerMult;

//TODO: Ugh, doesn't this feel relatively crusty and ancient?
enum class ConfigType
{
	Boolean,
	Float,
	Integer
};

struct ConfigEntry
{
	const char* name;
	ConfigType type;
	void* ptr;
};

ConfigEntry configList[] = {
	{"ScreenMode", ConfigType::Integer, &PatchScreenMode},
	{"NewMouse", ConfigType::Boolean, &PatchMouseLibrary},
	{"NewSoundSystem", ConfigType::Boolean, &PatchSoundSystem},
	{"Force32Bit", ConfigType::Boolean, &ConfigForce32BitMode},
	{"ForceZ32", ConfigType::Boolean, &ConfigForceZ32},
	{"MultisampleCount", ConfigType::Integer, &ConfigMultisampleCount},
	{"DisplayNum", ConfigType::Integer, &DisplayNum},
	{"FieldOfView", ConfigType::Float, &DefaultFov},
	{"NewSoundSystemReverbs", ConfigType::Boolean, &ConfigUseReverbs},
	{"UseUserRegistry", ConfigType::Boolean, &PatchRegistryRoot},
	{"MouseScalar", ConfigType::Float, &MouseScalar},
	{"AlwaysKatmai", ConfigType::Boolean, &AlwaysKatmai},
	{"PatchOpenGLFog", ConfigType::Boolean, &ConfigFogHint},
	{"OpenGLSpecular", ConfigType::Boolean, &PatchOpenGLSpecular},
	{"UIFrameRate", ConfigType::Float, &UIFrameRate},
	{"DopplerFactor", ConfigType::Float, &SoundDopplerMult},
};

#define CONFIGLISTSIZE (sizeof(configList) / sizeof(*configList))

//I apologize for how lazy this configuration parser is.
void ParseConfig(const char* filename)
{
	std::ifstream stream = std::ifstream(filename);
	if (!stream.is_open())
	{
		PutLog(LogLevel::Error, "Failed to open %s. Using default configuration options", filename);
		return;
	}
	std::string hack, key, value;
	int ivalue;
	float fvalue;
	int position;
	int i;
	for (;;)
	{
		stream >> hack;
		if (stream.eof())
			break;

		//PutLogInit(LogLevel::Info, "%s", hack.c_str());
		position = hack.find('=', 0);
		if (position == std::string::npos)
		{
			PutLogInit(LogLevel::Warning, "Token %s lacks = character.", hack.c_str());
		}
		else
		{
			key = hack.substr(0, position);
			value = hack.substr(position + 1);

			//todo: from_chars or something else I guess
			ivalue = atoi(value.c_str());
			//PutLogInit(LogLevel::Info, "%s=%d", key.c_str(), ivalue);

			//Find the key. TODO: Stupid linear search atm
			for (i = 0; i < CONFIGLISTSIZE; i++)
			{
				if (!strcmp(configList[i].name, key.c_str()))
				{
					if (configList[i].type == ConfigType::Boolean)
					{
						bool* ptr = (bool*)configList[i].ptr;
						*ptr = ivalue != 0;
					}
					else if (configList[i].type == ConfigType::Float)
					{
						fvalue = atof(value.c_str());
						float* fptr = (float*)configList[i].ptr;
						*fptr = fvalue;
					}
					else
					{
						int* iptr = (int*)configList[i].ptr;
						*iptr = ivalue;
					}
					break;
				}
			}
			
			if (i == CONFIGLISTSIZE)
			{
				PutLogInit(LogLevel::Warning, "Unknown config key %s.", key.c_str());
			}
		}
	}

	stream.close();
}

void LoadConfig()
{
	PutLogInit(LogLevel::Info, "Initializing configuration.");
	ParseConfig("InjectD3.cfg");
	//Determine auto options
	if (PatchScreenMode == SCREENMODE_BORDERLESS || ConfigMultisampleCount > 1)
	{
		AutoUseSubBuffer = true;
	}

	if (PatchScreenMode || AutoUseSubBuffer || PatchOpenGLSpecular)
	{
		AutoPatchOpenGL = true;
		if (!PatchScreenMode)
			PatchScreenMode = SCREENMODE_BORDERLESS;
	}
}

//-----------------------------------------------------------------------------
// Simple logging system.
//-----------------------------------------------------------------------------

FILE* logfile = nullptr;

const char* levels[3] = { "INFO", "WARN", "ERR " };

void OpenLog(const char* filename)
{
	logfile = fopen(filename, "w");

	if (logfile)
	{
		PutLogInit(LogLevel::Info, "Logging started successfully.");
	}
}

void PutLog(LogLevel level, const char* fmt, ...)
{
	if (!logfile) return;
	if ((int)level < 0 || (int)level >= 3) return; //shouldn't happen, but be safe

	va_list list;
	va_start(list, fmt);

	//Put header
	fprintf(logfile, "[%s] [%f] ", levels[(int)level], timer_GetTime());
	//Put message
	vfprintf(logfile, fmt, list);
	//Put EOL
	fprintf(logfile, "\n");

	va_end(list);
}

void PutLogInit(LogLevel level, const char* fmt, ...)
{
	if (!logfile) return;
	if ((int)level < 0 || (int)level >= 3) return; //shouldn't happen, but be safe

	va_list list;
	va_start(list, fmt);

	//Put header
	fprintf(logfile, "[%s] [-.------] ", levels[(int)level]);
	//Put message
	vfprintf(logfile, fmt, list);
	//Put EOL
	fprintf(logfile, "\n");

	va_end(list);
}

void CloseLog()
{
	if (logfile)
	{
		fclose(logfile);
		logfile == nullptr;
	}
}
