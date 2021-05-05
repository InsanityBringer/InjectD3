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

#include <stdint.h>
#include <gl/GL.h>

#define EXTERN(x,y) extern x y

 // lighting state
typedef enum
{
	LS_NONE,				// no lighting, fully lit rendering
	LS_GOURAUD,			// Gouraud shading
	LS_PHONG,			// Phong shading
	LS_FLAT_GOURAUD	 // Take color from flat color
} light_state;

typedef enum
{
	CM_MONO,		// monochromatic (intensity) model - default
	CM_RGB,			// RGB model
} color_model;

typedef enum
{
	TT_FLAT,					// solid color
	TT_LINEAR,					// textured linearly
	TT_PERSPECTIVE,				// texture perspectively
	TT_LINEAR_SPECIAL,			// A textured polygon drawn as a flat color
	TT_PERSPECTIVE_SPECIAL,			// A textured polygon drawn as a flat color
} texture_type;


typedef enum
{
	RENDERER_SOFTWARE_8BIT,
	RENDERER_SOFTWARE_16BIT,
	RENDERER_OPENGL,
	RENDERER_DIRECT3D,
	RENDERER_GLIDE,
	RENDERER_NONE,
} renderer_type;

typedef enum
{
	WT_WRAP,			// Texture repeats
	WT_CLAMP,		// Texture clamps
	WT_WRAP_V		// Texture wraps in v
} wrap_type;

typedef struct
{
	int8_t initted;

	int8_t cur_bilinear_state;
	int8_t cur_zbuffer_state;
	int8_t cur_fog_state;
	int8_t cur_mip_state;

	texture_type cur_texture_type;
	color_model	cur_color_model;
	light_state cur_light_state;
	int8_t			cur_alpha_type;

	wrap_type	cur_wrap_type;

	float cur_fog_start, cur_fog_end;
	float cur_near_z, cur_far_z;
	float gamma_value;

	int			cur_alpha;
	uint32_t	cur_color;
	uint32_t	cur_fog_color;

	int8_t cur_texture_quality;		// 0-none, 1-linear, 2-perspective

	int clip_x1, clip_x2, clip_y1, clip_y2;
	int screen_width, screen_height;

}	rendering_state;

typedef struct
{
	uint8_t mipping;
	uint8_t filtering;
	float gamma;
	uint8_t bit_depth;
	int width, height;
	uint8_t vsync_on;
} renderer_preferred_state;

typedef struct
{
	int type;
	uint16_t* data;
	int bytes_per_row;
} renderer_lfb;

typedef struct
{
	int poly_count;
	int vert_count;
	int texture_uploads;
}tRendererStats;

int rGL_Init(oeWin32Application* app, renderer_preferred_state* pref_state);
void rGL_Flip();
