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

#pragma once

#define MAX_BITMAPS 5000
#ifndef MACINTOSH	//DAJ do this in opengl/glide headers
#define NUM_MIP_LEVELS 5
#endif
// It really doesn't matter what these are, as long as its above 10
#define OUTRAGE_4444_COMPRESSED_MIPPED	121
#define OUTRAGE_1555_COMPRESSED_MIPPED	122
#define OUTRAGE_NEW_COMPRESSED_MIPPED	123
#define OUTRAGE_COMPRESSED_MIPPED		124
#define OUTRAGE_COMPRESSED_OGF_8BIT		125
#define OUTRAGE_TGA_TYPE					126
#define OUTRAGE_COMPRESSED_OGF			127
#define BITMAP_NAME_LEN			35
#define BAD_BITMAP_HANDLE	0
// Bitmap flags
#define BF_TRANSPARENT			1
#define BF_CHANGED				2			// this bitmap has changed since last frame (useful for hardware cacheing)
#define BF_MIPMAPPED			4			// This bitmap has mip levels
#define BF_NOT_RESIDENT			8			// This bitmap is not paged in
#define BF_WANTS_MIP			16			// Calculate mip levels when this bitmap is paged in
#define BF_WANTS_4444			32			// Read data as 4444 when this bitmap is paged in
#define BF_BRAND_NEW			64			// This bitmap was just allocated and hasn't been to the video card
#define BF_COMPRESSABLE			128			// This bitmap is compressable for 3dhardware that supports it
// Bitmap priorities
#define BITMAP_FORMAT_STANDARD	0
#define BITMAP_FORMAT_1555			0
#define BITMAP_FORMAT_4444			1
typedef struct
{
	unsigned short* data16;					// 16bit data
	unsigned short width, height;			// Width and height in pixels
	unsigned short used;					// Is this bitmap free to be allocated?

	short cache_slot;					// For use by the rendering lib
	unsigned char mip_levels;
	unsigned char flags;

	unsigned char format;						// See bitmap format types above
	char name[BITMAP_NAME_LEN];	// Whats the name of this bitmap? (ie SteelWall)	
} bms_bitmap;

typedef struct chunked_bitmap
{
	int pw, ph;							// pixel width and height
	int w, h;							// width and height in square bitmaps.
	int* bm_array;						// array of bitmap handles.
} chunked_bitmap;

//Variables
extern bms_bitmap* GameBitmaps;

//Functions
//These work off the imported GameBitmaps pointer, but are local to the dll. 

//Called at startup to grab pointers for the bitmap and lightmap arrays. 
void InitBitmaps();

// given a handle to a bitmap, returns its width, or -1 if handle is invalid
int bm_w(int handle, int miplevel);

// given a handle to a bitmap, returns its height, or -1 if handle is invalid
int bm_h(int handle, int miplevel);

// given a handle to a bitmap, returns mipped status, or -1 if handle is invalid
int bm_mipped(int handle);

// Returns the format of this bitmap
int bm_format(int handle);

// given a handle to a bitmap, returns a pointer to its data, or NULL if handle is invalid
unsigned short* bm_data(int handle, int miplevel);

//Lightmaps
#define MAX_LIGHTMAPS (65534)
#define BAD_LM_INDEX	65535

// lightmap flags
#define LF_CHANGED				1			// this bitmap has changed since last frame (useful for hardware cacheing)
#define LF_LIMITS					2			// This lightmap has a specific area that has changed since last frame
#define LF_WRAP					4			// This lightmap should be drawn with wrapping (not clamping)
#define LF_BRAND_NEW				8			// This lightmap is brand new and hasn't been to the video card yet

typedef struct
{
	unsigned char width, height;			// Width and height in pixels
	unsigned short* data;					// 16bit data

	unsigned short used;
	unsigned char flags;
	short	cache_slot;				// for the renderers use
	unsigned char square_res;				// for renderers use
	unsigned char cx1, cy1, cx2, cy2;		// Change x and y coords 
} bms_lightmap;

//Variables
extern bms_lightmap* GameLightmaps;

//Functions

// returns a lightmaps width  else -1 if something is wrong
int lm_w(int handle);

// returns a lightmaps height , else -1 if something is wrong
int lm_h(int handle);

// returns a lightmaps data else NULL if something is wrong
unsigned short* lm_data(int handle);
