// Derived from "Inject", released into the public domain.
//
// Inject is a tool which injects an ordered list of shared libraries into the
// address space of a binary executable. The created process is initially
// suspended, and resumes execution once the ordered list of shared libraries
// have been loaded into its address space, and their respective DllMain
// functions have finished executing.

#include <stdio.h>
#include <windows.h>
#include <string>

int main(int argc, char** argv)
{
	int i, len;
	void* page;
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	HANDLE hThread;
	std::string str;
	char lib_path[_MAX_PATH] = "Descent3PatcherDll.dll";
	char exe_path[256] = "main.exe";

	// Find alternate exe file (maybe Pyromania will work?)
	for (i = 1; i < argc; i++)
	{
		if (!strcmp("-exe", argv[i]) && i + 1 < argc)
		{
			strncpy(exe_path, argv[i + 1], 255);
			//Ensure null terminated
			exe_path[255] = '\0';
		}
		
		//This is for debugging purposes, to allow the debugger to mount the DLL I want. 
		if (!strcmp("-dll", argv[i]) && i + 1 < argc)
		{
			strncpy(lib_path, argv[i + 1], _MAX_PATH - 1);
			//Ensure null terminated
			lib_path[_MAX_PATH - 1] = '\0';
		}
	}
	// Execute the process in suspended mode.
	//sprintf(buf, "%s -launched", exe_path);

	//[ISB] Build Descent 3's command line
	//Use C++ string because I'm a lazy fuck
	str.append(const_cast<char*>(exe_path));
	str.append(" -launched");
	//TODO: Skip the exe path at some point I guess
	for (i = 1; i < argc; i++)
	{
		str.append(" ");
		str.append(const_cast<char*>(argv[i]));
	}

	si.cb = sizeof(STARTUPINFO);
	if (!CreateProcess(NULL, const_cast<LPSTR>(str.c_str()), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
	{
		fprintf(stderr, "CreateProcess(\"%s\") failed; error code = 0x%08X\n", exe_path, GetLastError());
		return 1;
	}

	// Allocate a page in memory for the arguments of LoadLibrary.
	page = VirtualAllocEx(pi.hProcess, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (page == NULL) {
		fprintf(stderr, "VirtualAllocEx failed; error code = 0x%08X\n", GetLastError());
		return 1;
	}

	len = strlen(lib_path) + 1;
	if (len > MAX_PATH)
	{
		fprintf(stderr, "path length (%d) exceeds MAX_PATH (%d).\n", len, MAX_PATH);
		return 1;
	}
	//[ISB] Ensure the inject lib exists
	if (GetFileAttributes(lib_path) == INVALID_FILE_ATTRIBUTES)
	{
		fprintf(stderr, "unable to locate library (%s).\n", lib_path);
		return 1;
	}

	// Write library path to the page used for LoadLibrary arguments.
	if (WriteProcessMemory(pi.hProcess, page, lib_path, len, NULL) == 0)
	{
		fprintf(stderr, "WriteProcessMemory failed; error code = 0x%08X\n", GetLastError());
		return 1;
	}

	// Inject the shared library into the address space of the process,
	// through a call to LoadLibrary.
	hThread = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, page, 0, NULL);
	if (hThread == NULL)
	{
		fprintf(stderr, "CreateRemoteThread failed; error code = 0x%08X\n", GetLastError());
		return 1;
	}

	// Wait for DllMain to return.
	if (WaitForSingleObject(hThread, INFINITE) == WAIT_FAILED) {
		fprintf(stderr, "WaitForSingleObject failed; error code = 0x%08X\n", GetLastError());
		return 1;
	}

	// Cleanup.
	CloseHandle(hThread);

	// Resume the execution of the process, once all libraries have been injected
	// into its address space.
	if (ResumeThread(pi.hThread) == -1)
	{
		fprintf(stderr, "ResumeThread failed; error code = 0x%08X\n", GetLastError());
		return 1;
	}

	// Cleanup.
	CloseHandle(pi.hProcess);
	VirtualFreeEx(pi.hProcess, page, MAX_PATH, MEM_RELEASE);
	return 0;
}
