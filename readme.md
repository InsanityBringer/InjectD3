# InjectD3

Project to patch Descent 3 in memory with dll injection. Inspired by [inject](https://github.com/mewrev/inject) in turn inspired by [diablo-improvements](https://github.com/r1sc/diablo-improvements)
It may eventually override a dll the game normally uses (like OpenGL32.dll) to avoid the need for a separate inject program. 

## Credits
Arne for getting this mess started in the first place by finding functions in the game's executable, for pointing me to inject, and all sorts of technical support.
mewrev for inject, which InjectD3 was built on top of.
The new sound code uses OpenAL Soft for full 3D sound. 
