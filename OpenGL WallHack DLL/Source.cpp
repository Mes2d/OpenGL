#include <Windows.h>
HMODULE openGLHandle = NULL;
void(__stdcall* glDepthFunc)(unsigned int) = NULL;
void(__stdcall* glDepthRange)(double, double) = NULL;
unsigned char* hook_location;
DWORD ret_address = 0;
DWORD old_protect;
DWORD count = 0;
__declspec(naked) void codecave() {
	__asm {
		pushad
		mov eax, dword ptr ds : [esp + 0x10]
		mov count, eax
		popad
		pushad
	}
	if (count > 500) {
		(*glDepthRange)(0.0, 0.0);
		(*glDepthFunc)(0x207);
	}
	else {
		(*glDepthRange)(0.0, 1.0);
		(*glDepthFunc)(0x203);
	}
	__asm {
		popad
		mov esi, dword ptr ds : [esi + 0xA18]
		jmp ret_address
	}
}
void injected_thread() {
	while (true) {
		if (openGLHandle == NULL) {
			openGLHandle = GetModuleHandle(L"opengl32.dll");
		}
		if (openGLHandle != NULL && glDepthFunc == NULL) {
			glDepthFunc = (void(__stdcall*)(unsigned int))GetProcAddress(openGLHandle, "glDepthFunc");
			glDepthRange = (void(__stdcall*)(double, double))GetProcAddress(openGLHandle, "glDepthRange");
			hook_location = (unsigned char*)GetProcAddress(openGLHandle, "glDrawElements");
			hook_location += 0x16;
			VirtualProtect((void*)hook_location, 5, PAGE_EXECUTE_READWRITE, &old_protect);
			*hook_location = 0xE9;
			*(DWORD*)(hook_location + 1) = (DWORD)&codecave - ((DWORD)hook_location + 5);
			*(hook_location + 5) = 0x90;
			ret_address = (DWORD)(hook_location + 0x6);
		}
		Sleep(1);
	}
}
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)injected_thread, NULL, 0, NULL);
	}
	return true;
}