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

#include <stdlib.h>
#include <algorithm>

#include "Configuration.h"
#include "D3Defs.h"
#include "Sound.h"
#include "GameOffsets.h"

//Configuration
bool ConfigUseReverbs = false;

//Pointers to original data:
sound_info* pSounds;
sound_file_info* pSoundFiles;

LPALGENAUXILIARYEFFECTSLOTS dalGenAuxiliaryEffectSlots;
LPALGENEFFECTS dalGenEffects;
LPALDELETEEFFECTS dalDeleteEffects;
LPALDELETEAUXILIARYEFFECTSLOTS dalDeleteAuxiliaryEffectSlots;
LPALEFFECTI dalEffecti;
LPALEFFECTF dalEffectf;
LPALAUXILIARYEFFECTSLOTI dalAuxiliaryEffectSloti;

char (*SoundLoadWaveFile)(char* filename, float percent_volume, int sound_file_index, bool f_high_quality, bool f_load_sample_data, int* e_type);

llsSystem::llsSystem()
{
	m_geometry = nullptr;
	m_lib_error_code = 0;
	memset(m_error_text, 0, sizeof(m_error_text));
}

void llsSystem::ErrorText(char* fmt, ...)
{
}

void llsSystem::CheckForErrors()
{
}

void llsSystem__ErrorText(char* fmt, ...)
{
}

//llsOpenAL implementation
void llsOpenAL::SetSoundCard(const char* name)
{
}

int llsOpenAL::InitSoundLib(char mixer_type, oeWin32Application* sos, unsigned char max_sounds_played)
{
	int i, numsends;
	ALCint attribs[4] = {};
	//MessageBoxA(nullptr, "Horrible hack has been started up", "Cursed", MB_OK);
	Device = alcOpenDevice(nullptr);
	//ALErrorCheck("Opening device");
	if (!Device)
	{
		PutLog(LogLevel::Error, "OpenAL LLS failed to open a device.");
		return 0;
	}

	EffectsSupported = alcIsExtensionPresent(nullptr, "ALC_EXT_EFX") != AL_FALSE;
	if (!EffectsSupported)
		PutLog(LogLevel::Warning, "OpenAL effects extension not present!");
	else
	{
		if (ConfigUseReverbs)
		{
			EffectsSupported = false;
		}
		else
		{
			//Just one environment available, so only one send needed
			attribs[0] = ALC_MAX_AUXILIARY_SENDS;
			attribs[1] = 1;
		}
	}

	Context = alcCreateContext(Device, attribs);
	//ALErrorCheck("Creating context");
	if (!Context)
	{
		alcCloseDevice(Device);
		PutLog(LogLevel::Error, "OpenAL LLS failed to create a context.");
		return 0;
	}
	alcMakeContextCurrent(Context);

	LoopPointsSupported = alIsExtensionPresent("AL_SOFT_loop_points") != AL_FALSE;
	if (!LoopPointsSupported)
		PutLog(LogLevel::Warning, "OpenAL Soft loop points extension not present!");

	//Check if that one send is available, though there's no reason to assume it won't...
	alcGetIntegerv(Device, ALC_MAX_AUXILIARY_SENDS, 1, &numsends);

	if (numsends < 1)
	{
		PutLog(LogLevel::Warning, "OpenAL effects extension failed to provide any sends?");
		EffectsSupported = false;
	}

	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

	NumSoundChannels = max_sounds_played;
	SoundEntries = (llsOpenALSoundEntry*)malloc(sizeof(llsOpenALSoundEntry) * NumSoundChannels);

	//Create sources for all entries now
	for (i = 0; i < NumSoundChannels; i++)
	{
		alGenSources(1, &SoundEntries[i].handle);
	}
	ALErrorCheck("Creating default sources");
	for (i = 0; i < NumSoundChannels; i++)
	{
		alGenBuffers(1, &SoundEntries[i].bufferHandle);
	}
	ALErrorCheck("Creating default buffers");

	if (EffectsSupported)
	{
		//Load and validate function pointers
		dalGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
		dalDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
		dalGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
		dalDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
		dalEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
		dalEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
		dalAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");

		if (!dalGenAuxiliaryEffectSlots || !dalDeleteAuxiliaryEffectSlots || !dalGenEffects || !dalDeleteEffects || !dalEffecti || !dalEffectf || !dalAuxiliaryEffectSloti)
		{
			PutLog(LogLevel::Warning, "Failed to get OpenAL effects extension function pointers.");
			EffectsSupported = false;
		}
		else
		{
			//Generate the effect and aux effect slot
			//Just one since there's only a single global environment.
			dalGenEffects(1, &EffectSlot);
			ALErrorCheck("Creating effect");
			dalGenAuxiliaryEffectSlots(1, &AuxEffectSlot);
			ALErrorCheck("Creating aux effect");

			//Make the effect an EAX reverb
			dalEffecti(EffectSlot, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
			ALErrorCheck("Setting effect type");
			SetGlobalReverbProperties(0.5, 0.3, 10.0); //These appear to be the default values Descent 3 uses.

			//Make the aux effect slot use the effect
			dalAuxiliaryEffectSloti(AuxEffectSlot, AL_EFFECTSLOT_EFFECT, EffectSlot);
			ALErrorCheck("Setting aux effect slot");
		}
	}

	PutLog(LogLevel::Info, "OpenAL LLS started successfully. %d channels specified.", max_sounds_played);
	Initalized = true;
	
	return 1;
}

void llsOpenAL::DestroySoundLib(void)
{
	//MessageBoxA(nullptr, "Horrible hack has been shutdown", "Cursed", MB_OK);
	if (Initalized)
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(Context);
		alcCloseDevice(Device);

		Context = nullptr; Device = nullptr;

		if (SoundEntries)
			free(SoundEntries);

		PutLog(LogLevel::Info, "OpenAL LLS shut down.");
		Initalized = false;
	}
}

