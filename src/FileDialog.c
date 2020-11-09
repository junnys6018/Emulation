#include "FileDialog.h"

#ifdef PLATFORM_WINDOWS
#include <Windows.h>

int OpenFileDialog(char* filepath, int size)
{
	OPENFILENAMEA ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = filepath;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = size;
	ofn.lpstrFilter = "All\0*.*\0Nes (.nes)\0*.nes\0";
	ofn.nFilterIndex = 2;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	return ((GetOpenFileNameA(&ofn) != 0) ? 0 : 1);
}

#endif

#ifdef PLATFORM_LINUX

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int OpenFileDialog(char* filepath, int size)
{
	FILE* f = popen("zenity --file-selection --modal", "r");
	if (!f)
	{
		return 1;
	}

	fgets(filepath, size, f);
	printf(filepath);

	// Remove newline
	char* newline = strrchr(filepath, '\n');
	*newline = '\0';

	if (pclose(f) == -1)
	{
		return 1;
	}

	return 0;
}
#endif

#ifdef __EMSCRIPTEN__
#include <string.h>
int OpenFileDialog(char* filepath, int size)
{
	strcpy(filepath, "roms/LegendofZelda.nes");
	return 0;
}
#endif

