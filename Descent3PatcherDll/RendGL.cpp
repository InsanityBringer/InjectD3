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

#include <algorithm>
#include <vector>

#include "Win32App.h"
#include "Rend.h"
#include "Memory.h"
#include "GrDefs.h"
#include "Configuration.h"
#include "GameOffsets.h"
#include "Bitmap.h"

#define MAINCODE
#include "GLLoad.h"

//Pointers to original game OpenGL state
HWND* phOpenGLWnd;
HDC* phOpenGLDC;
HGLRC* pResourceContext;
bool* pOpenGLInitialized;
rendering_state* pOpenGL_state;
float* pAlpha_multiplier;
renderer_preferred_state* pOpenGL_preferred_state;
HMODULE* pOpenGLDLLHandle;
int* pAlready_loaded;
int* pCur_texture_object_num;
int* pCur_texture_unit_num;

uint32_t* opengl_Upload_data = NULL;
uint32_t* opengl_Translate_table = NULL;
uint32_t* opengl_4444_translate_table = NULL;
uint16_t* opengl_packed_Upload_data = NULL;
uint16_t* opengl_packed_Translate_table = NULL;
uint16_t* opengl_packed_4444_translate_table = NULL;
uint16_t* OpenGL_bitmap_remap;
uint16_t* OpenGL_lightmap_remap;
uint8_t* OpenGL_bitmap_states;
uint8_t* OpenGL_lightmap_states;

int* pOpenGL_sets_this_frame[10];
int* pOpenGL_packed_pixels;
int* pOpenGL_multitexture;
int* OpenGL_cache_initted;
int* pOpenGL_last_bound;
int* pWindowGL;

bool* pUseMultitexture;

//true fullscreen resolution
int screenWidth, screenHeight;
//screen rectangle
int screenOffsetX, screenOffsetY, screenOffsetW, screenOffsetH;

//Framebuffer nonsense
GLuint framebufferHandle, colorTextureHandle, depthTextureHandle, colorSubTextureHandle, depthSubTextureHandle, subFramebufferHandle;

//Pointers to old functions, loaded from rGL_Init
bool functionsLoaded = false;
int (*rGL_Setup)(HDC glhdc);
void (*rGL_Close)();
int (*rGL_CheckExtension)(char* extName);
int (*rGL_InitCache)();
void (*rGL_SetDefaults)();
void (*rGL_InitMultitexture)();

//Texture list
GLuint texture_name_list[10000];

/*void __stdcall glBindTextureShim(GLenum target, GLuint texture)
{
	if (texture >= 10000) return;

	glBindTexture(target, texture_name_list[texture]);
}*/

//Hack state
struct Monitor
{
	HMONITOR hMonitor;
	HDC hDC;
	RECT rect;
};

std::vector<Monitor> monitors = std::vector<Monitor>();

/*
void rGL_Close()
{
}*/

BOOL Monitorenumproc(HMONITOR hMonitor, HDC hDC, LPRECT pRect, LPARAM lParam)
{
	Monitor monitor;
	monitor.hMonitor = hMonitor;
	monitor.hDC = hDC;
	monitor.rect = *pRect;

	monitors.push_back(monitor);

	return TRUE;
}

const char* GLErrors[5] = { "Invalid enum", "Invalid name", "Invalid operation", "Invalid value", "Invalid framebuffer operation" };

bool GLErrorCheck(const char* context)
{
	int error;
	error = glGetError();
	const char* msg = "";
	if (error != GL_NO_ERROR)
	{
		if (error == GL_INVALID_ENUM)
			msg = GLErrors[0];
		//else if (error == GL_INVALID_NAME)
		//	msg = GLErrors[1];
		else if (error == GL_INVALID_OPERATION)
			msg = GLErrors[2];
		else if (error == GL_INVALID_VALUE)
			msg = GLErrors[3];
		else if (error == GL_INVALID_FRAMEBUFFER_OPERATION)
			msg = GLErrors[4];
		else
			msg = "Unknown error code";

		PutLog(LogLevel::Error, "Error in context %s: %s.", context, msg);
		return true;
	}
	return false;
}