bool llsOpenAL::LockSound(int sound_uid)
{
	return false;
}

bool llsOpenAL::UnlockSound(int sound_uid)
{
	return false;
}

bool llsOpenAL::SetSoundQuality(char quality)
{
	PutLog(LogLevel::Info, "LLS Quality set to %d", quality);
	if (Quality == quality) return false;
	Quality = SQT_HIGH;
	return true;
}

char llsOpenAL::GetSoundQuality(void)
{
	return Quality;
}

bool llsOpenAL::SetSoundMixer(char mixer_type)
{
	return false;
}

char llsOpenAL::GetSoundMixer(void)
{
	return SOUND_MIXER_DS3D_16;
}

int llsOpenAL::PlaySound2d(play_information* play_info, int sound_index, float volume, float pan, bool f_looped)
{
	if (!Initalized) return -1;
	if (pSoundFiles[pSounds[sound_index].sample_index].used == 0) return -1;
	short sound_uid = FindSoundSlot(volume, play_info->priority);
	if (sound_uid < 0) return -1;

	bool looped = f_looped || (pSounds[sound_index].flags & SPF_LOOPED) != 0;

	//PutLog(LogLevel::Info, "Initializing 2D source.");
	InitSource2D(SoundEntries[sound_uid].handle, &pSounds[sound_index], volume);
	if (looped)
		alSourcei(SoundEntries[sound_uid].handle, AL_LOOPING, AL_TRUE);
	if (ALErrorCheck("Setting 2D sound source properties.")) return -1;

	play_info->left_volume = play_info->right_volume = volume;

	//Unbind the buffer so I can rewrite it:
	//TODO: Should each entry in pSound have their own buffer instead?
	alSourcei(SoundEntries[sound_uid].handle, AL_BUFFER, 0);
	//PutLog(LogLevel::Info, "Setting 2D source data.");
	BindBufferData(SoundEntries[sound_uid].bufferHandle, sound_index, looped);
	if (ALErrorCheck("Binding 2D sound source buffer")) return -1;
	alSourcei(SoundEntries[sound_uid].handle, AL_BUFFER, SoundEntries[sound_uid].bufferHandle);
	if (ALErrorCheck("Setting 2D sound source buffer")) return -1;

	//PutLog(LogLevel::Info, "Starting 2D source.");
	alSourcePlay(SoundEntries[sound_uid].handle);
	ALErrorCheck("Starting 2D source");
	NextUID++;
	SoundEntries[sound_uid].playing = true;
	SoundEntries[sound_uid].streaming = false;
	SoundEntries[sound_uid].volume = volume;
	SoundEntries[sound_uid].info = play_info;
	SoundEntries[sound_uid].soundNum = sound_index;
	SoundEntries[sound_uid].soundUID = NextUID * 256 + sound_uid;
	//PutLog(LogLevel::Info, "Starting 2D sound %s with uid %d (slot %d)", pSoundFiles[pSounds[sound_index].sample_index].name, SoundEntries[sound_uid].soundUID, sound_uid);

	NumSoundsPlaying++;
	return SoundEntries[sound_uid].soundUID;
}

