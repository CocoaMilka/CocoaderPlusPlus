#pragma once
#include <string>

#ifdef COCOADER_EXPORTS
#define COCOADER_API __declspec(dllexport)
#else
#define COCOADER_API __declspec(dllimport)
#endif

extern "C" COCOADER_API int initializeUDPVideoStream(const char* streamUrl);

extern "C" COCOADER_API void startStream();

extern "C" COCOADER_API void stopStream();

extern "C" COCOADER_API void getRawImageBytes(unsigned char* data, int width, int height);

extern "C" COCOADER_API void setLogFilePath(const char* path);

// Function defined in Unity to pass logs
extern "C" void UnityLog(const char* message);
