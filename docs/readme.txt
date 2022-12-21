InjectD3:

	This is a program to patch Descent 3 with various fixes and changes. 

Requirements:

	At the moment, InjectD3 works with two versions of Descent 3:
		* EU 1.4, 1,818,624 bytes, 
			SHA256: 8B6756BFE10EAD8ABAEFF2E76E58BC3AB0564F8D2DA9860B577B3620585FA50F
			If you have the dateiliste repack version, you should have this version.
			The version with Alexi's 32-bit patch should work without conflicts with my patch. 
			
		* US 1.4, 1,794,048 bytes,
			SHA256: 74D2F678BCB2C4369DD1E05C2F78D7A038884C54B64AE639211C17652316C3F4
			This version is preferred, since it is included with copies purchased on GOG.com and Steam.
			The "Black Pyro Justice" mod is built against this version, and may work with patches.
			
	Additionally, InjectD3Configuration requires a .net runtime to run. If this isn't present, the config file
	can still be edited manually.
			
Basic usage:

* Unzip the contents of the archive into your Descent 3 directory. 
* Configure Descent 3 with the original launcher to use OpenGL graphics, as only the OpenGL renderer is patched at the moment.
* (optional) Rename or delete the "movies" folder. The FMV sequences will still disrupt the screen resolution, as I haven't 
	found where to patch it yet.
* Run InjectD3Configuration to configure the patch. This only needs to be done once, unless you wish to change options later.
* Run InjectD3 instead of the original launcher to start the game. 