int llsOpenAL::PlayStream(play_information* play_info)
{
	float peakVolume = max(play_info->left_volume, play_info->right_volume);
	if (!Initalized) return -1;
	short sound_uid = FindSoundSlot(peakVolume, play_info->priority);
	if (sound_uid < 0) return -1;

	//PutLog(LogLevel::Info, "Starting a stream");
	InitSourceStreaming(SoundEntries[sound_uid].handle, peakVolume);

	//Generate buffers
	alGenBuffers(4, SoundEntries[sound_uid].bufferHandles);
	memset(SoundEntries[sound_uid].bufferStatus, 0, sizeof(SoundEntries[sound_uid].bufferStatus)); //all ready to use
	alSourcei(SoundEntries[sound_uid].handle, AL_BUFFER, 0); //ensure no buffer is bound

	SoundEntries[sound_uid].streamFormat = -1;

	if (play_info->m_stream_format & SIF_STREAMING_8_M)
		SoundEntries[sound_uid].streamFormat = AL_FORMAT_MONO8;
	else if (play_info->m_stream_format & SIF_STREAMING_16_M)
		SoundEntries[sound_uid].streamFormat = AL_FORMAT_MONO16;
	else if (play_info->m_stream_format & SIF_STREAMING_8_S)
		SoundEntries[sound_uid].streamFormat = AL_FORMAT_STEREO8;
	else if (play_info->m_stream_format & SIF_STREAMING_16_S)
		SoundEntries[sound_uid].streamFormat = AL_FORMAT_STEREO16;

	if (SoundEntries[sound_uid].streamFormat == -1) return -1;

	//PutLog(LogLevel::Info, "Queueing initial data");
	alBufferData(SoundEntries[sound_uid].bufferHandles[0], SoundEntries[sound_uid].streamFormat, play_info->m_stream_data, play_info->m_stream_bufsize, 22050);
	ALErrorCheck("Creating initial stream buffer");
	alSourceQueueBuffers(SoundEntries[sound_uid].handle, 1, &SoundEntries[sound_uid].bufferHandles[0]);
	SoundEntries[sound_uid].bufferQueue[0] = 0;
	SoundEntries[sound_uid].bufferStatus[0] = true;
	ALErrorCheck("Queueing initial streaming buffer");

	//PutLog(LogLevel::Info, "Starting");
	alSourcePlay(SoundEntries[sound_uid].handle);
	ALErrorCheck("Starting streaming source");

	NextUID++;
	SoundEntries[sound_uid].playing = true;
	SoundEntries[sound_uid].streaming = true;
	SoundEntries[sound_uid].volume = peakVolume;
	SoundEntries[sound_uid].info = play_info;
	SoundEntries[sound_uid].soundUID = NextUID * 256 + sound_uid;

	NumSoundsPlaying++;
	//PutLog(LogLevel::Info, "Starting stream with uid %d (slot %d). Format is %d. Buffer size is %d. Stream size is %d.", SoundEntries[sound_uid].soundUID, sound_uid, play_info->m_stream_format, play_info->m_stream_bufsize, play_info->m_stream_size);

	return SoundEntries[sound_uid].soundUID;
}

void llsOpenAL::SetListener(pos_state* cur_pos)
{
	//PutLog(LogLevel::Info, "Setting listener.");
	ListenerOrient = *cur_pos->orient;
	ListenerPosition = *cur_pos->position;
	ListenerVelocty = *cur_pos->velocity;
	ListenerRoomNum = cur_pos->roomnum;

	//Update listener
	ALfloat buf[6];
	buf[0] = -ListenerOrient.fvec.x; buf[1] = ListenerOrient.fvec.y; buf[2] = ListenerOrient.fvec.z;
	buf[3] = -ListenerOrient.uvec.x; buf[4] = ListenerOrient.uvec.y; buf[5] = ListenerOrient.uvec.z;
	alListener3f(AL_POSITION, -ListenerPosition.x, ListenerPosition.y, ListenerPosition.z);
	alListener3f(AL_VELOCITY, -ListenerVelocty.x, ListenerVelocty.y, ListenerVelocty.z);
	alListenerfv(AL_ORIENTATION, (const ALfloat*)&buf);
	ALErrorCheck("Updating listener");
}

int llsOpenAL::PlaySound3d(play_information* play_info, int sound_index, pos_state* cur_pos, float master_volume, bool f_looped, float reverb)
{
	if (!Initalized) return -1;
	if (pSoundFiles[pSounds[sound_index].sample_index].used == 0) return -1;
	short sound_uid = FindSoundSlot(master_volume, play_info->priority);
	if (sound_uid < 0) return -1;

	//PutLog(LogLevel::Info, "Starting 3D source");

	bool looped = f_looped || (pSounds[sound_index].flags & SPF_LOOPED) != 0;

	InitSource3D(SoundEntries[sound_uid].handle, &pSounds[sound_index], cur_pos, master_volume);
	if (looped)
		alSourcei(SoundEntries[sound_uid].handle, AL_LOOPING, AL_TRUE);
	if (ALErrorCheck("Setting 3D sound source properties.")) return -1;

	play_info->left_volume = play_info->right_volume = master_volume;

	//Unbind the buffer so I can rewrite it:
	//TODO: Should each entry in pSound have their own buffer instead?
	alSourcei(SoundEntries[sound_uid].handle, AL_BUFFER, 0);
	BindBufferData(SoundEntries[sound_uid].bufferHandle, sound_index, looped);
	if (ALErrorCheck("Binding 3D sound source buffer.")) return -1;
	alSourcei(SoundEntries[sound_uid].handle, AL_BUFFER, SoundEntries[sound_uid].bufferHandle);
	if (ALErrorCheck("Setting 3D sound source buffer.")) return -1;

	alSourcePlay(SoundEntries[sound_uid].handle);
	ALErrorCheck("Starting 3D source.");
	NextUID++;
	SoundEntries[sound_uid].playing = true;
	SoundEntries[sound_uid].streaming = false;
	SoundEntries[sound_uid].volume = master_volume;
	SoundEntries[sound_uid].info = play_info;
	SoundEntries[sound_uid].soundNum = sound_index;
	SoundEntries[sound_uid].soundUID = NextUID * 256 + sound_uid;
	//PutLog(LogLevel::Info, "Starting 3D sound %s with uid %d (slot %d)", pSoundFiles[pSounds[sound_index].sample_index].name, SoundEntries[sound_uid].soundUID, sound_uid);

	NumSoundsPlaying++;
	return SoundEntries[sound_uid].soundUID;
}

