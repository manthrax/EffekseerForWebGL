
#pragma once

#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <emscripten.h>
#include "Effekseer.h"

namespace EfkWebViewer
{
	class CustomFileReader : public Effekseer::FileReader
	{
		uint8_t* fileData;
		size_t fileSize;
		int currentPosition;
	public:
		CustomFileReader(uint8_t* fileData, size_t fileSize) 
			: fileData(fileData), fileSize(fileSize), currentPosition(0) {
		}
		~CustomFileReader() {
			free(fileData);
		}
		size_t Read( void* buffer, size_t size) {
			if (currentPosition + size > fileSize) {
				size = fileSize - currentPosition;
			}
			memcpy(buffer, fileData + currentPosition, size);
			currentPosition += size;
			return size;
		}
		void Seek(int position) {
			currentPosition = position;
		}
		int GetPosition() {
			return currentPosition;
		}
		size_t GetLength() {
			return fileSize;
		}
	};

	class CustomFileInterface : public Effekseer::FileInterface
	{
	public:
		Effekseer::FileReader* OpenRead( const EFK_CHAR* path ) {
			// Request to load file
			int loaded = EM_ASM_INT({
				return Module._loadBinary(UTF16ToString($0)) != null;
			}, path);
			if (!loaded) {
				return nullptr;
			}

			uint8_t *fileData;
			int fileSize;

			// Copy data from arraybuffer
			EM_ASM_INT({
				var buffer = Module._loadBinary(UTF16ToString($0));
				var memptr = _malloc(buffer.byteLength);
				HEAP8.set(new Uint8Array(buffer), memptr);
				setValue($1, memptr, "i32");
				setValue($2, buffer.byteLength, "i32");
			}, path, &fileData, &fileSize);

			return new CustomFileReader(fileData, fileSize);
		}
		Effekseer::FileWriter* OpenWrite( const EFK_CHAR* path ) {
			return nullptr;
		}
	};
}