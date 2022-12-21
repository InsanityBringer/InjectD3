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

#include <map>
#include <stdlib.h>
#include <Windows.h>

#include "D3Defs.h"
#include "Configuration.h"
#include "GameOffsets.h"

std::map<PatchPoint, uint32_t> PatchMap;
const char* VersionStrings[4] = { "1.4, 1,818,624 bytes", "1.4, 1,806,336 bytes", "1.4, 1,794,048", "Pyromania 1.4, 1,859,584" };

GameVersion DetectVersion()
{
	char testString[] = "savegame";

	__try
	{
		//TODO: Check more points
		//Check 1.5, 1,794,048
		if (!strncmp(testString, (const char*)0x5a5668, 8))
			return GameVersion::Main_1_5_1794048;
		//Check 1.4, 1,818,624 bytes
		else if (!strncmp(testString, (const char*)0x5ab668, 8))
			return GameVersion::Main_1_4_1818624;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
	{
		PutLogInit(LogLevel::Error, "Trapped access violation detecting version, executable version unknown.");
		return GameVersion::Unknown;
	}

	return GameVersion::Unknown;
}

bool InitPatchPoints(GameVersion ver)
{
	if (ver == GameVersion::Unknown)
	{
		PutLogInit(LogLevel::Error, "Can't identify Descent 3 version, no patches performed.");
		return false;
	}
	PutLogInit(LogLevel::Info, "Loading offsets for version %s.", VersionStrings[(int)ver]);
	switch (ver)
	{
	case GameVersion::Main_1_4_1818624:
		PatchMap[PatchPoint::InitMouseCall] = 0x502b0c;
		PatchMap[PatchPoint::OeAppWindowedParam] = 0x4f7c6a;
		PatchMap[PatchPoint::OeAppInitWindowParams] = 0x555585;
		PatchMap[PatchPoint::RendInitrGLInitCall] = 0x531ed4;
		PatchMap[PatchPoint::RendSetModerGLInitCall] = 0x53f9b8;
		PatchMap[PatchPoint::RendFliprGLCall] = 0x5323c6;
		PatchMap[PatchPoint::LLSSizeOf] = 0x53fe67;
		PatchMap[PatchPoint::LLSConstructor] = 0x53fe85;
		PatchMap[PatchPoint::FovVariable] = 0x5ab504;
		PatchMap[PatchPoint::FovDoEffects1] = 0x4a37ce;
		PatchMap[PatchPoint::FovDoEffects2] = 0x59d1b0;
		PatchMap[PatchPoint::FovUndoPlayerZoom] = 0x4f4233;
		PatchMap[PatchPoint::FovDoPlayerZoom] = 0x4f42A4;
		PatchMap[PatchPoint::ZoomVariable] = 0x5ab508;
		PatchMap[PatchPoint::ZoomClearGameEffects] = 0x441faf;
		PatchMap[PatchPoint::ZoomDoRespawnZoomEffect] = 0x4bbac0;
		PatchMap[PatchPoint::ZoomInitLevel] = 0x446b18;
		PatchMap[PatchPoint::TimerGetTimeFunc] = 0x504900;
		PatchMap[PatchPoint::MemMallocSubFunc] = 0x564230;
		PatchMap[PatchPoint::RegistryRoot] = 0x555009;
		PatchMap[PatchPoint::KatmaiDisable] = 0x4f7830;
		PatchMap[PatchPoint::UIFrameRateCheck] = 0x55fa5a;
		PatchMap[PatchPoint::UIFrameRateClamp] = 0x55fa72;
		PatchMap[PatchPoint::BitmapMakeResident] = 0x4fc8e9;
		PatchMap[PatchPoint::GameBitmaps] = 0xc4c448;
		PatchMap[PatchPoint::GameLightmaps] = 0xcabbf8;

		//mouse things
		PatchMap[PatchPoint::AppVar] = 0x63f15c;
		PatchMap[PatchPoint::DDIOMouseStateVar] = 0xe1d110;
		PatchMap[PatchPoint::DDIOMouseInitVar] = 0xe1d1b8;
		PatchMap[PatchPoint::DIMButtonsVar] = 0xe1d160;
		PatchMap[PatchPoint::MBQueueVar] = 0xe1d0c8;
		PatchMap[PatchPoint::MouseBtnDownTimeThunk] = 0x504fd0;
		PatchMap[PatchPoint::MouseResetThunk] = 0x504cb0;
		PatchMap[PatchPoint::MouseGetStateThunk] = 0x504ed0;
		PatchMap[PatchPoint::MouseFrameCall] = 0x502b65;

		//sound things
		PatchMap[PatchPoint::SoundsVar] = 0xdfa8d8;
		PatchMap[PatchPoint::SoundFilesVar] = 0x0e0d1b8;
		PatchMap[PatchPoint::SoundLoadWaveFileFunc] = 0x501e60;

		//opengl renderer things
		PatchMap[PatchPoint::OpenGLPreferredStateVar] = 0x5b2640;
		PatchMap[PatchPoint::OpenGLStateVar] = 0xf3f340;
		PatchMap[PatchPoint::OpenGLDCVar] = 0xf40104;
		PatchMap[PatchPoint::OpenGLDCWndVar] = 0xf39368;
		PatchMap[PatchPoint::OpenGLPackedPixelsVar] = 0xf40114;
		PatchMap[PatchPoint::OpenGLMultitextureVar] = 0xf40118;
		PatchMap[PatchPoint::WindowGLVar] = 0x0f35a8c;
		PatchMap[PatchPoint::UseMultitextureVar] = 0xf35a73;
		PatchMap[PatchPoint::rGLSetupFunc] = 0x53ca20;
		PatchMap[PatchPoint::rGLCloseFunc] = 0x53d0a0;
		PatchMap[PatchPoint::rGLCheckExtensionFunc] = 0x53c560;
		PatchMap[PatchPoint::rGLInitCacheFunc] = 0x53c690;
		PatchMap[PatchPoint::rGLSetDefaultsFunc] = 0x53c7c0;
		PatchMap[PatchPoint::rGLInitMultitextureFunc] = 0x53c510;
		PatchMap[PatchPoint::OpenGLLastBound] = 0xf3ed00;
		PatchMap[PatchPoint::OpenGLPackedUploadData] = 0xf40138;
		PatchMap[PatchPoint::OpenGLPackedTranslateTable] = 0xf4013c;
		PatchMap[PatchPoint::OpenGLPackedTranslate4444Table] = 0xf40140;
		PatchMap[PatchPoint::OpenGLUploadData] = 0xf4012c;
		PatchMap[PatchPoint::OpenGLTranslateTable] = 0xf40130;
		PatchMap[PatchPoint::OpenGLTranslate4444Table] = 0xf40134;
		PatchMap[PatchPoint::OpenGLFogHint1] = 0x53cc18 - 0x310;
		PatchMap[PatchPoint::OpenGLFogHint2] = 0x53ccba - 0x310;
		PatchMap[PatchPoint::OpenGLAlphaMultiplier] = 0x5ac644 + 0x5FF8;
		PatchMap[PatchPoint::rGLSetAlphaTypeCall] = 0x532eb4 - 784;
		PatchMap[PatchPoint::RendererSpecOpenGLCheck] = 0x4c5468 - 0x400;
		PatchMap[PatchPoint::OpenGLCurrentTextureObjectNum] = 0x5ac638 + 0x5FF8;
		PatchMap[PatchPoint::OpenGLCurrentTextureUnitNum] = 0x5ac63c + 0x5FF8;
		PatchMap[PatchPoint::OpenGLTranslateBitmapToOpenGL] = 0x53d240;
		PatchMap[PatchPoint::OpenGLMakeTextureObject] = 0x53c5e0;

		//opengl function pointers
		//TODO: these can be dropped and my code can wglGetProcAddress things myself.
		PatchMap[PatchPoint::DGLBegin] = 0xf40040;
		PatchMap[PatchPoint::DGLBindTexture] = 0xf40044;
		PatchMap[PatchPoint::DGLClear] = 0xf4004c;
		PatchMap[PatchPoint::DGLClearColor] = 0xf40050;
		PatchMap[PatchPoint::DGLColor4f] = 0xf40060;
		PatchMap[PatchPoint::DGLDisable] = 0xf40078;
		PatchMap[PatchPoint::DGLEnable] = 0xf40088;
		PatchMap[PatchPoint::DGLEnd] = 0xf40090;
		PatchMap[PatchPoint::DGLTexParameteri] = 0xf400dc;
		PatchMap[PatchPoint::DGLTexImage2D] = 0xf400d8;
		PatchMap[PatchPoint::DGLMatrixMode] = 0xf400b0;
		PatchMap[PatchPoint::DGLLoadIdentity] = 0xf400ac;
		PatchMap[PatchPoint::DGLOrtho] = 0xf400b4;
		PatchMap[PatchPoint::DGLViewport] = 0xf400f0;
		PatchMap[PatchPoint::DWGLGetProcAddress] = 0xf40038;

		return true;

		case GameVersion::Main_1_5_1794048:

		PatchMap[PatchPoint::InitMouseCall] = 0x502f2c;
		PatchMap[PatchPoint::OeAppWindowedParam] = 0x4f807a;
		PatchMap[PatchPoint::OeAppInitWindowParams] = 0x555895;
		PatchMap[PatchPoint::RendInitrGLInitCall] = 0x5321e4;
		PatchMap[PatchPoint::RendSetModerGLInitCall] = 0x53fcc8;
		PatchMap[PatchPoint::RendFliprGLCall] = 0x5326d6;
		PatchMap[PatchPoint::LLSSizeOf] = 0x540177;
		PatchMap[PatchPoint::LLSConstructor] = 0x540195;
		PatchMap[PatchPoint::FovVariable] = 0x5a5504;
		PatchMap[PatchPoint::FovDoEffects1] = 0x4a3bce;
		PatchMap[PatchPoint::FovDoEffects2] = 0x597160;
		PatchMap[PatchPoint::FovUndoPlayerZoom] = 0x4f4643;
		PatchMap[PatchPoint::FovDoPlayerZoom] = 0x4f46b4;
		PatchMap[PatchPoint::ZoomVariable] = 0x5a5508;
		PatchMap[PatchPoint::ZoomClearGameEffects] = 0x441f6f;
		PatchMap[PatchPoint::ZoomDoRespawnZoomEffect] = 0x4bbec0;
		PatchMap[PatchPoint::ZoomInitLevel] = 0x446ad8;
		PatchMap[PatchPoint::TimerGetTimeFunc] = 0x504bf0;
		PatchMap[PatchPoint::MemMallocSubFunc] = 0x564580;
		PatchMap[PatchPoint::RegistryRoot] = 0x555319;
		PatchMap[PatchPoint::KatmaiDisable] = 0x04f7c40;
		PatchMap[PatchPoint::UIFrameRateCheck] = 0x55fd9a;
		PatchMap[PatchPoint::UIFrameRateClamp] = 0x55fdb0;
		PatchMap[PatchPoint::BitmapMakeResident] = 0x4fccf9;
		PatchMap[PatchPoint::GameBitmaps] = 0xc456b0;
		PatchMap[PatchPoint::GameLightmaps] = 0xca4e60;

		//mouse things
		PatchMap[PatchPoint::AppVar] = 0x6383c4;
		PatchMap[PatchPoint::DDIOMouseStateVar] = 0xe16378;
		PatchMap[PatchPoint::DDIOMouseInitVar] = 0xe16420;
		PatchMap[PatchPoint::DIMButtonsVar] = 0xe163c8;
		PatchMap[PatchPoint::MBQueueVar] = 0xe16330;
		PatchMap[PatchPoint::MouseBtnDownTimeThunk] = 0x5052c0;
		PatchMap[PatchPoint::MouseResetThunk] = 0x504fa0;
		PatchMap[PatchPoint::MouseGetStateThunk] = 0x5051c0;
		PatchMap[PatchPoint::MouseFrameCall] = 0x502f85;

		//sound things
		PatchMap[PatchPoint::SoundsVar] = 0xdf3b40;
		PatchMap[PatchPoint::SoundFilesVar] = 0xe06420;
		PatchMap[PatchPoint::SoundLoadWaveFileFunc] = 0x502280;

		//opengl renderer things
		PatchMap[PatchPoint::OpenGLPreferredStateVar] = 0x5ac648;
		PatchMap[PatchPoint::OpenGLStateVar] = 0xf385a8;
		PatchMap[PatchPoint::OpenGLDCVar] = 0xf3936c;
		PatchMap[PatchPoint::OpenGLDCWndVar] = 0xf39368;
		PatchMap[PatchPoint::OpenGLPackedPixelsVar] = 0xf3937c;
		PatchMap[PatchPoint::OpenGLMultitextureVar] = 0xf39380;
		PatchMap[PatchPoint::WindowGLVar] = 0xf2ecf4;
		PatchMap[PatchPoint::UseMultitextureVar] = 0xf2ecdb;
		PatchMap[PatchPoint::rGLSetupFunc] = 0x53cd30;
		PatchMap[PatchPoint::rGLCloseFunc] = 0x53d3b0;
		PatchMap[PatchPoint::rGLCheckExtensionFunc] = 0x53c870;
		PatchMap[PatchPoint::rGLInitCacheFunc] = 0x53c9a0;
		PatchMap[PatchPoint::rGLSetDefaultsFunc] = 0x53cad0;
		PatchMap[PatchPoint::rGLInitMultitextureFunc] = 0x53c820;
		PatchMap[PatchPoint::rGLSetAlphaTypeCall] = 0x532eb4;
		PatchMap[PatchPoint::OpenGLLastBound] = 0xf37f68;
		PatchMap[PatchPoint::OpenGLPackedUploadData] = 0xf393a0;
		PatchMap[PatchPoint::OpenGLPackedTranslateTable] = 0xf393a4;
		PatchMap[PatchPoint::OpenGLPackedTranslate4444Table] = 0xf393a8;
		PatchMap[PatchPoint::OpenGLUploadData] = 0xf39394;
		PatchMap[PatchPoint::OpenGLTranslateTable] = 0xf39398;
		PatchMap[PatchPoint::OpenGLTranslate4444Table] = 0xf3939c;
		PatchMap[PatchPoint::OpenGLFogHint1] = 0x53cc18;
		PatchMap[PatchPoint::OpenGLFogHint2] = 0x53ccba;
		PatchMap[PatchPoint::OpenGLAlphaMultiplier] = 0x5ac644;
		PatchMap[PatchPoint::RendererSpecOpenGLCheck] = 0x4c5468;
		PatchMap[PatchPoint::OpenGLCurrentTextureObjectNum] = 0x5ac638;
		PatchMap[PatchPoint::OpenGLCurrentTextureUnitNum] = 0x5ac63c;
		PatchMap[PatchPoint::OpenGLTranslateBitmapToOpenGL] = 0x53d550;
		PatchMap[PatchPoint::OpenGLMakeTextureObject] = 0x53c8f0;

		//opengl function pointers
		//TODO: these can be dropped and my code can wglGetProcAddress things myself.
		PatchMap[PatchPoint::DGLBegin] = 0xf392a8;
		PatchMap[PatchPoint::DGLBindTexture] = 0xf392ac;
		PatchMap[PatchPoint::DGLClear] = 0xf392b4;
		PatchMap[PatchPoint::DGLClearColor] = 0xf392b8;
		PatchMap[PatchPoint::DGLColor4f] = 0xf392c8;
		PatchMap[PatchPoint::DGLDisable] = 0xf392e0;
		PatchMap[PatchPoint::DGLEnable] = 0xf392f0;
		PatchMap[PatchPoint::DGLEnd] = 0xf392f8;
		PatchMap[PatchPoint::DGLTexParameteri] = 0xf39344;
		PatchMap[PatchPoint::DGLTexImage2D] = 0xf39340;
		PatchMap[PatchPoint::DGLMatrixMode] = 0xf39318;
		PatchMap[PatchPoint::DGLLoadIdentity] = 0xf39314;
		PatchMap[PatchPoint::DGLOrtho] = 0xf3931c;
		PatchMap[PatchPoint::DGLViewport] = 0xf39358;
		PatchMap[PatchPoint::DWGLGetProcAddress] = 0xf392a0;

		return true;
	}

	return false;
}

uint32_t GetPatchPoint(PatchPoint point)
{
	return PatchMap[point];
}
