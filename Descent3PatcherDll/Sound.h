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
#include "D3Defs.h"
#include "Win32App.h"
#include "Vecmat.h"

#define MAX_GAME_VOLUME (float) 1.0 //helps against clipping

 // Size of streaming buffers
#define STREAM_BUFFER_SIZE 4096	//(4 * 1024)

//object information needed by the sound code.  Calling code should copy data into here & pass to sound
typedef struct
{
	vector* position;						// Where in the world is this object
	matrix* orient;
	vector* velocity;
	int roomnum;
} pos_state;

typedef struct
{

	void* (*m_stream_cback)(void* user_data, int handle, int* size);  // Streaming callback
	void* m_stream_data;					// passed in
	int			m_stream_size;					// passed in
	int			m_stream_handle;				// passed in
	int			m_stream_bufsize;				// passed in
	void* user_data;						// this is passed to the stream callback by the caller that defined this.

	uint8_t		sample_skip_interval;		// Allows us to skip samples (i.e. simulate lower sampling rates)
	uint8_t		priority;						// priority of sound.
	uint16_t		m_stream_format;				// passed in

//	internal data.
	int		m_samples_played;
		
	float		samples_per_22khz_sample;	// passed in
	float		left_volume;
	float		right_volume;

	float		m_ticks; // Always incrementing counter (current sample position if no looping)
} play_information;

typedef struct sound_file_info
{
	char				name[PAGENAME_LEN];
	char				used;
	int				use_count;				// how many buffers does this sound take up.

	unsigned char* sample_8bit;			// 8bit sound data
	short* sample_16bit;			// 16bit sound data

	int				sample_length;			// Length of sound in samples
	int				np_sample_length;		// non-padded

} sound_file_info;

typedef struct sound_info
{
	char				name[PAGENAME_LEN];
	char				used;

	int				sample_index;

	int				loop_start;			// Start byte of repeated loop for looping samples
	int				loop_end;			// End byte of repeating loop for looping samples
	unsigned int	flags;				// 2d/3d, variable frequency
	float				max_distance;		// Maximum distance in which a sound is heard
	float				min_distance;		// Sound gets no louder at min_distance
	int				inner_cone_angle;	// Angle in which sound is played at full base volume
	int	         outer_cone_angle;	// Angle in which sound is at its lowest base volume
	float				outer_cone_volume;// A sounds lowest base volume level
	float				import_volume;		// Volume multiplier
} sound_info;

// Supported sound mixers
#define SOUND_MIXER_SOFTWARE_16	0
#define SOUND_MIXER_DS_16			1
#define SOUND_MIXER_DS_8			2
#define SOUND_MIXER_DS3D_16		3
#define SOUND_MIXER_AUREAL			4
#define SOUND_MIXER_CREATIVE_EAX	6	// switched because launcher uses 5 as NONE.
#define SOUND_MIXER_NONE			5

#define SQT_NORMAL						0
#define SQT_HIGH						1

// Parameters of the sound library
#define SLF_USE_3D			1		// Use 3d effects
#define SLF_DELTA_FREQ		2		// Use frequency shifts (i.e. water effects)
#define SLF_USE_16_BIT		4		// Use 16bit samples (else 8bit)
#define SLF_USE_22_KHZ		8		// Use 22khz (else 44khz)
#define SLF_PAUSED			16		// Sound library is currently paused
#define SLF_FULL_3D			32		// Full 3d hardware support
#define SLF_MOST_3D			64		// No fully static 3d -- i.e. cockpit type stuff (use 2d instead)
#define SLF_LIGHT_3D			128	// Dynamically updating 3d sounds if sound is longer than a given threshold
#define SLF_GOOD_2D			256	// all linked sounds update position
#define SLF_OK_2D				512	// if a sound is longer than a threshold, it updates

// Sound Properties Flags
#define SPF_LOOPED				1		// Sound is looped
#define SPF_FIXED_FREQ			2		// No doppler shift
#define SPF_OBJ_UPDATE			4		// Sound updates with attached object movements
#define SPF_FOREVER				8		// Always plays in high-level, this flag should be ignored in low-level
#define SPF_PLAYS_EXCLUSIVELY 16
#define SPF_PLAYS_ONCE			32
#define SPF_USE_CONE				64
#define SPF_LISTENER_UPDATE	128	// Sound updates with listener movements 
#define SPF_ONCE_PER_OBJ		256

