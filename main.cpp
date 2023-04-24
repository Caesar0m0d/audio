#include <iostream>
#include <stdio.h>
#include <vector>
#include "portaudio.h"
#pragma warning(disable:4996)
#define _CRT_SECURE_NO_WARNINGS


constexpr auto FRAMES_PER_BUFFER(30512);
constexpr auto NUM_SECONDS(2);

int maxx=0;

typedef struct {
    float* buffer;
    unsigned long framesPerBuffer;
    unsigned long bufferPosition;
} paTestData;

int inputCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags, void* userData)
{
    paTestData* data = (paTestData*)userData;
    const float* inputData = (const float*)inputBuffer;

    // Copy the input data to the circular buffer
    for (unsigned long i = 0; i < framesPerBuffer; i++)
    {
        data->buffer[data->bufferPosition++] = inputData[i];
        if (maxx < data->bufferPosition)
        {
            maxx = data->bufferPosition;
        }
        
        if (data->bufferPosition >= data->framesPerBuffer)
            data->bufferPosition = 0;
    }

    return paContinue;
}

int main()
{
    PaError err;
    const PaDeviceInfo* info;
    PaStream* stream = NULL;
    paTestData data = { 0, };
    std::vector<const PaDeviceInfo*> inputDevices;
    int j=0;

    // Initialize PortAudio
    err = Pa_Initialize();
    if (err != paNoError)
    {
        std::cout << "Error initializing PortAudio: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    // Print the list of available audio devices
    int numDevices = Pa_GetDeviceCount();
    for (int i = 0; i < numDevices; i++)
    {
        info = Pa_GetDeviceInfo(i);
        //std::cout << "Device " << i << ": " << info->name << " (max input channels: " << info->maxInputChannels << ")" << std::endl;
        if (info->maxInputChannels > 0) {
            inputDevices.push_back(info);
            std::cout << "Device " << j << ": " << info->name << " (max input channels: " << info->maxInputChannels << ")" << std::endl;
            j++;
        }
    }

    if (inputDevices.empty())
    {
        std::cout << "Error: no audio input devices found" << std::endl;
        return -1;
    }
    int deviceIndex = 0;  // Select the first input device by default
    
    if (inputDevices.size() > 1)
    {
        std::cout << "Select an audio input device (enter device index): ";
        std::cin >> deviceIndex;
        if (deviceIndex < 0 || deviceIndex >= inputDevices.size())
        {
            std::cout << "Error: invalid device index" << std::endl;
            return -1;
        }
        else
        {
            printf("no. %d is chosen\n", deviceIndex);
        }
    }
    info = inputDevices[deviceIndex];

    // Configure the input stream
    PaStreamParameters inputParams = { 0, };
    inputParams.device = deviceIndex;
    inputParams.channelCount = 1;  // Mono input
    inputParams.sampleFormat = paFloat32;  // 32-bit floating-point input
    inputParams.suggestedLatency = info->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = NULL;

    // Allocate the circular buffer
    data.framesPerBuffer = FRAMES_PER_BUFFER;
    data.buffer = new float[data.framesPerBuffer];
    data.bufferPosition = 0;

    // Open the input stream
    err = Pa_OpenStream(&stream, &inputParams, NULL, info->defaultSampleRate,
        data.framesPerBuffer, paNoFlag, inputCallback, &data);
    if (err != paNoError)
    {
        std::cout << "Error opening stream: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    // Start the stream
    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        std::cout << "Error starting stream: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    Pa_Sleep(NUM_SECONDS * 1000);

    err = Pa_StopStream(stream);
    if (err != paNoError)
    {
        std::cout << "Error stopping stream: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError)
    {
        std::cout << "Error closing stream: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }


    // Print the recorded audio data
    std::cout << "Recorded audio data:" << std::endl;

    for (unsigned long i = 0; i < data.framesPerBuffer; i++)
    {
        std::cout << data.buffer[i] << " ";
    }
    std::cout << std::endl;

    // Save the recorded audio data to a file
    if (data.bufferPosition > 0 || maxx != 0)
    {
        FILE* fp;
        fp = fopen("recorded_audio.pcm", "wb");
        //fwrite(data.buffer, sizeof(float), data.bufferPosition * inputParams.channelCount, fp);
        fwrite(data.buffer, sizeof(float), maxx* inputParams.channelCount, fp);
        fclose(fp);
    }
    else
    {
        std::cout << "No audio data was recorded." << std::endl;
    }
    printf("size %d and buf pos %d\n", maxx, data.bufferPosition);

    // Free the circular buffer
    delete[] data.buffer;

    // Terminate PortAudio
    Pa_Terminate();

    return 0;
}