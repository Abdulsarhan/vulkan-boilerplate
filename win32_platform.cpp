#include <Windows.h>
#include "vk_init.h"
#include "vk_renderer.h"
#include "vk_renderer.hpp"
#include "platform.h"
#include "win32_platform.h"

static bool running = true;
HWND window = 0;

LRESULT CALLBACK platform_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_CLOSE:
		running = false;
		break;
	}

	return DefWindowProcA(window, msg, wParam, lParam);
}

bool platform_create_window() {
	HINSTANCE instance = GetModuleHandleA(NULL);

	WNDCLASSEXA windowclass = {0};

	windowclass.cbSize = sizeof(WNDCLASSEXA);
	windowclass.lpfnWndProc = platform_window_callback;
	windowclass.hInstance = instance;
	windowclass.lpszClassName = "Vulkan_Window_Class";
	windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClassExA(&windowclass)) {
		MessageBoxA(0, "Failed Registering Window Class", "Error", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	window = CreateWindowExA(WS_EX_APPWINDOW, windowclass.lpszClassName, "Triangle",
							WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_OVERLAPPED,
							100, 100, 1280, 720, 0, 0, instance, 0);
	if (window == 0) {
		MessageBoxA(0, "Failed creating window", "Error", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	ShowWindow(window, SW_SHOW);

	return true;
}

void platform_update_window(HWND window)
{
	MSG msg;

	while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

int main() {

	VkContext vkcontext = {};

	if (!platform_create_window()) {
		return -1;
	}

	if (!InitVulkan(&vkcontext, window)) {
		MessageBoxA(window, "Failed to Initialize Vullkan", "Error", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	while (running) {
		platform_update_window(window);
		Renderer(&vkcontext);
	}

	return 0;
}


void platform_get_window_size(uint32_t* width, uint32_t* height) {
	RECT rect;
	GetClientRect(window, &rect);

	*width = rect.right - rect.left;
	*height = rect.bottom - rect.top;
}

char* platform_read_file(const char* path, uint32_t* length) {
	char* result = 0;

	HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (file != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER size;
		if (GetFileSizeEx(file, &size)) {
			*length = (uint32_t)size.QuadPart;
			result = new char[*length];

			DWORD bytesRead;
			if (ReadFile(file, result, *length, &bytesRead, 0)) {
				// Success
			}
			else {
				std::cout << "ERROR: Failed to read file." << "\n";

			}
			
		}
		else {
			std::cout << "ERROR: Failed to get size of file." << "\n";
		}
		
		CloseHandle(file);
	}
	else {
		std::cout << "ERROR: Failed to open file." << "\n";
	}
	return result;
}