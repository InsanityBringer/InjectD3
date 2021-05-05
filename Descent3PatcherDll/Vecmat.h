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

//Angles are unsigned shorts
typedef unsigned short angle;	//make sure this matches up with fix.h


typedef struct
{
	angle p, h, b;
} angvec;

#define IDENTITY_MATRIX		{{1.0,0,0},{0,1.0,0},{0,0,1.0}}

typedef struct
{
	float x, y, z;
} vector;

typedef struct vector4
{
	float x, y, z, kat_pad;
} vector4;


typedef struct
{
	float xyz[3];
} vector_array;

#define IDENTITY_MATRIX		{{1.0,0,0},{0,1.0,0},{0,0,1.0}}

typedef struct
{
	vector rvec, uvec, fvec;
} matrix;

typedef struct
{
	vector4 rvec, uvec, fvec;
} matrix4;

// Zero's out a vector
inline void vm_MakeZero(vector* v)
{
	v->x = v->y = v->z = 0;
}

// Set an angvec to {0,0,0}
inline void vm_MakeZero(angvec* a)
{
	a->p = a->h = a->b = 0;
}
