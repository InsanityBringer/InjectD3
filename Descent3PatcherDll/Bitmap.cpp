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

#include "Bitmap.h"
#include "GameOffsets.h"

bms_bitmap* GameBitmaps;
bms_lightmap* GameLightmaps;

//Needed because the local bitmap functions may be called on non-resident bitmaps. 
int (*bm_MakeBitmapResident)(int handle);

void InitBitmaps()
{
	GameBitmaps = (bms_bitmap*)GetPatchPoint(PatchPoint::GameBitmaps);
	GameLightmaps = (bms_lightmap*)GetPatchPoint(PatchPoint::GameLightmaps);
	bm_MakeBitmapResident = (int(*)(int))GetPatchPoint(PatchPoint::BitmapMakeResident);
}

// returns a bitmaps width (based on miplevel), else -1 if something is wrong
int bm_w(int handle, int miplevel)
{
	int w;
	if (!GameBitmaps[handle].used)
	{
		return -1;
	}
	if (GameBitmaps[handle].flags & BF_NOT_RESIDENT)
		if (!bm_MakeBitmapResident(handle))
			return 0;

	if (!(GameBitmaps[handle].flags & BF_MIPMAPPED))
		miplevel = 0;
	w = GameBitmaps[handle].width;
	w >>= miplevel;
	return (w);
}

// returns a bitmaps height (based on miplevel), else -1 if something is wrong
int bm_h(int handle, int miplevel)
{
	int h;
	if (!GameBitmaps[handle].used)
	{
		return -1;
	}
	// If this bitmap is not page in, do so!
	if (GameBitmaps[handle].flags & BF_NOT_RESIDENT)
		if (!bm_MakeBitmapResident(handle))
			return 0;

	if (!(GameBitmaps[handle].flags & BF_MIPMAPPED))
		miplevel = 0;
	h = GameBitmaps[handle].height;
	h >>= miplevel;
	return (h);
}

// returns a bitmaps mipped status, else -1 if something is wrong
int bm_mipped(int handle)
{
	if (!GameBitmaps[handle].used)
	{
		return -1;
	}
	// If this bitmap is not page in, do so!
	if (GameBitmaps[handle].flags & BF_NOT_RESIDENT)
		if (!bm_MakeBitmapResident(handle))
			return 0;

	if (GameBitmaps[handle].flags & BF_MIPMAPPED)
		return  1;
	return 0;
}

// Returns the format of this bitmap
int bm_format(int handle)
{
	if (!GameBitmaps[handle].used)
	{
		return -1;
	}
	if (GameBitmaps[handle].flags & BF_NOT_RESIDENT)
		if (!bm_MakeBitmapResident(handle))
			return 0;

	return GameBitmaps[handle].format;
}

// returns a bitmaps data (based on given miplevel), else NULL if something is wrong
unsigned short* bm_data(int handle, int miplevel)
{
	unsigned short* d;
	int i;
	if (!GameBitmaps[handle].used)
	{
		return NULL;
	}

	// If this bitmap is not page in, do so!
	if (GameBitmaps[handle].flags & BF_NOT_RESIDENT)
		if (!bm_MakeBitmapResident(handle))
			return NULL;

	d = GameBitmaps[handle].data16;
	for (i = 0; i < miplevel; i++) {
		d += (GameBitmaps[handle].width >> i) * (GameBitmaps[handle].height >> i);
	}
	return (d);
}

// returns a lightmaps width  else -1 if something is wrong
int lm_w(int handle)
{
	int w;
	if (!GameLightmaps[handle].used)
	{
		return -1;
	}
	w = GameLightmaps[handle].width;

	return (w);
}

// returns a lightmaps height , else -1 if something is wrong
int lm_h(int handle)
{
	int h;
	if (!GameLightmaps[handle].used)
	{
		return -1;
	}
	h = GameLightmaps[handle].height;

	return (h);
}

// returns a lightmaps data else NULL if something is wrong
unsigned short* lm_data(int handle)
{
	if (!GameLightmaps[handle].used)
	{
		return NULL;
	}

	return GameLightmaps[handle].data;
}
