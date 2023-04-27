#include <iostream>
#include <cmath>
#include "portaudio.h"

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (1024)
//#define REF_LVL (32767.0)
#define REF_LVL (1.0)
//#define OFFSET (1e-10)

using namespace std;

float ref_val = 1.0; //ref lvl for dB calcul
float rms = 0.0;
float OFFSET = (1e-10);

int paCallback	(const void* inputBuffer, 
		void* outputBuffer,
               	unsigned long framesPerBuffer,
               	const PaStreamCallbackTimeInfo* timeInfo,
               	PaStreamCallbackFlags statusFlags,
               	void* userData
		) {

    // Cast input and output buffers
    unsigned int i;
    rms = 0.0;
    float dB;//x is sum of buffer val
    const float* in = (const float*)inputBuffer;
    float* out = (float*)outputBuffer;
    
    for(i = 0; i < framesPerBuffer; i++) {
       float x = *(in+i);
       rms += x * x;
    }
    rms = sqrt(rms / framesPerBuffer);
    //dB = 20 * log10(rms / ref_val);
    dB = 20 * log10(rms / REF_LVL +1);
    cout<<"dB: "<<dB << endl;
    // Pass through input buffer to output buffer
    for(i = 0; i < framesPerBuffer; i++) {
        //*out++ = *in++;
           *out++ = 0.0;
    }
    
    return paContinue;
}

int main() {
    PaStream* stream;
    PaError err;
    
    // Initialize PortAudio
    err = Pa_Initialize();
    if(err != paNoError) {
        cout << "Error: PortAudio initialization failed." << endl;
        return -1;
    }
    
    // Open stream with default input and output devices
    err = Pa_OpenDefaultStream(&stream, 1, 1, paFloat32,
                               SAMPLE_RATE, FRAMES_PER_BUFFER,
                               paCallback, NULL);
    if(err != paNoError) {
        cout << "Error: PortAudio stream opening failed." << endl;
        Pa_Terminate();
        return -1;
    }
    
    // Start the stream
    
    err = Pa_StartStream(stream);
    if(err != paNoError) {
        cout << "Error: PortAudio stream starting failed." << endl;
        Pa_Terminate();
        return -1;
    }
    
    // Wait for user to press Enter key
    cout << "Press Enter key to stop recording and playback." << endl;
    cin.get();
    
    // Stop and close the stream

    err = Pa_StopStream(stream);
    if(err != paNoError) {
        cout << "Error: PortAudio stream stopping failed." << endl;
        Pa_Terminate();
        return -1;
    }

    err = Pa_CloseStream(stream);
    if(err != paNoError) {
        cout << "Error: PortAudio stream closing failed." << endl;
        Pa_Terminate();
        return -1;
    }
    
    // Terminate PortAudio
    Pa_Terminate();
    return 0;
}