Available patches:

	The following patches are available at the moment, grouped into 3 sections currently.
	
	General patches:
		Use HKEY_CURRENT_USER:
			Makes the game store registry configuration within the HKEY_CURRENT_USER hive. This makes the configuration 
			per-user, and makes it possible to configure the game without having to run as administrator on later versions
			of Windows. Use the included configuration program to set basic configuration options in place of the game's
			original launcher program.
			
			InjectD3.cfg name: UseUserRegistry
			Valid values: 0, 1. 
	
	Graphics patches:
		Display adaptor: 
			Forces the screen onto a given display. Only works with Windowed or NewFullscreen modes.
		
			InjectD3.cfg name: DisplayNum
			Valid values: -1 to the highest numbered display available.
			
		Field of view:
			The game's field of view. 72 is the default.
			
			InjectD3.cfg name: FieldOfView
			Valid values: 10 to 179. 
			
		Force 32-bit color:
			Forces the game to always use 32-bit color for the display.
			
			InjectD3.cfg name: Force32Bit
			Valid values: 0 or 1.
		
		Force -z32
			Forces the -z32 command line option.
			
			InjectD3.cfg name: ForceZ32
			Valid values: 0 or 1.
			
		Multisampling count
			Enables multisampling for basic anti-aliasing. The count is the amount of samples taken.
			
			InjectD3.cfg name: MultisampleCount
			Valid values: 1 to disable, 2, 4, 8, 16... to enable
			
		Screen mode:
			Changes the game's screen mode. There are 3 options:
				*Original: The original fullscreen mode, which disrupts your desktop resolution and layout. Not recommended.
				*Windowed: Windowed mode. The size of the window will match the game's current resolution. 
					Use -width and -height command line parameters to change this resolution in-game.
					Note that at the moment the dedicated server will not function correctly when this patch is set.
				*BorderlessFullscreen: Fullscreen via borderless window. The game's contents will be scaled to match your
					current desktop resolution. Use -width and -height command line parameters to change this resolution 
					in-game.
	
			InjectD3.cfg name: ScreenMode
			Valid values: 0 for Original, 1 for Windowed, 2 for BorderlessFullscreen.
			
		Always use SSE: 
			Forces the use of SSE features, and other Katmai only features. Among other things, this will enable motion blur
			and powerup sparkles by default on AMD and other non-Intel systems.
		
			InjectD3.cfg name: AlwaysKatmai
			Valid values: 0 or 1.
			
		Enable OpenGL specular highlights: 
			Enables specular highlights in OpenGL. This was deliberately disabled in the original game for some reason, but I
			haven't noticed any serious issues enabling its support. 
		
			InjectD3.cfg name: OpenGLSpecular
			Valid values: 0 or 1.
			
		UI Framerate:
			The maximum framerate when a UI window is open. Defaults to 20. Increasing the framerate will also increase the
			speed at which the mission selector dialog will open. 
			
			InjectD3.cfg name: UIFrameRate
			Valid values: Anything 20 or above. 
			
	Input patches:
		New mouse library:
			Enables new mouse code, using Windows's Raw Input APIs. Absolutely vital for windowed and borderless windowed modes
			I don't even know why this is a user option, it is literally impossible to use those modes with the old DirectInput
			mouse code. This also *should* fix Fusion and Mass Driver dropping inputs at high framerates, so long as you don't
			run with -lorestimer for some dumb reason. 
			
			InjectD3.cfg name: NewMouse
			Valid values: 1.
			
		Mouse prescalar:
			Prescales mouse input by the specified scalar. The default mouse values can be very extreme, and the Descent 3
			configuration doesn't allow very precise control over mouse sensitivity, so this should make it easier to work
			with. 
			
			InjectD3.cfg name: MouseScalar
			Valid values: Any valid floating point value. 
			
	Audio patches:
		OpenAL sound code:
			Patches the original sound system with a new OpenAL based sound system. This option is currently very experimental,
			and may crash or sound weird on some systems. If it works, this system allows for better positional audio 
			(currently with linear falloff though), and avoids the crackling problem with DirectSound on modern Windows
			systems.
			
			InjectD3.cfg name: NewSoundSystem
			Valid values: 0, 1
		OpenAL reverbs:
			Enables reverbations for the OpenAL sound system, similar to the original game's sound system when using EAX-
			enabled sound cards.
			
			InjectD3.cfg name: NewSoundSystemReverbs
			Valid values: 0, 1
			
	Additionally, the following patches are currently always applied:
		Postrender overflow fix:
			When too many particles are created, especially in levels with terrain, the "postrender" list can overflow and
			corrupt memory, always leading to a crash. This is patched so that the limit is be enforced. This should
			prevent crashes while fighting the Hellion, crashes fighting the ships in level 7, and so on.
			
		OpenGL fixes:
			When using any of the new screen modes or multisampling, the game will apply patches to the OpenGL renderer.
			This includes a fix for textures not working on AMD GPUs that are using the new AMD OpenGL implementation. 
			
Known bugs:

	The following bugs are known and will be fixed in future versions, hopefully:
		*FMV sequences will still disrupt the screen resolution. 
		*Dedicated server won't work when Windowed mode is patched. 
		*Cockpit is unusable with widescreen resolutions.
		*Borderless window is slightly hacky. Ah, if I had the patience I'd make an API on top of vulkan that isn't as low
			level, but retains the much better context creation features to avoid Windows's OpenGL hacks.
		*Terrain system draw limits are made worse by the expanded FOV. Setting your terrain quality to 27 will fix issues
			but reqires the minimum draw distance. To increase the draw distance ATM will involve finding a balance of quality
			and render distance. I hope to patch this in the future, but it is a nontrivial fix. 
			
Future patches:

	The following patches are planned:
		*Shader-based OpenGL renderer that supports bumpmaps.
		*Ability to automatically set -width, -height, -aspect, -framecap, and other command-line parameters automatically
			from InjectD3.cfg
		*Support for more versions of Descent 3. 
		
Credits:

Arne for getting this mess started in the first place by finding functions in the game's executable, for pointing me to inject, and all sorts of technical support.
mewrev for inject (https://github.com/mewrev/inject), which InjectD3 was built on top of.
The new sound code uses OpenAL Soft for full 3D sound.
