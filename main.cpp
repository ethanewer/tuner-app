#include <iostream>
#include <string>
#include <algorithm>
#include <numbers>
#include <cmath>
#include <complex>
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioQueue.h>

#define SAMPLE_RATE 44100.0
#define BUF_COUNT 8
#define BUF_SIZE 8192
#define MAX_FREQ 1760.0
#define KERNAL_SIZE 128
#define A_FREQ (441.0 / 8)

using namespace std::complex_literals;

std::string note_names[] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};

std::complex<double> buf[BUF_SIZE];
std::complex<double> out[BUF_SIZE];

void fft(std::complex<double>* buf_ptr, std::complex<double>* out_ptr, int step) {
	if (step < BUF_SIZE) {
		fft(out_ptr, buf_ptr, step * 2);
		fft(out_ptr + step, buf_ptr + step, step * 2);

		for (int k = 0; k < BUF_SIZE; k += 2 * step) {
			std::complex<double> t = std::exp(-1i * (std::numbers::pi * k / BUF_SIZE)) * out_ptr[k + step];
      buf_ptr[k / 2] = out_ptr[k] + t;
			buf_ptr[(k + BUF_SIZE) / 2] = out_ptr[k] - t;
		}
	}
}

void tune(int16_t* data, int data_size) {
  for (int i = 0; i < BUF_SIZE; i++) {
    buf[i] = i < data_size ? static_cast<std::complex<double>>(data[i]) : 0.0;
    out[i] = i < data_size ? static_cast<std::complex<double>>(data[i]) : 0.0;
  }

  fft(buf, out, 1);
  
  double max_val = 0;
  int max_idx = 0;
  for (int i = 0; i < std::ceil(MAX_FREQ * BUF_SIZE / SAMPLE_RATE); i++) {
    double val = std::abs(buf[i]);
    if (val > max_val) {
      max_val = val;
      max_idx = i;
    }
  }
  
  double strongest_freq = SAMPLE_RATE * max_idx / BUF_SIZE;
  double real_note_num = 12.0 * std::log2(strongest_freq / A_FREQ);
  int note_num = std::round(real_note_num);
  int cents = std::round(100 * (real_note_num - note_num));
  std::string note_name = note_names[note_num % 12];

  std::cout << strongest_freq << ' ' << note_name << ' ' << cents << '\n';
}

void AudioInputCallback(
  void *inUserData, 
  AudioQueueRef inAQ, 
  AudioQueueBufferRef inBuffer, 
  const AudioTimeStamp *inStartTime, 
  UInt32 inNumPackets, 
  const AudioStreamPacketDescription *inPacketDesc
) {
  tune(static_cast<int16_t*>(inBuffer->mAudioData), inBuffer->mAudioDataByteSize / sizeof(int16_t));
  AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, nullptr);
}

int main() {
  AudioQueueRef audioQueue;
  AudioStreamBasicDescription format;
  format.mSampleRate = SAMPLE_RATE;
  format.mFormatID = kAudioFormatLinearPCM;
  format.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
  format.mFramesPerPacket = 1;
  format.mChannelsPerFrame = 1;
  format.mBytesPerFrame = 2;
  format.mBytesPerPacket = 2;
  format.mBitsPerChannel = 16;

  AudioQueueNewInput(&format, AudioInputCallback, nullptr, nullptr, kCFRunLoopCommonModes, 0, &audioQueue);

  AudioQueueBufferRef audioBuffers[BUF_COUNT];
  for (int i = 0; i < BUF_COUNT; i++) {
    AudioQueueAllocateBuffer(audioQueue, BUF_SIZE * sizeof(int16_t), &audioBuffers[i]);
    AudioQueueEnqueueBuffer(audioQueue, audioBuffers[i], 0, nullptr);
  }

  AudioQueueStart(audioQueue, nullptr);

  std::cin.get();
 
  // AudioQueueStop(audioQueue, true);
  // AudioQueueDispose(audioQueue, true);

  return 0;
}