void llsOpenAL::AdjustSound(int sound_uid, float f_volume, float f_pan, unsigned short frequency)
{
	if (!Initalized) return;
}

void llsOpenAL::AdjustSound(int sound_uid, pos_state* cur_pos, float adjusted_volume, float reverb)
{
	int id = sound_uid & 255;
	if (!Initalized) return;
	if (!SoundEntries || id < 0 || id >= NumSoundChannels || SoundEntries[id].soundUID != sound_uid) return;

	ALuint handle = SoundEntries[id].handle;
	alSource3f(handle, AL_DIRECTION, -cur_pos->orient->fvec.x, cur_pos->orient->fvec.y, cur_pos->orient->fvec.z);
	alSource3f(handle, AL_VELOCITY, -cur_pos->velocity->x, cur_pos->velocity->y, cur_pos->velocity->z);
	alSource3f(handle, AL_POSITION, -cur_pos->position->x, cur_pos->position->y, cur_pos->position->z);
	alSourcef(handle, AL_GAIN, adjusted_volume * pSounds[SoundEntries[id].soundNum].import_volume);
}

void llsOpenAL::StopAllSounds(void)
{
	if (!Initalized) return;
	//PutLog(LogLevel::Info, "Request to stop all sounds");
	for (int i = 0; i < NumSoundChannels; i++)
	{
		StopSound(SoundEntries[i].soundUID);
	}
}

bool llsOpenAL::IsSoundInstancePlaying(int sound_uid)
{
	int id = sound_uid & 255;
	if (!Initalized) return false;
	if (!SoundEntries || id < 0 || id >= NumSoundChannels || SoundEntries[id].soundUID != sound_uid) return false;
	return SoundEntries[id].playing;
}

int llsOpenAL::IsSoundPlaying(int sound_index)
{
	if (!Initalized) return -1;
	for (int i = 0; i < NumSoundChannels; i++)
	{
		if (SoundEntries[i].soundNum == sound_index && SoundEntries[i].playing)
			return SoundEntries[i].soundUID;
	}
	return -1;
}

void llsOpenAL::StopSound(int sound_uid, unsigned char f_immediately)
{
	int id = sound_uid & 255;
	if (!Initalized) return;
	if (!SoundEntries || id < 0 || id >= NumSoundChannels || SoundEntries[id].soundUID != sound_uid) return;
	if (SoundEntries[id].playing)
	{
		if (f_immediately == 1)
		{
			alSourceStop(SoundEntries[id].handle);
			SoundCleanup(id);

			//PutLog(LogLevel::Info, "External code trying to stop sound %d (%s). Elimination mode %d.", sound_uid, pSoundFiles[pSounds[SoundEntries[id].soundNum].sample_index].name, f_immediately);
		}
		else
		{
			//Disable looping traits on the sound instead
			alSourcei(SoundEntries[id].handle, AL_LOOPING, AL_FALSE);
			alSourcePlay(SoundEntries[id].handle);
			ALErrorCheck("Is it not okay to change AL_LOOPING on a playing source?");

			//PutLog(LogLevel::Info, "External code trying to stop sound %d (%s) from looping.", sound_uid, pSoundFiles[pSounds[SoundEntries[id].soundNum].sample_index].name);
		}

	}
}

// Pause all sounds/resume all sounds
void llsOpenAL::PauseSounds(void)
{
	if (!Initalized) return;
	int i;
	for (int i = 0; i < NumSoundChannels; i++)
	{
		PauseSound(i);
	}
}

void llsOpenAL::ResumeSounds(void)
{
	if (!Initalized) return;
	int i;
	for (int i = 0; i < NumSoundChannels; i++)
	{
		ResumeSound(i);
	}
}

void llsOpenAL::PauseSound(int sound_uid)
{
	int id = sound_uid & 255;
	if (!Initalized) return;
	if (!SoundEntries || id < 0 || id >= NumSoundChannels || SoundEntries[id].soundUID != sound_uid) return;
	alSourcePause(SoundEntries[id].handle);
}

