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
#pragma once

#include <stdint.h>

enum class GameVersion
{
	Main_1_4_1818624,
	Main_1_5_1806336,
	Main_1_5_1794048, //version available from GOG
	Pyromania_1_5_1859584,

	Unknown
};

enum class PatchPoint
{
	InitMouseCall,
	OeAppWindowedParam,
	OeAppInitWindowParams,
	RendInitrGLInitCall,
	RendSetModerGLInitCall,
	RendFliprGLCall,
	LLSSizeOf,
	LLSConstructor,
	FovVariable,
	FovDoEffects1,
	FovDoEffects2,
	FovUndoPlayerZoom,
	FovDoPlayerZoom,
	ZoomVariable,
	ZoomClearGameEffects,
	ZoomDoRespawnZoomEffect,
	ZoomInitLevel,
	RegistryRoot,
	KatmaiDisable,
	UIFrameRateCheck,
	UIFrameRateClamp,
	BitmapMakeResident,
	GameBitmaps,
	GameLightmaps,

	//funcs
	TimerGetTimeFunc,
	MemMallocSubFunc,
	MemFreeSubFunc,
	SoundLoadWaveFileFunc,
	MouseBtnDownTimeThunk,
	MouseResetThunk,
	MouseGetStateThunk,
	MouseFrameCall,

	//variables
	AppVar,
	DDIOMouseStateVar,
	DDIOMouseInitVar,
	DIMButtonsVar,
	MBQueueVar,
	SoundsVar,
	SoundFilesVar,

	//OpenGL nonsense
	OpenGLPreferredStateVar,
	OpenGLStateVar,
	OpenGLDCVar,
	OpenGLDCWndVar,
	OpenGLPackedPixelsVar,
	OpenGLMultitextureVar,
	WindowGLVar,
	UseMultitextureVar,
	rGLSetupFunc,
	rGLCloseFunc,
	rGLCheckExtensionFunc,
	rGLInitCacheFunc,
	rGLSetDefaultsFunc,
	rGLInitMultitextureFunc,
	rGLSetAlphaTypeCall,
	OpenGLLastBound,
	OpenGLPackedUploadData,
	OpenGLPackedTranslateTable,
	OpenGLPackedTranslate4444Table,
	OpenGLUploadData,
	OpenGLTranslateTable,
	OpenGLTranslate4444Table,
	OpenGLFogHint1,
	OpenGLFogHint2,
	OpenGLAlphaMultiplier,
	RendererSpecOpenGLCheck,
	OpenGLCurrentTextureObjectNum,
	OpenGLCurrentTextureUnitNum,
	OpenGLTranslateBitmapToOpenGL,
	OpenGLMakeTextureObject,

	DGLBegin,
	DGLBindTexture,
	DGLClear,
	DGLClearColor,
	DGLColor4f,
	DGLDisable,
	DGLEnable,
	DGLEnd,
	DGLTexParameteri,
	DGLTexImage2D,
	DGLMatrixMode,
	DGLLoadIdentity,
	DGLOrtho,
	DGLViewport,
	DWGLGetProcAddress,

	//Postrender patch
	NumPostrenders,
	IsPointVisible,
	//Callsites for IsPointVisible that need to be patched
	PostRenderRoomIPV1,
	PostRenderRoomIPV2,
	RenderAllTerrainObjectsIPV1,
	RenderAllTerrainObjectsIPV2
};

GameVersion DetectVersion();
bool InitPatchPoints(GameVersion ver);
uint32_t GetPatchPoint(PatchPoint point);
