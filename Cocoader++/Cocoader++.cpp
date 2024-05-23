#include "pch.h"
#include "Cocoader++.h"

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <stdexcept>

// Global VideoCapture object
cv::VideoCapture camera;
std::atomic<bool> isStreaming(false);

void initializeUDPVideoStream(const std::string& streamUrl) 
{
    // Try to open the video stream
    if (!camera.open(streamUrl)) 
    {
        std::cerr << "Failed to open video stream: " << streamUrl << std::endl;
    }
    else 
    {
        isStreaming.store(true);
    }
}

void startStream() 
{
    if (!isStreaming.load() && !camera.isOpened()) 
    {
        std::cerr << "Stream is not initialized or already running." << std::endl;
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
    }
}

void getRawImageBytes(unsigned char* data, int width, int height) 
{
    cv::Mat _currentFrame;

    // Check if the camera is opened properly
    if (!camera.isOpened()) 
    {
        std::cerr << "Camera stream is not open." << std::endl;
        return;
    }

    // Read the current frame from the UDP stream
    if (!camera.read(_currentFrame) || _currentFrame.empty()) 
    {
        std::cerr << "Failed to read frame or frame is empty." << std::endl;
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