int rGL_Init(oeWin32Application* app, renderer_preferred_state* pref_state)
{
	int width, height;
	int retval = 1;

	//Load pointers
	if (!functionsLoaded)
	{
		pOpenGL_last_bound = (int*)GetPatchPoint(PatchPoint::OpenGLLastBound);
		pOpenGL_preferred_state = (renderer_preferred_state*)GetPatchPoint(PatchPoint::OpenGLPreferredStateVar);
		pOpenGL_state = (rendering_state*)GetPatchPoint(PatchPoint::OpenGLStateVar);
		phOpenGLDC = (HDC*)GetPatchPoint(PatchPoint::OpenGLDCVar);
		phOpenGLWnd = (HWND*)GetPatchPoint(PatchPoint::OpenGLDCWndVar);
		pOpenGL_packed_pixels = (int*)GetPatchPoint(PatchPoint::OpenGLPackedPixelsVar);
		pOpenGL_multitexture = (int*)GetPatchPoint(PatchPoint::OpenGLMultitextureVar);
		pWindowGL = (int*)GetPatchPoint(PatchPoint::WindowGLVar);
		pAlpha_multiplier = (float*)GetPatchPoint(PatchPoint::OpenGLAlphaMultiplier);
		pCur_texture_object_num = (int*)GetPatchPoint(PatchPoint::OpenGLCurrentTextureObjectNum);
		pCur_texture_unit_num = (int*)GetPatchPoint(PatchPoint::OpenGLCurrentTextureUnitNum);

		pUseMultitexture = (bool*)GetPatchPoint(PatchPoint::UseMultitextureVar);

		rGL_Setup = (int(*)(HDC))GetPatchPoint(PatchPoint::rGLSetupFunc);
		rGL_Close = (void(*)())GetPatchPoint(PatchPoint::rGLCloseFunc);
		rGL_CheckExtension = (int(*)(char*))GetPatchPoint(PatchPoint::rGLCheckExtensionFunc);
		rGL_InitCache = (int(*)())GetPatchPoint(PatchPoint::rGLInitCacheFunc);
		rGL_SetDefaults = (void(*)())GetPatchPoint(PatchPoint::rGLSetDefaultsFunc);
		rGL_InitMultitexture = (void(*)())GetPatchPoint(PatchPoint::rGLInitMultitextureFunc);

		PutLog(LogLevel::Info, "Loaded OpenGL library functions.");
		functionsLoaded = true;
	}

	if (pref_state)
		memcpy(pOpenGL_preferred_state, pref_state, sizeof(renderer_preferred_state));

	if (ConfigForce32BitMode)
		pOpenGL_preferred_state->bit_depth = 32;

	memset(pOpenGL_state, 0, sizeof(rendering_state));

	if (app != nullptr)
	{
		*phOpenGLWnd = (HWND)((oeWin32Application*)app)->m_hWnd;
	}

	*phOpenGLDC = GetDC(*phOpenGLWnd);

	//Position the window on screen. 
	//width and height are always the preferred sizes in this version of the code.
	width = pOpenGL_preferred_state->width;
	height = pOpenGL_preferred_state->height;

	//rect will contain the centered coordinates to adjust. 
	RECT rect;
	int baseX, baseY, centerX, centerY, mWidth, mHeight;
	bool useOldMethod = false;
	rect.left = 0; rect.right = pOpenGL_preferred_state->width;
	rect.top = 0; rect.bottom = pOpenGL_preferred_state->height;

	//Attempt to get the coordinates of the user's preferred monitor
	monitors.clear();
	if (EnumDisplayMonitors(nullptr, nullptr, (MONITORENUMPROC)&Monitorenumproc, 0))
	{
		if (DisplayNum >= 0 && DisplayNum < monitors.size())
		{
			mWidth = monitors[DisplayNum].rect.right - monitors[DisplayNum].rect.left;
			mHeight = monitors[DisplayNum].rect.bottom - monitors[DisplayNum].rect.top;

			centerX = (mWidth / 2);
			centerY = (mHeight / 2);
			baseX = monitors[DisplayNum].rect.left;
			baseY = monitors[DisplayNum].rect.top;
		}
		else
			useOldMethod = true;
	}
	else
		useOldMethod = true;

	if (useOldMethod)
	{
		baseX = baseY = 0;
		mWidth = GetSystemMetrics(SM_CXSCREEN);
		mHeight = GetSystemMetrics(SM_CYSCREEN);
		centerX = mWidth / 2;
		centerY = mHeight / 2;
	}

	if (PatchScreenMode == SCREENMODE_WINDOWED)
	{
		centerX += baseX - (width / 2);
		centerY += baseY - (height / 2);
		rect.left += centerX; rect.right += centerX;
		rect.top += centerY; rect.bottom += centerY;

		AdjustWindowRectEx(&rect, WS_CAPTION, FALSE, 0);

		screenOffsetX = screenOffsetY = 0;
		screenOffsetW = width;
		screenOffsetH = height;
	}
	else
	{
		float baseAspect = width / (float)height;
		float trueAspect = mWidth / (float)mHeight;

		if (baseAspect < trueAspect) //base screen is less wide, so pillarbox it
		{
			screenOffsetH = mHeight; screenOffsetY = 0;
			screenOffsetW = mHeight * baseAspect; screenOffsetX = (mWidth - screenOffsetW) / 2;
		}
		else //base screen is more wide, so letterbox it
		{
			screenOffsetW = mWidth; screenOffsetX = 0;
			screenOffsetH = mWidth / baseAspect; screenOffsetY = (mHeight - screenOffsetH) / 2;
		}
		rect.left = baseX;
		rect.top = baseY;
		rect.right = baseX + mWidth;
		rect.bottom = baseY + mHeight;
		if (PatchScreenMode == SCREENMODE_BORDERLESS)
			rect.right += 1; //This is filthy, but if the window is the size of the screen, it messes up
	}

	int lwidth = abs(rect.right - rect.left);
	int lheight = abs(rect.bottom - rect.top);

	screenWidth = mWidth; screenHeight = mHeight;
	MoveWindow(*phOpenGLWnd, rect.left, rect.top, lwidth, lheight, TRUE);

	//GetWindowRect((HWND)*phOpenGLWnd, &rect);
	//width = abs(rect.right - rect.left);
	//height = abs(rect.bottom - rect.top);

	pOpenGL_state->screen_width = width;
	pOpenGL_state->screen_height = height;

	/*if (PatchScreenMode == SCREENMODE_BORDERLESS)
	{
		rect.right -= 1;
	}*/


	if (!rGL_Setup(*phOpenGLDC))
	{
		PutLog(LogLevel::Error, "Failed to start OpenGL renderer.");
		rGL_Close();
		return 0;
	}

	//Copy OpenGL function pointers.
	dglBegin = *((glBegin_fp*)GetPatchPoint(PatchPoint::DGLBegin));
	dglBindTexture = *((glBindTexture_fp*)GetPatchPoint(PatchPoint::DGLBindTexture));
	dglClear = *((glClear_fp*)GetPatchPoint(PatchPoint::DGLClear));
	dglClearColor = *((glClearColor_fp*)GetPatchPoint(PatchPoint::DGLClearColor));
	dglColor4f = *((glColor4f_fp*)GetPatchPoint(PatchPoint::DGLColor4f));
	dglDisable = *((glDisable_fp*)GetPatchPoint(PatchPoint::DGLDisable));
	dglEnable = *((glEnable_fp*)GetPatchPoint(PatchPoint::DGLEnable));
	dglEnd = *((glEnd_fp*)GetPatchPoint(PatchPoint::DGLEnd));
	dglTexParameteri = *((glTexParameteri_fp*)GetPatchPoint(PatchPoint::DGLTexParameteri));
	dglTexImage2D = *((glTexImage2D_fp*)GetPatchPoint(PatchPoint::DGLTexImage2D));
	dglMatrixMode = *((glMatrixMode_fp*)GetPatchPoint(PatchPoint::DGLMatrixMode));
	dglLoadIdentity = *((glLoadIdentity_fp*)GetPatchPoint(PatchPoint::DGLLoadIdentity));
	dglOrtho = *((glOrtho_fp*)GetPatchPoint(PatchPoint::DGLOrtho));
	dglViewport = *((glViewport_fp*)GetPatchPoint(PatchPoint::DGLViewport));
	dwglGetProcAddress = *((wglGetProcAddress_fp*)GetPatchPoint(PatchPoint::DWGLGetProcAddress)); //important, since borderless fullscreen needs more functions

	//Replace the version of glBindTexture that Descent 3 loaded with my shim
	//glBindTexture_fp* pdglBindTexture = ((glBindTexture_fp*)GetPatchPoint(PatchPoint::DGLBindTexture));
	//*pdglBindTexture = &glBindTextureShim;

	dglBlendFuncSeparate = (glBlendFuncSeparate_fp)dwglGetProcAddress("glBlendFuncSeparate");
	dglActiveTexture = (glActiveTexture_fp)dwglGetProcAddress("glActiveTexture");

	dglMatrixMode(GL_PROJECTION);
	dglLoadIdentity();
	dglOrtho(0.0f, (GLfloat)width, (GLfloat)height, 0, 0, 1);
	dglViewport(0, 0, width, height);
	dglMatrixMode(GL_MODELVIEW);
	dglLoadIdentity();

	int i;
	for (i = 0; i < 10000; i++)
	{
		texture_name_list[i] = 0;
	}
	rGL_InitCache();

	*pOpenGL_packed_pixels = rGL_CheckExtension((char*)"GL_EXT_packed_pixels");
	if (*pOpenGL_packed_pixels)
		PutLog(LogLevel::Info, "Using GL_EXT_packed_pixels");
	else
		PutLog(LogLevel::Info, "GL_EXT_packed_pixels not found");
	*pOpenGL_multitexture = rGL_CheckExtension((char*)"GL_ARB_multitexture");
	if (*pOpenGL_multitexture)
		PutLog(LogLevel::Info, "Using GL_ARB_multitexture");
	else
		PutLog(LogLevel::Info, "GL_ARB_multitexture not found");
	rGL_InitMultitexture();

	if (*pOpenGL_packed_pixels)
	{
		uint16_t** popengl_packed_Upload_data = (uint16_t**)GetPatchPoint(PatchPoint::OpenGLPackedUploadData);
		uint16_t** popengl_packed_Translate_table = (uint16_t**)GetPatchPoint(PatchPoint::OpenGLPackedTranslateTable);
		uint16_t** popengl_packed_4444_translate_table = (uint16_t**)GetPatchPoint(PatchPoint::OpenGLPackedTranslate4444Table);
		opengl_packed_Upload_data = (uint16_t*)mem_malloc_sub(256 * 256 * 2, (char*)"opengl_packed_Upload_data", 0);
		opengl_packed_Translate_table = (uint16_t*)mem_malloc_sub(65536 * 2, (char*)"opengl_packed_Translate_table", 0);
		opengl_packed_4444_translate_table = (uint16_t*)mem_malloc_sub(65536 * 2, (char*)"opengl_packed_4444_translate_table", 0);

		//mprintf((0, "Building packed OpenGL translate table...\n"));

		for (i = 0; i < 65536; i++)
		{
			int r = (i >> 10) & 0x1f;
			int g = (i >> 5) & 0x1f;
			int b = i & 0x1f;

			uint16_t pix;

			if (!(i & OPAQUE_FLAG))
				pix = 0;
			else
				pix = (r << 11) | (g << 6) | (b << 1) | 1;

			opengl_packed_Translate_table[i] = pix;

			// 4444 table
			int a = (i >> 12) & 0xf;
			r = (i >> 8) & 0xf;
			g = (i >> 4) & 0xf;
			b = i & 0xf;

			//a = 0xf; //only when WindowGL is set, which isn't ever in the original game I think. 

			pix = (r << 12) | (g << 8) | (b << 4) | a;

			opengl_packed_4444_translate_table[i] = pix;

			*popengl_packed_Upload_data = opengl_packed_Upload_data;
			*popengl_packed_Translate_table = opengl_packed_Translate_table;
			*popengl_packed_4444_translate_table = opengl_packed_4444_translate_table;
		}
	}
	else
	{
		uint32_t** popengl_Upload_data = (uint32_t**)GetPatchPoint(PatchPoint::OpenGLUploadData);
		uint32_t** popengl_Translate_table = (uint32_t**)GetPatchPoint(PatchPoint::OpenGLTranslateTable);
		uint32_t** popengl_4444_translate_table = (uint32_t**)GetPatchPoint(PatchPoint::OpenGLTranslate4444Table);
		opengl_Upload_data = (uint32_t*)mem_malloc_sub(256 * 256 * 4, (char*)"opengl_Upload_data", 0);
		opengl_Translate_table = (uint32_t*)mem_malloc_sub(65536 * 4, (char*)"opengl_Translate_table", 0);
		opengl_4444_translate_table = (uint32_t*)mem_malloc_sub(65536 * 4, (char*)"opengl_4444_translate_table", 0);

		//mprintf((0, "Building OpenGL translate table...\n"));

		for (i = 0; i < 65536; i++)
		{
			uint32_t pix = GR_16_TO_COLOR(i);
			/*int r = GR_COLOR_RED(pix);
			int g = GR_COLOR_GREEN(pix);
			int b = GR_COLOR_BLUE(pix);*/
			int r = ((i >> 10) & 31) / 31.0f * 255.0f;
			int g = ((i >> 5) & 31) / 31.0f * 255.0f;
			int b = (i & 31) / 31.0f * 255.0f;

			if (!(i & OPAQUE_FLAG))
				pix = 0;
			else
				pix = (255 << 24) | (b << 16) | (g << 8) | (r);

			opengl_Translate_table[i] = pix;

			// Do 4444
			int a = ((i >> 12) & 0xf) / 16.0f * 255.0f;
			r = ((i >> 8) & 0xf) / 16.0f * 255.0f;
			g = ((i >> 4) & 0xf) / 16.0f * 255.0f;
			b = (i & 0xf) / 16.0f * 255.0f;

			//a = 0xf;
			pix = (a << 24) | (b << 16) | (g << 8) | (r);

			opengl_4444_translate_table[i] = pix;

			*popengl_Upload_data = opengl_Upload_data;
			*popengl_Translate_table = opengl_Translate_table;
			*popengl_4444_translate_table = opengl_4444_translate_table;
		}
	}

	if (*pOpenGL_multitexture)
	{
		*pUseMultitexture = true;
	}

	rGL_SetDefaults();

	//Read framebuffer functions
	if (AutoUseSubBuffer)
	{
		GLenum err = GL_NO_ERROR;
		dglBindFramebuffer = (glBindFramebuffer_fp)dwglGetProcAddress("glBindFramebuffer");
		dglGenFramebuffers = (glGenFramebuffers_fp)dwglGetProcAddress("glGenFramebuffers");
		dglDeleteFramebuffers = (glDeleteFramebuffers_fp)dwglGetProcAddress("glDeleteFramebuffers");
		dglFramebufferTexture2d = (glFramebufferTexture2d_fp)dwglGetProcAddress("glFramebufferTexture2D");
		dglCheckFramebufferStatus = (glCheckFramebufferStatus_fp)dwglGetProcAddress("glCheckFramebufferStatus");
		dglBlitFramebuffer = (glBlitFramebuffer_fp)dwglGetProcAddress("glBlitFramebuffer");
		dglTexImage2DMultisample = (glTexImage2DMultisample_fp)dwglGetProcAddress("glTexImage2DMultisample");
		PutLog(LogLevel::Info, "Getting function pointers.");

		if (ConfigMultisampleCount > 1 && !dglTexImage2DMultisample)
		{
			PutLog(LogLevel::Warning, "Disabling multisampling, can't load glTexImage2DMultisample,");
			ConfigMultisampleCount = 1;
		}

		if (dglGenFramebuffers) //check if the func loaded
		{
			GLenum fbstatus;
			GLenum textureType = GL_TEXTURE_2D;
			dglGenFramebuffers(1, &framebufferHandle);
			dglGenFramebuffers(1, &subFramebufferHandle);
			/*colorTextureHandle = 30000; //pick an arbritary high ID so Descent 3 doesn't overwrite it. Old OpenGL is fun.
			depthTextureHandle = 30001;
			colorSubTextureHandle = 30002;
			depthSubTextureHandle = 30003;*/
			glGenTextures(1, &colorTextureHandle);
			glGenTextures(1, &depthTextureHandle);
			glGenTextures(1, &colorSubTextureHandle);
			glGenTextures(1, &depthSubTextureHandle);

			if (ConfigMultisampleCount > 1)
			{
				dglBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorTextureHandle);
				//dglTexImage2D(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_RGB8, pOpenGL_state->screen_width, pOpenGL_state->screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
				dglTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, ConfigMultisampleCount, GL_RGBA8, pOpenGL_state->screen_width, pOpenGL_state->screen_height, GL_FALSE);
				GLErrorCheck("Creating framebuffer texture");
				PutLog(LogLevel::Info, "Creating color texture.");
				textureType = GL_TEXTURE_2D_MULTISAMPLE;

				dglBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureHandle);
				dglTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, ConfigMultisampleCount, GL_DEPTH_COMPONENT32F, pOpenGL_state->screen_width, pOpenGL_state->screen_height, GL_FALSE);
				GLErrorCheck("Setting framebuffer depth texture params");
				PutLog(LogLevel::Info, "Creating depth texture.");

				dglBindTexture(GL_TEXTURE_2D, colorSubTextureHandle);
				dglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, pOpenGL_state->screen_width, pOpenGL_state->screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
				GLErrorCheck("Creating framebuffer texture");
				dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				GLErrorCheck("Setting framebuffer texture params");
				PutLog(LogLevel::Info, "Creating color texture.");


				dglBindFramebuffer(GL_FRAMEBUFFER, subFramebufferHandle);
				GLErrorCheck("Binding framebuffer");
				PutLog(LogLevel::Info, "Binding sub framebuffer.");
				dglFramebufferTexture2d(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorSubTextureHandle, 0);
				fbstatus = dglCheckFramebufferStatus(GL_FRAMEBUFFER);
				if (fbstatus != GL_FRAMEBUFFER_COMPLETE)
				{
					PutLog(LogLevel::Error, "Sub framebuffer deemed incomplete %d.", fbstatus);
				}

				dglEnable(GL_MULTISAMPLE);
			}
			else
			{
				dglBindTexture(GL_TEXTURE_2D, colorTextureHandle);
				dglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, pOpenGL_state->screen_width, pOpenGL_state->screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
				GLErrorCheck("Creating framebuffer texture");
				dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				GLErrorCheck("Setting framebuffer texture params");
				PutLog(LogLevel::Info, "Creating color texture.");

				dglBindTexture(GL_TEXTURE_2D, depthTextureHandle);
				dglTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, pOpenGL_state->screen_width, pOpenGL_state->screen_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
				GLErrorCheck("Creating framebuffer depth texture");
				dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				GLErrorCheck("Setting framebuffer depth texture params");
				PutLog(LogLevel::Info, "Creating depth texture.");
			}

			dglBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle);
			GLErrorCheck("Binding framebuffer");
			PutLog(LogLevel::Info, "Binding framebuffer.");

			dglFramebufferTexture2d(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureType, colorTextureHandle, 0);
			dglFramebufferTexture2d(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureType, depthTextureHandle, 0);
			GLErrorCheck("Setting up framebuffer");
			PutLog(LogLevel::Info, "Binding framebuffer.");

			fbstatus = dglCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (fbstatus != GL_FRAMEBUFFER_COMPLETE)
			{
				PutLog(LogLevel::Error, "Framebuffer deemed incomplete %d.", fbstatus);
			}
		}
	}
	pOpenGL_state->initted = 1;
	PutLog(LogLevel::Info, "OpenGL library started successfully.");

	return retval;
}

