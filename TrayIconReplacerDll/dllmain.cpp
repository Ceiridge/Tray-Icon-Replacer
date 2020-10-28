#include "pch.h"

#define DEBUGGER_BREAK() if(IsDebuggerPresent()) { DebugBreak(); } // Break only if a debugger is present
typedef BOOL(WINAPI* Shell_NotifyIconW_Type)(DWORD, PNOTIFYICONDATAW);

HICON icon = NULL; // A handle to the icon that replaces the old ones
Shell_NotifyIconW_Type Orig_Shell_NotifyIconW;

BOOL WINAPI Hook_Shell_NotifyIconW(DWORD dwMessage, PNOTIFYICONDATAW lpData) {
	lpData->hIcon = icon;
	return Orig_Shell_NotifyIconW(dwMessage, lpData);
}

BOOL APIENTRY Main(LPVOID hModule) { // Hook Shell_NotifyIconW
	icon = (HICON)LoadImage(NULL, TEXT("replacedicon.ico"), IMAGE_ICON, NULL, NULL, LR_LOADFROMFILE); // Load .\replacedicon.ico with its width and height

	if (!icon) {
		DEBUGGER_BREAK();
		return FALSE;
	}

	if (MH_Initialize() != MH_OK) {
		DEBUGGER_BREAK();
		return FALSE;
	}

	if (MH_CreateHook(&Shell_NotifyIconW, &Hook_Shell_NotifyIconW, (LPVOID*)&Orig_Shell_NotifyIconW) != MH_OK) {
		DEBUGGER_BREAK();
		return FALSE;
	}

	if (MH_EnableHook(&Shell_NotifyIconW) != MH_OK) {
		DEBUGGER_BREAK();
		return FALSE;
	}

	return TRUE;
}

BOOL APIENTRY UnHook(LPVOID hModule) { // Remove the Shell_NotifyIconW hook
	if (MH_DisableHook(&Shell_NotifyIconW) != MH_OK) {
		DEBUGGER_BREAK();
		return FALSE;
	}

	if (MH_Uninitialize() != MH_OK) {
		DEBUGGER_BREAK();
		return FALSE;
	}

	return TRUE; // Note: I intentionally keep the icon handle loaded (memory leak), because the struct may not change => prevents crashes
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)Main, hModule, NULL, NULL);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)UnHook, hModule, NULL, NULL);
			break;
	}

	return TRUE;
}

