#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef LINUX
	#define __declspec(x)
	#define __cdecl
	#define __stdcall
#endif

#endif //PLATFORM_H
