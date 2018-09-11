#include "util.h"
#include <stdio.h>
#include <stdlib.h>

extern r32 utilRandomFloat(r32 min, r32 max)
{
    r32 scale = rand() / (r32)RAND_MAX;
    return min + scale * (max - min);
}

extern s8* utilReadFile(const s8* path, s32* _fileLength)
{
	FILE* file;
	s8* buffer;
	s32 fileLength;

	file = fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	fileLength = ftell(file);
	rewind(file);

	buffer = (s8*)malloc((fileLength + 1) * sizeof(s8));
	fread(buffer, fileLength, 1, file);
	fclose(file);

	buffer[fileLength] = '\0';

	if (_fileLength)
		*_fileLength = fileLength;

	return buffer;
}

extern void utilFreeFile(s8* file)
{
	free(file);
}