void rGL_Flip()
{
	GLenum fbstatus;
	if (dglBindFramebuffer && AutoUseSubBuffer)
	{
		dglBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferHandle);

		//Second pass to blit MSAA framebuffer to sub framebuffer that will then be 
		if (ConfigMultisampleCount > 1)
		{
			dglBindFramebuffer(GL_DRAW_FRAMEBUFFER, subFramebufferHandle);
			dglBlitFramebuffer(0, 0, pOpenGL_state->screen_width, pOpenGL_state->screen_height, 0, 0, pOpenGL_state->screen_width, pOpenGL_state->screen_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			dglBindFramebuffer(GL_READ_FRAMEBUFFER, subFramebufferHandle);
			GLErrorCheck("Blitting to sub framebuffer.");
		}

		dglBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		dglClearColor(0.0, 0.0, 0.0, 1.0);
		dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		dglBlitFramebuffer(0, 0, pOpenGL_state->screen_width, pOpenGL_state->screen_height, screenOffsetX, screenOffsetY, screenOffsetX + screenOffsetW, screenOffsetY + screenOffsetH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		GLErrorCheck("Blitting to main framebuffer;");

		SwapBuffers(*phOpenGLDC);

		dglBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle);
	}
	else
	{
		SwapBuffers(*phOpenGLDC);
	}
}