void llsOpenAL::ResumeSound(int sound_uid)
{
	int id = sound_uid & 255;
	if (!Initalized) return;
	if (!SoundEntries || id < 0 || id >= NumSoundChannels || SoundEntries[id].soundUID != sound_uid) return;
	alSourcePlay(SoundEntries[id].handle);
}

bool llsOpenAL::CheckAndForceSoundDataAlloc(int sound_file_index)
{
	int sound_file_index_00;
	char cVar1;

	//PutLog(LogLevel::Info, "Allocating sound file");

	sound_file_index_00 = pSounds[sound_file_index].sample_index;
	if ((sound_file_index_00 < 0) || (999 < sound_file_index_00)) 
	{
		return false;
	}
	if ((pSoundFiles[sound_file_index_00].sample_16bit == NULL) && (pSoundFiles[sound_file_index_00].sample_8bit == NULL)) 
	{
		//PutLog(LogLevel::Info, "Force loading sound %d (%s)", sound_file_index, pSoundFiles[sound_file_index_00].name);
		cVar1 = SoundLoadWaveFile(pSoundFiles[sound_file_index_00].name, pSounds[sound_file_index].import_volume, sound_file_index_00, Quality == SQT_HIGH, true, NULL);
		return cVar1 != '\0';
	}
	return true;
}

// Begin sound frame
void llsOpenAL::SoundStartFrame(void)
{
	if (!Initalized) return;

	//Service streams
	for (int i = 0; i < NumSoundChannels; i++)
	{
		if (SoundEntries[i].playing && SoundEntries[i].streaming)
		{
			//PutLog(LogLevel::Info, "Servicing a stream");
			ServiceStream(i);
		}
	}
}

// End sound frame 
void llsOpenAL::SoundEndFrame(void)
{
	if (!Initalized) return;
	int i;
	ALint state;

	//PutLog(LogLevel::Info, "Ending sound system frame");

	//Free buffers and update playing state of all current sounds.
	for (int i = 0; i < NumSoundChannels; i++)
	{
		if (SoundEntries[i].playing)
		{
			//if (SoundEntries[i].streaming)
			//	ServiceStream(i);

			alGetSourcei(SoundEntries[i].handle, AL_SOURCE_STATE, &state);
			if (state == AL_STOPPED)
			{
				SoundCleanup(i);
				//PutLog(LogLevel::Info, "Sound %d (%s) has been stopped", SoundEntries[i].soundUID, pSoundFiles[pSounds[SoundEntries[i].soundNum].sample_index].name);
			}
			else if (state == AL_PLAYING)
			{
				SoundEntries[i].info->m_ticks++;
			}
		}
	}
}

bool llsOpenAL::SetGlobalReverbProperties(float volume, float damping, float decay)
{
	if (!EffectsSupported)
		return false;

	PutLog(LogLevel::Info, "Got Environmental params: vol: %f, damping: %f, decay: %f.", volume, damping, decay);
	Volume = volume;
	Damping = damping;
	Decay = decay;

	dalEffectf(EffectSlot, AL_EAXREVERB_DECAY_TIME, decay);
	ALErrorCheck("Setting reverb decay");
	dalEffectf(EffectSlot, AL_EAXREVERB_GAIN, min(volume, 1.0f));
	ALErrorCheck("Setting reverb gain");
	dalEffectf(EffectSlot, AL_EAXREVERB_GAINHF, min(damping, 1.0f));
	ALErrorCheck("Setting reverb gainhf");
	return true;
}

void llsOpenAL::SetEnvironmentValues(const t3dEnvironmentValues* env)
{
}

void llsOpenAL::GetEnvironmentValues(t3dEnvironmentValues* env)
{
}

void llsOpenAL::SetEnvironmentToggles(const t3dEnvironmentToggles* env)
{
}

void llsOpenAL::GetEnvironmentToggles(t3dEnvironmentToggles* env)
{
	env->doppler = true;
	env->geometry = false;
	env->flags = ENV3DVALF_DOPPLER;
}

llsOpenAL* llsOpenAL::llsOpenAL_PatchConstructor()
{
	//C++ doesn't allow taking the pointer of a constructor or destructor,
	//but we can instead remove the magician's illusion and make a function
	//that will replicate the constructor by creating a new instance of the class,
	//and then copying it into the pointer provided by __thiscall. It works since
	//there's no dynamically allocated storage (update: there's dynamically allocated storage but not created in the constructor, but uggghhhhh)

	//Good programmers don't write code that looks like... this...
	//MessageBoxA(nullptr, "Horrible hack has been sprung", "Cursed", MB_OK);
	llsOpenAL HorribleHackWhyAmIDoingthis = llsOpenAL();
	memcpy(this, &HorribleHackWhyAmIDoingthis, sizeof(HorribleHackWhyAmIDoingthis));

	//May as well get pointers now
	SoundLoadWaveFile = (char (*)(char* filename, float percent_volume, int sound_file_index, bool f_high_quality, bool f_load_sample_data, int* e_type))GetPatchPoint(PatchPoint::SoundLoadWaveFileFunc);
	pSounds = (sound_info*)GetPatchPoint(PatchPoint::SoundsVar);
	pSoundFiles = (sound_file_info*)GetPatchPoint(PatchPoint::SoundFilesVar);

	return this;
}