// Sound Instance flags (Move this out of here)
#define SIF_UNUSED				0		// Not a valid sound item
#define SIF_PLAYING_2D			1		// Sound is currently playing
#define SIF_PLAYING_3D			2
#define SIF_OBJ_UPDATE			4
#define SIF_TOO_FAR				8		// We will play it, but it currently too far away(stop sound in low-level)		
#define SIF_NO_3D_EFFECTS		16
#define SIF_LOOPING           32
#define SIF_STREAMING_8_M		64
#define SIF_STREAMING_16_M		128
#define SIF_STREAMING_8_S		256
#define SIF_STREAMING_16_S    512
#define SIF_STREAMING         (64 | 128 | 256 | 512)

// What is the sound cone linked to (and mask to make it else to look at the important bits)
#define SPFT_CONE_LINK_MASK		0x00000300
#define SPFT_CONE_LINK_OBJECT		0x00000000
#define SPFT_CONE_LINK_TURRET1	0x00000100
#define SPFT_CONE_LINK_TURRET2   0x00000200
#define SPFT_CONE_LINK_TURRET3   0x00000300

// Direction of the sound cone relative to its link (and mask to make it else to look at the important bits)
#define SPFT_CONE_DIR_MASK			0x00000C00
#define SPFT_CONE_DIR_FORWARD    0x00000000
#define SPFT_CONE_DIR_BACKWARD   0x00000400
#define SPFT_CONE_DIR_UPWARD     0x00000800
#define SPFT_CONE_DIR_DOWNWARD   0x00000C00

// Sound kill types
#define SKT_STOP_AFTER_LOOP	0		// Allows a looping sample to play until the end of the sample
#define SKT_STOP_IMMEDIATELY	1		// Stops and cleans up after a sound (For StopAllSounds)
#define SKT_HOLD_UNTIL_STOP	2		// Hold until sound stops.

// Sound Library Internal Error Codes
#define SSL_OK								0

// structure to get and set environment values
#define ENV3DVALF_DOPPLER		1
#define ENV3DVALF_GEOMETRY		2

typedef struct t3dEnvironmentValues
{
	int flags;								// use flags above
	float doppler_scalar;				// values from 0.0f to ???? (1.0f = normal)
}
t3dEnvironmentValues;

typedef struct t3dEnvironmentToggles
{
	int flags;								// use flags above
	int supported;							// returns flag values to inform caller of supported features (doppler, ie.)

	bool doppler;							// state of doppler effects
	bool geometry;							// support hardware geometry
}
t3dEnvironmentToggles;

/* So, C++ is actually a sane language and doesn't let you take function pointers of constructors, and limits typecasts of function
* pointers of class methods. That's all good and all for the purposes of sane programs, but this isn't a sane program, so I'm doing
* something much, muuuuch dumber...*/


//Abstract LLS system class
class llsSystem
{
protected:
	void* m_geometry;			// geometry object.
	int m_lib_error_code;				// library error code 
	char m_error_text[512];				// text for error.

public:
	void SetError(int code) { m_lib_error_code = code; };
	void ErrorText(char* fmt, ...);	// error text function called inside library.  a stack is kept of errors
	virtual void CheckForErrors();	// called by sound library every frame to reset error count.

// Public functions 

	llsSystem();

	// may be called before init (must be to be valid, the card passed here will be initialized in InitSoundLib)
	virtual void SetSoundCard(const char* name) = 0;

	// Starts the sound library, maybe have it send back some information -- 3d support?
	virtual int InitSoundLib(char mixer_type, oeWin32Application* sos, unsigned char max_sounds_played) = 0;
	// Cleans up after the Sound Library
	virtual void DestroySoundLib(void) = 0;

	// Locks and unlocks sounds (used when changing play_info data)
	virtual bool LockSound(int sound_uid) = 0;
	virtual bool UnlockSound(int sound_uid) = 0;

	virtual bool SetSoundQuality(char quality) = 0;
	virtual char GetSoundQuality(void) = 0;
	virtual bool SetSoundMixer(char mixer_type) = 0;
	virtual char GetSoundMixer(void) = 0;

	// Plays a 2d sound
	virtual int PlaySound2d(play_information* play_info, int sound_index, float volume, float pan, bool f_looped) = 0;
	virtual int PlayStream(play_information* play_info) = 0;

	virtual void SetListener(pos_state* cur_pos) = 0;
	virtual int PlaySound3d(play_information* play_info, int sound_index, pos_state* cur_pos, float master_volume, bool f_looped, float reverb = 0.5f) = 0; //, unsigned short frequency)
	virtual void AdjustSound(int sound_uid, float f_volume, float f_pan, unsigned short frequency) = 0;
	virtual void AdjustSound(int sound_uid, pos_state* cur_pos, float adjusted_volume, float reverb = 0.5f) = 0;

