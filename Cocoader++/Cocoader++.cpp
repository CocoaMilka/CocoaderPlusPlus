#include "pch.h"
#include "Cocoader++.h"

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <stdexcept>

#include <fstream>
#include <chrono>
#include <ctime>

// Global VideoCapture object
cv::VideoCapture camera;
std::atomic<bool> isStreaming(false);
std::string g_logFilePath = "cocoader_log.txt";  // Default log file path

enum ErrorCode
{
    Success = 0,
    CannotOpenStream = 1,
    StreamAlreadyRunning = 2,
    FailedToReadFrame = 3
};

int initializeUDPVideoStream(const char* streamUrl) 
{
    camera.open(streamUrl);
    //camera.open(0); camera works but not stream >.>

    if (!camera.isOpened())
    {
        UnityLog("Failed to open video stream.");
        return CannotOpenStream;
    }
    else
    {
        UnityLog("Stream is open and ready!");
        isStreaming.store(true);
        return Success;
    }
}

void startStream()
{
    if (isStreaming.load())
    {
        //std::cerr << "Stream is already running." << std::endl;
        UnityLog("Stream is already running");
        return;
    }
    if (!camera.isOpened())
    {
        //std::cerr << "Stream is not initialized." << std::endl;
        UnityLog("Stream is not initialized");
        return;
    }

    isStreaming.store(true);
}

void stopStream() 
{
    if (isStreaming.load()) 
    {
        isStreaming.store(false);
        camera.release();
        UnityLog("Stream disabled");
    }
}

void getRawImageBytes(unsigned char* data, int width, int height) 
{
    cv::Mat _currentFrame;

    // Check if the camera is opened properly
    if (!camera.isOpened()) 
    {
        //std::cerr << "Camera stream is not open." << std::endl;
        UnityLog("Camera stream is not open");
        return;
    }

    // Read the current frame from the UDP stream
    if (!camera.read(_currentFrame) || _currentFrame.empty()) 
    {
        //std::cerr << "Failed to read frame or frame is empty." << std::endl;
        UnityLog("Failed to read frame or frame is empty.");
        return;
    }

    // Resize the Mat to match the array size passed from C#
    cv::Mat resizedMat(height, width, _currentFrame.type());
    cv::resize(_currentFrame, resizedMat, resizedMat.size(), cv::INTER_CUBIC);

    // Convert from BGR (OpenCV default) to ARGB (as expected by Unity)
    cv::Mat argb_img;
    cv::cvtColor(resizedMat, argb_img, cv::COLOR_BGR2BGRA);

    // Rearrange the channels: BGRA to ARGB
    std::vector<cv::Mat> bgra_channels;
    cv::split(argb_img, bgra_channels);
    std::swap(bgra_channels[0], bgra_channels[3]);  // Swap B and A
    std::swap(bgra_channels[1], bgra_channels[2]);  // Swap G and R

    // Merge back to a single ARGB image
    cv::merge(bgra_channels, argb_img);

    // Copy the processed image data to the provided data pointer
    std::memcpy(data, argb_img.data, argb_img.total() * argb_img.elemSize());
}

void setLogFilePath(const char* path)
{
    if (path != nullptr) {
        g_logFilePath = std::string(path) + "/cocoader_log.txt";  // Update the global log file path
    }
    else {
        std::cerr << "Invalid log file path provided." << std::endl;
    }
}


void UnityLog(const char* message) {
    // Open a log file in append mode
    std::ofstream logFile(g_logFilePath, std::ios::app);

    if (!logFile.is_open()) 
    {
        std::cerr << "Failed to open log file." << std::endl;
        return;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    struct tm ptm;
    localtime_s(&ptm, &now_time);

    char buffer[32];
    // Format: YYYY-MM-DD HH:MM:SS
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &ptm);

    // Write the formatted time and message to the file
    logFile << buffer << " - " << message << std::endl;

    logFile.close();
}