float rGL_GetAlphaMultiplier()
{
	switch (pOpenGL_state->cur_alpha_type)
	{
	case AT_ALWAYS:
		return 1.0f;
	case AT_CONSTANT:
		return pOpenGL_state->cur_alpha / 255.0f;
	case AT_TEXTURE:
		return 1.0f;
	case AT_CONSTANT_TEXTURE:
		return pOpenGL_state->cur_alpha / 255.0f;
	case AT_VERTEX:
		return 1.0f;
	case AT_CONSTANT_TEXTURE_VERTEX:
	case AT_CONSTANT_VERTEX:
		return pOpenGL_state->cur_alpha / 255.0f;
	case AT_TEXTURE_VERTEX:
		return 1.0f;
	case AT_LIGHTMAP_BLEND:
	case AT_LIGHTMAP_BLEND_SATURATE:
		return pOpenGL_state->cur_alpha / 255.0f;
	case AT_SATURATE_TEXTURE:
		return pOpenGL_state->cur_alpha / 255.0f;
	case AT_SATURATE_VERTEX:
		return 1.0f;
	case AT_SATURATE_CONSTANT_VERTEX:
		return pOpenGL_state->cur_alpha / 255.0f;
	case AT_SATURATE_TEXTURE_VERTEX:
		return 1.0f;
	case AT_SPECULAR:
		return 1.0f;
	default:
		return 0;
	}
}