	virtual void StopAllSounds(void) = 0;

	// Checks if a sound is playing (removes finished sound);
	virtual bool IsSoundInstancePlaying(int sound_uid) = 0;
	virtual int IsSoundPlaying(int sound_index) = 0;

	//	virtual void AdjustSound(int sound_uid, play_information *play_info) = 0;

	// Stops 2d and 3d sounds
	virtual void StopSound(int sound_uid, unsigned char f_immediately = SKT_STOP_IMMEDIATELY) = 0;

	// Pause all sounds/resume all sounds
	virtual void PauseSounds(void) = 0;
	virtual void ResumeSounds(void) = 0;
	virtual void PauseSound(int sound_uid) = 0;
	virtual void ResumeSound(int sound_uid) = 0;

	virtual bool CheckAndForceSoundDataAlloc(int sound_file_index) = 0;

	// Begin sound frame
	virtual void SoundStartFrame(void) = 0;

	// End sound frame 
	virtual void SoundEndFrame(void) = 0;

	// Returns current error code
	int GetLastError() 
	{ 
		int code = m_lib_error_code; m_lib_error_code = 0; return code; 
	};

	// environmental sound interface
	// volume modifier (0-1), damping(0-1), 1 = complete, 0 = none
	//	decay 0.1 to 100 seconds, how long it takes for a sound to die.
	virtual bool SetGlobalReverbProperties(float volume, float damping, float decay) = 0;

	// set special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	virtual void SetEnvironmentValues(const t3dEnvironmentValues* env) = 0;

	// get special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	virtual void GetEnvironmentValues(t3dEnvironmentValues* env) = 0;

	// enable special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	virtual void SetEnvironmentToggles(const t3dEnvironmentToggles* env) = 0;

	// get states of special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	virtual void GetEnvironmentToggles(t3dEnvironmentToggles* env) = 0;

	// returns interface to sound geometry manipulation if available.
	virtual void* GetGeometryInterface() 
	{
		return m_geometry;
	};

	/////////////////////////////////////////////////////////////////////////////////
		// set auxillary 3d sound properties
	virtual bool SoundPropertySupport() const { return false; };

	// sound obstruction from 0 to 1.0 (1.0  = fully obstructed)
	virtual void SetSoundProperties(int sound_uid, float obstruction) {};
};

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

//This is the amount of buffers that will be queued in OpenAL for a streaming audio source. This needs to be high enough to avoid starvation,
//but not too high to add noticable latency to stream changes. 
#define NUM_STREAMING_BUFFERS 3

//Taken from streamaudio.h
//	stream format
#define SAF_8BIT_MASK		0x00
#define SAF_16BIT_MASK		0x01
#define SAF_MONO_MASK		0x00
#define SAF_STEREO_MASK		0x10
	
#define SAF_8BIT_M			(SAF_8BIT_MASK | SAF_MONO_MASK)		//0x00
#define SAF_8BIT_S			(SAF_8BIT_MASK | SAF_STEREO_MASK)	//0x10
#define SAF_16BIT_M			(SAF_16BIT_MASK | SAF_MONO_MASK)		//0x01
#define SAF_16BIT_S			(SAF_16BIT_MASK | SAF_STEREO_MASK)	//0x11

//OpenAL LLS system implementation
struct llsOpenALSoundEntry
{
	uint32_t handle, bufferHandle;
	int soundNum;
	int soundUID;
	float volume;
	//this can be figured out by asking OpenAL, but this is effectively a race condition since the mixer is running on another thread. 
	//This will be updated in EndSoundFrame, which is still a race condition but I don't want it happening on every call to IsSoundPlaying
	bool playing;
	//True if the source is streaming, since streaming sources need to be kept fed and also closed differently.
	bool streaming;
	play_information *info;

	//Streaming buffer handles. These are the actual handles that will be queued.
	ALuint bufferHandles[NUM_STREAMING_BUFFERS];
	//The state of each buffer, true if the buffer is currently queued and shouldn't be reused. 
	bool bufferStatus[NUM_STREAMING_BUFFERS];

	//Buffer queue, list of handles currently in use.
	ALuint bufferQueue[NUM_STREAMING_BUFFERS];
	uint32_t streamFormat;

	t3dEnvironmentValues envValues;
};

class llsOpenAL : public llsSystem
{
	vector ListenerPosition;
	vector ListenerVelocty;
	matrix ListenerOrient;
	int ListenerRoomNum;

