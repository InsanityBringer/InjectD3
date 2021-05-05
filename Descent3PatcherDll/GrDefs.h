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

#define OPAQUE_FLAG16	0x8000
#define TRANSPARENT_COLOR32 0x0000FF00
#define NEW_TRANSPARENT_COLOR	0x0000
#define OPAQUE_FLAG		OPAQUE_FLAG16

inline uint32_t GR_RGB(int r, int g, int b)
{
	return ((r << 16) + (g << 8) + b);
}

inline uint16_t GR_RGB16(int r, int g, int b)
{
	return (((r >> 3) << 10) + ((g >> 3) << 5) + (b >> 3));
}

inline uint16_t GR_COLOR_TO_16(uint32_t c)
{
	int r, g, b;
	r = ((c & 0x00ff0000) >> 16);
	g = ((c & 0x0000ff00) >> 8);
	b = (c & 0x000000ff);

	return (uint16_t)(((r >> 3) << 10) + ((g >> 3) << 5) + (b >> 3));
}

inline int GR_COLOR_RED(uint32_t c)
{
	int r = ((c & 0x00ff0000) >> 16);
	return (int)r;
}

inline int GR_COLOR_GREEN(uint32_t c)
{
	int g = ((c & 0x0000ff00) >> 8);
	return (int)g;
}

inline int GR_COLOR_BLUE(uint32_t c)
{
	int b = (c & 0x000000ff);
	return (int)b;
}

inline uint32_t GR_16_TO_COLOR(uint16_t col)
{
	int r, g, b;

	r = (col & 0x7c00) >> 7;
	g = (col & 0x03e0) >> 2;
	b = (col & 0x001f) << 3;

	return GR_RGB(r, g, b);
}