const char* ALErrors[4] = { "Invalid enum", "Invalid name", "Invalid operation", "Invalid value" };

bool llsOpenAL::ALErrorCheck(const char* context)
{
	int error;
	error = alGetError();
	const char* msg = "";
	if (error != AL_NO_ERROR)
	{
		if (error == AL_INVALID_ENUM)
			msg = ALErrors[0];
		else if (error == AL_INVALID_NAME)
			msg = ALErrors[1];
		else if (error == AL_INVALID_OPERATION)
			msg = ALErrors[2];
		else if (error == AL_INVALID_VALUE)
			msg = ALErrors[3];
		else
			msg = "Unknown error code";

		PutLog(LogLevel::Error, "Error in context %s: %s.", context, msg);
		return true;
	}
	return false;
}

short llsOpenAL::FindSoundSlot(float volume, int priority)
{
	int i;
	int bestPriority = INT_MAX;
	int bestSlot = -1;
	float bestVolume = 1.0f;
	//PutLog(LogLevel::Info, "Finding a sound slot");
	//No free slots, so bump a low priorty one.
	if (NumSoundsPlaying >= NumSoundChannels)
	{
		for (i = 0; i < NumSoundChannels; i++)
		{
			//check for lower priority sound
			if (SoundEntries[i].info->priority < bestPriority)
			{
				bestSlot = i;
				bestVolume = SoundEntries[i].volume;
				bestPriority = SoundEntries[i].info->priority;
			}
			//At least attempt to get the quietest sound possible
			else if (SoundEntries[i].info->priority == bestPriority)
			{
				if (SoundEntries[i].volume < bestVolume)
				{
					bestSlot = i;
					bestVolume = SoundEntries[i].volume;
				}
			}
		}

		if (bestSlot != -1)
		{
			StopSound(SoundEntries[bestSlot].soundUID);
			PutLog(LogLevel::Warning, "Bumping sound %d due to insufficient slots.", bestSlot);
		}
		else
		{
			PutLog(LogLevel::Warning, "Can't find sound slot and unable to bump sound.");
		}

		return bestSlot;
	}
	else //Free slots available, so find one.
	{
		for (i = 0; i < NumSoundChannels; i++)
		{
			if (!SoundEntries[i].playing)
				return i;
		}
	}
	PutLog(LogLevel::Warning, "Can't find sound slot. NumSoundsPlaying is %d.", NumSoundsPlaying);
	return -1;
}

void llsOpenAL::InitSource2D(uint32_t handle, sound_info* soundInfo, float volume)
{
	alSourcei(handle, AL_SOURCE_RELATIVE, AL_TRUE); //should glue source to listener pos
	alSourcef(handle, AL_ROLLOFF_FACTOR, 0.0f);
	alSource3f(handle, AL_DIRECTION, 0.f, 0.f, 0.f);
	alSource3f(handle, AL_VELOCITY, 0.f, 0.f, 0.f);
	alSource3f(handle, AL_POSITION, 0.f, 0.f, 0.f);
	alSourcef(handle, AL_MAX_GAIN, 1.f);
	alSourcef(handle, AL_GAIN, volume * soundInfo->import_volume);
	alSourcef(handle, AL_PITCH, 1.f);
	alSourcef(handle, AL_DOPPLER_FACTOR, 0.f);

	alSourcef(handle, AL_CONE_INNER_ANGLE, 360.0f);
	alSourcef(handle, AL_CONE_OUTER_ANGLE, 360.0f);

	alSourcei(handle, AL_LOOPING, AL_FALSE);
}

void llsOpenAL::InitSourceStreaming(uint32_t handle, float volume)
{
	alSourcei(handle, AL_SOURCE_RELATIVE, AL_TRUE); //should glue source to listener pos
	alSourcef(handle, AL_ROLLOFF_FACTOR, 0.0f);
	alSource3f(handle, AL_DIRECTION, 0.f, 0.f, 0.f);
	alSource3f(handle, AL_VELOCITY, 0.f, 0.f, 0.f);
	alSource3f(handle, AL_POSITION, 0.f, 0.f, 0.f);
	alSourcef(handle, AL_MAX_GAIN, 1.f);
	alSourcef(handle, AL_GAIN, volume);
	alSourcef(handle, AL_PITCH, 1.f);
	alSourcef(handle, AL_DOPPLER_FACTOR, 0.f);

	alSourcef(handle, AL_CONE_INNER_ANGLE, 360.0f);
	alSourcef(handle, AL_CONE_OUTER_ANGLE, 360.0f);

	alSourcei(handle, AL_LOOPING, AL_FALSE);
}