// ======================
// rGL_SetAlphaMultiplier
// ======================
//
// Sets the alpha multiply factor
void rGL_SetAlphaMultiplier()
{
	*pAlpha_multiplier = rGL_GetAlphaMultiplier();
}

// ====================
// rGL_SetAlphaValue
// ====================
//
// Sets the constant alpha value
void rGL_SetAlphaValue(uint8_t val)
{
	pOpenGL_state->cur_alpha = val;
	rGL_SetAlphaMultiplier();
}

int gl_blend_state = 1;

void rGL_SetAlphaType(int8_t atype)
{
	if (atype == pOpenGL_state->cur_alpha_type)
		return;		// don't set it redundantly

	if (atype == AT_ALWAYS)
	{
		if (gl_blend_state)
		{
			gl_blend_state = 0;
			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
		}
	}
	else if (!gl_blend_state)
	{
		gl_blend_state = 1;
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
	}

	switch (atype)
	{
	case AT_ALWAYS:
		rGL_SetAlphaValue(255);
		glBlendFunc(GL_ONE, GL_ZERO);
		break;
	case AT_CONSTANT:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case AT_TEXTURE:
		rGL_SetAlphaValue(255);
		glBlendFunc(GL_ONE, GL_ZERO);
		break;
	case AT_CONSTANT_TEXTURE:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case AT_VERTEX:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case AT_CONSTANT_TEXTURE_VERTEX:
	case AT_CONSTANT_VERTEX:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case AT_TEXTURE_VERTEX:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case AT_LIGHTMAP_BLEND:
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		break;
	case AT_SATURATE_TEXTURE:
	case AT_LIGHTMAP_BLEND_SATURATE:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case AT_SATURATE_VERTEX:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case AT_SATURATE_CONSTANT_VERTEX:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case AT_SATURATE_TEXTURE_VERTEX:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case AT_SPECULAR:
		//glBlendFunc(GL_DST_COLOR, GL_ONE);
		//glBlendFunc(GL_ONE, GL_ONE);
		//rGL_SetAlphaValue(255);
		//glBlendFunc(GL_ONE, GL_ZERO);
		//glBlendFunc(GL_DST_ALPHA, GL_ONE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glEnable(GL_TEXTURE_2D);
		
		pOpenGL_state->cur_texture_quality = 2;
		pOpenGL_state->cur_texture_type = TT_PERSPECTIVE;
		break;
	default:
		break;
	}

	pOpenGL_state->cur_alpha_type = atype;
	rGL_SetAlphaMultiplier();
}