	bool Initalized;
	bool LoopPointsSupported, EffectsSupported;
	char Quality;

	int NumSoundChannels;
	int NumSoundsPlaying;
	int NextUID;
	llsOpenALSoundEntry* SoundEntries;

	//OpenAL state
	ALCcontext* Context;
	ALCdevice* Device;

	ALuint AuxEffectSlot, EffectSlot;

	float Volume, Damping, Decay;

	bool ALErrorCheck(const char* context);

	short FindSoundSlot(float volume, int priority);

	void InitSource2D(uint32_t handle, sound_info *soundInfo, float volume);
	void InitSourceStreaming(uint32_t handle, float volume);
	void InitSource3D(uint32_t handle, sound_info* soundInfo, pos_state* posInfo, float volume);
	void BindBufferData(uint32_t handle, int sound_index, bool looped);
	void SoundCleanup(int soundID);

	void ServiceStream(int soundID);

public:
	llsOpenAL()
	{
		Initalized = false;
		LoopPointsSupported = EffectsSupported = false;
		Quality = SQT_HIGH;
		NumSoundChannels = 0;
		NumSoundsPlaying = 0;
		NextUID = 0;
		SoundEntries = nullptr;

		vm_MakeZero(&ListenerPosition);
		vm_MakeZero(&ListenerVelocty);
		ListenerOrient = IDENTITY_MATRIX;
		ListenerRoomNum = 0;

		Context = nullptr; Device = nullptr;

		AuxEffectSlot = EffectSlot = 0;
		Volume = Damping = Decay = 0;
	}
	// may be called before init (must be to be valid, the card passed here will be initialized in InitSoundLib)
	void SetSoundCard(const char* name) override;

	// Starts the sound library, maybe have it send back some information -- 3d support?
	int InitSoundLib(char mixer_type, oeWin32Application* sos, unsigned char max_sounds_played) override;
	// Cleans up after the Sound Library
	void DestroySoundLib(void) override;

	// Locks and unlocks sounds (used when changing play_info data)
	bool LockSound(int sound_uid) override;
	bool UnlockSound(int sound_uid) override;

	bool SetSoundQuality(char quality) override;
	char GetSoundQuality(void) override;
	bool SetSoundMixer(char mixer_type) override;
	char GetSoundMixer(void) override;

	// Plays a 2d sound
	int PlaySound2d(play_information* play_info, int sound_index, float volume, float pan, bool f_looped) override;
	int PlayStream(play_information* play_info) override;

	void SetListener(pos_state* cur_pos) override;
	int PlaySound3d(play_information* play_info, int sound_index, pos_state* cur_pos, float master_volume, bool f_looped, float reverb = 0.5f) override;
	void AdjustSound(int sound_uid, float f_volume, float f_pan, unsigned short frequency) override;
	void AdjustSound(int sound_uid, pos_state* cur_pos, float adjusted_volume, float reverb = 0.5f) override;

	void StopAllSounds(void) override;

	// Checks if a sound is playing (removes finished sound);
	bool IsSoundInstancePlaying(int sound_uid) override;
	int IsSoundPlaying(int sound_index) override;

	//	virtual void AdjustSound(int sound_uid, play_information *play_info) = 0;

	// Stops 2d and 3d sounds
	void StopSound(int sound_uid, unsigned char f_immediately = SKT_STOP_IMMEDIATELY) override;

	// Pause all sounds/resume all sounds
	void PauseSounds(void) override;
	void ResumeSounds(void) override;
	void PauseSound(int sound_uid) override;
	void ResumeSound(int sound_uid) override;

	bool CheckAndForceSoundDataAlloc(int sound_file_index) override;

	// Begin sound frame
	void SoundStartFrame(void) override;

	// End sound frame 
	void SoundEndFrame(void) override;

	// environmental sound interface
	// volume modifier (0-1), damping(0-1), 1 = complete, 0 = none
	//	decay 0.1 to 100 seconds, how long it takes for a sound to die.
	bool SetGlobalReverbProperties(float volume, float damping, float decay) override;

	// set special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	void SetEnvironmentValues(const t3dEnvironmentValues* env) override;

	// get special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	void GetEnvironmentValues(t3dEnvironmentValues* env) override;

	// enable special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	void SetEnvironmentToggles(const t3dEnvironmentToggles* env) override;

	// get states of special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	void GetEnvironmentToggles(t3dEnvironmentToggles* env) override;

	//Hack level: whyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy
	llsOpenAL* llsOpenAL_PatchConstructor();
};