void llsOpenAL::InitSource3D(uint32_t handle, sound_info* soundInfo, pos_state* posInfo, float volume)
{
	alSourcei(handle, AL_SOURCE_RELATIVE, AL_FALSE);
	alSourcef(handle, AL_ROLLOFF_FACTOR, 1.0f);
	alSource3f(handle, AL_DIRECTION, -posInfo->orient->fvec.x, posInfo->orient->fvec.y, posInfo->orient->fvec.z);
	alSource3f(handle, AL_VELOCITY, -posInfo->velocity->x, posInfo->velocity->y, posInfo->velocity->z);
	alSource3f(handle, AL_POSITION, -posInfo->position->x, posInfo->position->y, posInfo->position->z);
	alSourcef(handle, AL_MAX_GAIN, 1.f);
	alSourcef(handle, AL_GAIN, volume * soundInfo->import_volume);
	alSourcef(handle, AL_PITCH, 1.f);
	alSourcef(handle, AL_DOPPLER_FACTOR, 0.6f);

	//PutLog(LogLevel::Info, "Starting 3d sound at %f %f %f", posInfo->position->x, posInfo->position->y, posInfo->position->z);

	if (soundInfo->flags & SPF_USE_CONE)
	{
		alSourcef(handle, AL_CONE_OUTER_GAIN, soundInfo->outer_cone_volume);
		alSourcef(handle, AL_CONE_INNER_ANGLE, soundInfo->inner_cone_angle);
		alSourcef(handle, AL_CONE_OUTER_ANGLE, soundInfo->outer_cone_angle);
	}
	else
	{
		alSourcef(handle, AL_CONE_INNER_ANGLE, 360.0f);
		alSourcef(handle, AL_CONE_OUTER_ANGLE, 360.0f);
	}

	alSourcef(handle, AL_REFERENCE_DISTANCE, soundInfo->min_distance);
	alSourcef(handle, AL_MAX_DISTANCE, soundInfo->max_distance);

	alSourcei(handle, AL_LOOPING, AL_FALSE);

	if (EffectsSupported)
		alSource3i(handle, AL_AUXILIARY_SEND_FILTER, AuxEffectSlot, 0, NULL);
}

void llsOpenAL::BindBufferData(uint32_t handle, int sound_index, bool looped)
{
	ALenum fmt = Quality == SQT_HIGH ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
	int bpp = (Quality == SQT_HIGH ? 2 : 1);
	void* ptr = Quality == SQT_HIGH ? (ALvoid*)pSoundFiles[pSounds[sound_index].sample_index].sample_16bit : (ALvoid*)pSoundFiles[pSounds[sound_index].sample_index].sample_8bit;
	int len = bpp * pSoundFiles[pSounds[sound_index].sample_index].np_sample_length;

	//PutLog(LogLevel::Info, "Buffering sound id %d (%s)", sound_index, pSoundFiles[pSounds[sound_index].sample_index].name);
	if (!ptr)
	{
		PutLog(LogLevel::Error, "Tried to start sound %s but got nullptr.", pSoundFiles[pSounds[sound_index].sample_index].name);
		return;
	}
	alBufferData(handle, fmt, ptr, len, 22050);

	pSoundFiles[pSounds[sound_index].sample_index].use_count++;
	ALErrorCheck("Binding buffer data");

	
	if (looped && LoopPointsSupported)
	{
		ALint loopPoints[2];
		loopPoints[0] = pSounds[sound_index].loop_start;
		loopPoints[1] = pSounds[sound_index].loop_end;

		//uh
		if (bpp == 1)
		{
			loopPoints[0] >>= 1;
			loopPoints[1] >>= 1;
		}

		//bounds check
		if (loopPoints[0] > len) loopPoints[0] = 0;
		if (loopPoints[1] > len) loopPoints[1] = len;

		alBufferiv(handle, AL_LOOP_POINTS_SOFT, &loopPoints[0]);
		ALErrorCheck("Setting buffer loop points");
	}
}