#define GL_TEXTURE_MAX_LEVEL              0x813D

int rGL_MakeTextureObject(int tn)
{
	int iVar1;

	iVar1 = *pCur_texture_object_num;
	*pCur_texture_object_num = *pCur_texture_object_num + 1;

	//PutLog(LogLevel::Info, "Creating texture object for texture %d", iVar1);

	if (texture_name_list[iVar1] == 0)
	{
		glGenTextures(1, &texture_name_list[iVar1]);
		//PutLog(LogLevel::Info, "Generated GL texture %d", texture_name_list[iVar1]);
	}

	iVar1 = texture_name_list[iVar1];

	if ((*pOpenGL_multitexture != 0) && (*pCur_texture_unit_num != tn)) 
	{
		dglActiveTexture(GL_TEXTURE0 + tn);
		*pCur_texture_unit_num = tn;
	}

	glBindTexture(GL_TEXTURE_2D, iVar1);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
	return iVar1;
}

// Takes our 16bit format and converts it into the memory scheme that OpenGL wants
void rGL_TranslateBitmapToOpenGL(int texnum, int bm_handle, int map_type, int replace, int tn)
{
	unsigned short* bm_ptr;

	int w, h;
	int size;

	if (*pOpenGL_multitexture && *pCur_texture_unit_num != tn)
	{
		dglActiveTexture(GL_TEXTURE0 + tn);
		*pCur_texture_unit_num = tn;
	}

	//PutLog(LogLevel::Info, "Uploading handle %d of type %d to texture slot %d", bm_handle, map_type, texnum);

	if (map_type==MAP_TYPE_LIGHTMAP)
	{
		if (GameLightmaps[bm_handle].flags & LF_BRAND_NEW)
			replace=0;

		bm_ptr=lm_data (bm_handle);
		GameLightmaps[bm_handle].flags &=~(LF_CHANGED|LF_BRAND_NEW);

		w=lm_w(bm_handle);
		h=lm_h(bm_handle);
		size=GameLightmaps[bm_handle].square_res;
	}
	else
	{
		if (GameBitmaps[bm_handle].flags & BF_BRAND_NEW)
			replace = 0;

		bm_ptr = bm_data(bm_handle, 0);
		GameBitmaps[bm_handle].flags &= ~(BF_CHANGED | BF_BRAND_NEW);
		w = bm_w(bm_handle, 0);
		h = bm_h(bm_handle, 0);
		size = w;
	}

	if (pOpenGL_last_bound[tn] != texnum)
	{
		glBindTexture(GL_TEXTURE_2D, texnum);
		//*pOpenGL_sets_this_frame[0] = *pOpenGL_sets_this_frame[0] + 1; //TODO: export maybe?
		pOpenGL_last_bound[tn] = texnum;
	}

	int i;

	if (*pOpenGL_packed_pixels)
	{
		if (map_type==MAP_TYPE_LIGHTMAP)
		{
			unsigned short *left_data=(unsigned short*)opengl_packed_Upload_data;
			int bm_left=0;

			for (int i=0;i<h;i++,left_data+=size,bm_left+=w)
			{
				unsigned short *dest_data=left_data;
				for (int t=0;t<w;t++)
				{
					*dest_data++=opengl_packed_Translate_table[bm_ptr[bm_left+t]];
				}
			}

			if (replace)
			{
				glTexSubImage2D (GL_TEXTURE_2D,0,0,0,size,size,GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1_EXT,opengl_packed_Upload_data);
			}
			else
				glTexImage2D (GL_TEXTURE_2D,0,GL_RGB5_A1,size,size,0,GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1_EXT,opengl_packed_Upload_data);
		}
		else
		{
			int limit = 0;

			if (bm_mipped(bm_handle))
				limit = NUM_MIP_LEVELS + 3;
			else
				limit = 1;

			//AMD bugfix: AMD's new OpenGL implementation requires complete textures, even in compatible contexts. 
			//Set the limit of the texture's mipmap levels so it matches how many are present, since they don't go all the way down to 1x1
			//Do note that the new implementation does not expose GL_EXT_packed_pixels so this won't be executed unless it is readded later. 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, limit-1);

			for (int m = 0; m < limit; m++)
			{
				if (m < NUM_MIP_LEVELS)
				{
					bm_ptr = bm_data(bm_handle, m);
					w = bm_w(bm_handle, m);
					h = bm_h(bm_handle, m);
				}
				else
				{
					bm_ptr = bm_data(bm_handle, NUM_MIP_LEVELS - 1);
					w = bm_w(bm_handle, NUM_MIP_LEVELS - 1);
					h = bm_h(bm_handle, NUM_MIP_LEVELS - 1);

					w >>= m - (NUM_MIP_LEVELS - 1);
					h >>= m - (NUM_MIP_LEVELS - 1);

					if (w < 1)
						continue;

				}

				if (bm_format(bm_handle) == BITMAP_FORMAT_4444)
				{
					// Do 4444

					if (bm_mipped(bm_handle))
					{
						for (i = 0; i < w * h; i++)
							opengl_packed_Upload_data[i] = /*0xf |*/ (opengl_packed_4444_translate_table[bm_ptr[i]]);
					}
					else
					{
						for (i = 0; i < w * h; i++)
							opengl_packed_Upload_data[i] = opengl_packed_4444_translate_table[bm_ptr[i]];
					}

					if (replace)
						glTexSubImage2D(GL_TEXTURE_2D, m, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_EXT, opengl_packed_Upload_data);
					else
						glTexImage2D(GL_TEXTURE_2D, m, GL_RGBA4, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_EXT, opengl_packed_Upload_data);
				}
				else
				{
					// Do 1555
					for (i = 0; i < w * h; i++)
						opengl_packed_Upload_data[i] = opengl_packed_Translate_table[bm_ptr[i]];

					if (replace)
						glTexSubImage2D(GL_TEXTURE_2D, m, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1_EXT, opengl_packed_Upload_data);
					else
						glTexImage2D(GL_TEXTURE_2D, m, GL_RGB5_A1, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1_EXT, opengl_packed_Upload_data);
				}
			}
		}



	}
	else
	{
		if (map_type==MAP_TYPE_LIGHTMAP)
		{
			unsigned int *left_data=(unsigned int *)opengl_Upload_data;
			int bm_left=0;

			for (int i=0;i<h;i++,left_data+=size,bm_left+=w)
			{
				unsigned int *dest_data=left_data;
				for (int t=0;t<w;t++)
				{
					*dest_data++=opengl_Translate_table[bm_ptr[bm_left+t]];
				}
			}

			if (replace)
				glTexSubImage2D (GL_TEXTURE_2D,0,0,0,size,size,GL_RGBA,GL_UNSIGNED_BYTE,opengl_Upload_data);
			else
				glTexImage2D (GL_TEXTURE_2D,0,GL_RGBA,size,size,0,GL_RGBA,GL_UNSIGNED_BYTE,opengl_Upload_data);
		}
		else
		{
			int limit = 0;

			if (bm_mipped(bm_handle))
				limit = NUM_MIP_LEVELS;
			else
				limit = 1;

			//AMD bugfix: AMD's new OpenGL implementation requires complete textures, even in compatible contexts. 
			//Set the limit of the texture's mipmap levels so it matches how many are present, since they don't go all the way down to 1x1
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, limit-1);

			for (int m = 0; m < limit; m++)
			{
				bm_ptr = bm_data(bm_handle, m);
				w = bm_w(bm_handle, m);
				h = bm_h(bm_handle, m);

				if (bm_format(bm_handle) == BITMAP_FORMAT_4444)
				{
					// Do 4444

					if (bm_mipped(bm_handle))
					{
						for (i = 0; i < w * h; i++)
							opengl_Upload_data[i] = /*(255 << 24) |*/ opengl_4444_translate_table[bm_ptr[i]];
					}
					else
					{
						for (i = 0; i < w * h; i++)
							opengl_Upload_data[i] = opengl_4444_translate_table[bm_ptr[i]];
					}
				}
				else
				{
					// Do 1555

					for (i = 0; i < w * h; i++)
						opengl_Upload_data[i] = opengl_Translate_table[bm_ptr[i]];
				}

				if (replace)
					glTexSubImage2D(GL_TEXTURE_2D, m, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, opengl_Upload_data);
				else
					glTexImage2D(GL_TEXTURE_2D, m, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, opengl_Upload_data);

			}
		}
	}

	//mprintf ((1,"Doing slow upload to opengl!\n"));

	if (map_type==MAP_TYPE_LIGHTMAP)
		GameLightmaps[bm_handle].flags&=~LF_LIMITS;
}