void llsOpenAL::SoundCleanup(int soundID)
{
	SoundEntries[soundID].playing = false;
	pSoundFiles[pSounds[SoundEntries[soundID].soundNum].sample_index].use_count--;
	//PutLog(LogLevel::Info, "Sound %d (%s) has been stopped", SoundEntries[i].soundUID, pSoundFiles[pSounds[SoundEntries[i].soundNum].sample_index].name);
	if (NumSoundsPlaying > 0)
		NumSoundsPlaying--;
	else
		PutLog(LogLevel::Warning, "LLS Bookkeeping failed, NumSoundsPlaying went negative");

	//Unbind the buffer for later use
	alSourcei(SoundEntries[soundID].handle, AL_BUFFER, 0);
	//Clear the sound's send filter, if it has one. 
	if (EffectsSupported)
	{
		alSource3i(SoundEntries[soundID].handle, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, NULL);
		ALErrorCheck("Clearing source send filter");
	}

	//Streaming sounds need to clean up all their buffers
	if (SoundEntries[soundID].streaming)
	{
		ALint numProcessedBuffers, numQueuedBuffers;
		ALuint dequeueList[NUM_STREAMING_BUFFERS];
		int i;
		alGetSourcei(SoundEntries[soundID].handle, AL_BUFFERS_PROCESSED, &numProcessedBuffers);
		if (numProcessedBuffers > 0)
		{
			//back up current buffers in buffer queue
			for (i = 0; i < NUM_STREAMING_BUFFERS - numProcessedBuffers; i++)
			{
				dequeueList[i] = SoundEntries[soundID].bufferHandles[SoundEntries[soundID].bufferQueue[i]];
			}

			alSourceUnqueueBuffers(SoundEntries[soundID].handle, numProcessedBuffers, SoundEntries[soundID].bufferQueue);
		}
		ALErrorCheck("Dequeueing ended stream buffers");

		alDeleteBuffers(NUM_STREAMING_BUFFERS, SoundEntries[soundID].bufferHandles);
		ALErrorCheck("Destroying stream buffers");
	}
}

void llsOpenAL::ServiceStream(int soundID)
{
	ALint numProcessedBuffers, numQueuedBuffers;
	ALint position;
	ALuint dequeueList[NUM_STREAMING_BUFFERS];
	int i;
	alGetSourcei(SoundEntries[soundID].handle, AL_BUFFERS_PROCESSED, &numProcessedBuffers);
	if (numProcessedBuffers > 0)
	{
		//mark freed buffers as usable again
		for (i = 0; i < numProcessedBuffers; i++)
		{
			dequeueList[i] = SoundEntries[soundID].bufferHandles[SoundEntries[soundID].bufferQueue[i]];
			SoundEntries[soundID].bufferStatus[SoundEntries[soundID].bufferQueue[i]] = false;
		}
		//remove the unqueued buffers from the list by moving further buffer entries over them. 
		for (i = 0; i < NUM_STREAMING_BUFFERS - numProcessedBuffers; i++)
		{
			SoundEntries[soundID].bufferQueue[i] = SoundEntries[soundID].bufferQueue[i + numProcessedBuffers];
		}

		alSourceUnqueueBuffers(SoundEntries[soundID].handle, numProcessedBuffers, dequeueList);
		//PutLog(LogLevel::Info, "%d buffers dequeued. [%d %d %d]", numProcessedBuffers, SoundEntries[soundID].bufferQueue[0], SoundEntries[soundID].bufferQueue[1], SoundEntries[soundID].bufferQueue[2]);
	}
	ALErrorCheck("Dequeueing buffers");

	alGetSourcei(SoundEntries[soundID].handle, AL_BUFFERS_QUEUED, &numQueuedBuffers);
	alGetSourcei(SoundEntries[soundID].handle, AL_BYTE_OFFSET, &position);
	if (numQueuedBuffers < NUM_STREAMING_BUFFERS) //queue up a new one
	{
		int newSlot = 0;
		int size = SoundEntries[soundID].info->m_stream_bufsize;
		void* data;
		//Find first slot that's available
		for (i = 0; i < NUM_STREAMING_BUFFERS; i++)
		{
			if (!SoundEntries[soundID].bufferStatus[i])
			{
				newSlot = i;
				break;
			}
		}

		SoundEntries[soundID].info->m_samples_played += SoundEntries[soundID].info->m_stream_bufsize;

		//Get available data and queue it
		data = SoundEntries[soundID].info->m_stream_cback(SoundEntries[soundID].info->user_data, SoundEntries[soundID].info->m_stream_handle, &size);

		SoundEntries[soundID].info->m_stream_bufsize = size;
		SoundEntries[soundID].info->m_stream_data = data;

		if (!data)
		{
			//PutLog(LogLevel::Error, "Tried to queue streaming buffer, but got nullptr.");
			return;
		}

		if (size != 0 && data != nullptr)
		{
			alBufferData(SoundEntries[soundID].bufferHandles[newSlot], SoundEntries[soundID].streamFormat, data, size, 22050);
			ALErrorCheck("Creating stream buffer");
			alSourceQueueBuffers(SoundEntries[soundID].handle, 1, &SoundEntries[soundID].bufferHandles[newSlot]);
			SoundEntries[soundID].bufferQueue[numQueuedBuffers] = newSlot;
			SoundEntries[soundID].bufferStatus[newSlot] = true;
			ALErrorCheck("Queueing buffer");

			//PutLog(LogLevel::Info, "Buffer queued to %d.", newSlot);
		}
		else
		{
			//PutLog(LogLevel::Info, "Slots available to queue but callback returned no bytes.");
		}
	}
}
