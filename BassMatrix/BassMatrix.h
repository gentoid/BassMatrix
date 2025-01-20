#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
#include "open303/Source/DSPCode/rosic_Open303.h"
#include "IPlugProcessor.h"
#include "IPlugEditorDelegate.h"
#include "IPlugAPIBase.h"

const int kNumPresets = 1;
const int kNumberOfNoteBtns = 13;
const int kNumberOfPropertyBtns = 5;
const int kNumberOfTotalPropButtons = kNumberOfPropertyBtns * 16;
const int kNumberOfSeqButtons = kNumberOfNoteBtns * 16 + kNumberOfPropertyBtns * 16;
const int kNumberOfPatterns = 24;

enum EParams
{
  // First the parameters that is not saved.
  kLedBtn0 = 0,
  kBtnSeq0 = kLedBtn0 + 16,
  kBtnProp0 = kBtnSeq0 + 16 * kNumberOfNoteBtns,

  // Parameters that are saved
  kBtnPtnC = kBtnProp0 + 16 * kNumberOfPropertyBtns,
  kBtnPtnOct2 = kBtnPtnC + 12,
  kBtnPtnOct3,

  kParamCutOff,
  kParamResonance,
  kParamWaveForm,
  kParamTuning,
  kParamEnvMode,
  kParamDecay,
  kParamAccent,
  kParamVolume,
  kParamTempo,
  kParamDrive,
  kParamStop,
  kParamHostSync,
  kParamKeySync,
  kParamInternalSync,
  kParamMidiPlay,
  kKnobLoopSize,
  kParamCopy,
  kParamClear,
  kParamRandomize,
  kParamEffects,

  kNumParams
};

enum ECtrlTags
{
  kCtrlTagVersionNumber = 0,
  kCtrlTagBtnSeq0,
  kCtrlTagBtnProp0 = kCtrlTagBtnSeq0 + 16 * kNumberOfNoteBtns,
  kCtrlTagLedSeq0 = kCtrlTagBtnProp0 + 16 * kNumberOfPropertyBtns,
  kCtrlTagBtnPtnC = kCtrlTagLedSeq0 + 16,
  kCtrlTagBtnPtnOct2 = kCtrlTagBtnPtnC + 12,
  kCtrlTagBtnPtnOct3,
  kCtrlTagStop,
  kCtrlWaveForm,
  kCtrlTagHostSync,
  kCtrlTagKeySync,
  kCtrlTagInternalSync,
  kCtrlTagMidiPlay,
  kCtrlEffects,

  kNumCtrlTags
};

using namespace iplug;
using namespace igraphics;

class BassMatrix final : public Plugin
{
public:
  BassMatrix(const InstanceInfo &info);
  virtual ~BassMatrix();

#if IPLUG_EDITOR
  //  void OnParentWindowResize(int width, int height) override;
  bool OnHostRequestingSupportedViewConfiguration(int width, int height) override { return true; }
#if defined VST3_API
  bool SerializeState(IByteChunk &chunk) const override;
  int UnserializeState(const IByteChunk &chunk, int startPos) override;
  IGraphics *CreateGraphics() override;
#endif  // API
#endif  // IPLUG_EDITOR

#if IPLUG_DSP
  void ProcessMidiMsg(const IMidiMsg &msg) override;
  void OnReset() override;
#if defined VST3_API
  void OnParamChangeUI(int paramIdx, EParamSource source = kUnknown) override;
#else
  void OnParamChange(int paramIdx) override;
#endif  // VST3_API
  void OnIdle() override;
  bool OnMessage(int msgTag, int ctrlTag, int dataSize, const void *pData) override;
#endif  // IPLUG_DSP
  static std::array<bool, kNumberOfSeqButtons>
  CollectSequenceButtons(rosic::Open303 &open303Core, int patternNr = -1);
#if IPLUG_DSP  // http://bit.ly/2S64BDd
  void ProcessBlock(PLUG_SAMPLE_DST **inputs, PLUG_SAMPLE_DST **outputs, int nFrames) override;
#endif

protected:
  IMidiQueue mMidiQueue;

private:
  // the embedded core dsp object:
  rosic::Open303 open303Core;
  ISender<1, 1, int> mLedSeqSender;
  ISender<1, 1, std::array<bool, kNumberOfSeqButtons>> mSequencerSender;
  ISender<1, 1, int> mPatternSender;
  unsigned int mLastSamplePos;
  bool mStartSyncWithHost;
  int mKnobLoopSize;
  int mCurrentPattern;
  bool mHasChanged;
  double mPlugUIScale;
  bool mUseEffects;
};


//// ChatGpt mono
//class AcidDelayReverb
//{
//public:
//  AcidDelayReverb(double sampleRate, double delayTimeMs, double feedback, double wetLevel) :
//    sampleRate(sampleRate),
//    feedback(feedback),
//    wetLevel(wetLevel)
//  {
//    setDelayTime(delayTimeMs);
//  }
//
//  void setDelayTime(double delayTimeMs)
//  {
//    delayTimeSamples = static_cast<int>((delayTimeMs / 1000.0f) * sampleRate);
//    buffer.resize(delayTimeSamples + 1, 0.0f);
//    writeIndex = 0;
//    readIndex = 0;
//  }
//
//  void setFeedback(double newFeedback) { feedback = std::clamp(newFeedback, 0.0f, 0.99f); }
//
//  void setWetLevel(double newWetLevel) { wetLevel = std::clamp(newWetLevel, 0.0f, 1.0f); }
//
//  double processSample(double input)
//  {
//    // Read the delayed sample
//    double delayedSample = buffer[readIndex];
//
//    // Calculate the output (dry + wet mix)
//    double output = (1.0f - wetLevel) * input + wetLevel * delayedSample;
//
//    // Write the input + feedback to the buffer
//    buffer[writeIndex] = input + delayedSample * feedback;
//
//    // Increment and wrap indices
//    incrementIndices();
//
//    return output;
//  }
//
//private:
//  double sampleRate;
//  double feedback;
//  double wetLevel;
//  int delayTimeSamples;
//  std::vector<double> buffer;
//  int writeIndex;
//  int readIndex;
//
//  void incrementIndices()
//  {
//    writeIndex = (writeIndex + 1) % buffer.size();
//    readIndex = (readIndex + 1) % buffer.size();
//  }
//};
//
//int
//main()
//{
//  const double sampleRate = 44100.0f;  // Standard audio sample rate
//  const double delayTimeMs = 300.0f;   // 300ms delay
//  const double feedback = 0.7f;        // Feedback for reverb-like effect
//  const double wetLevel = 0.5f;        // 50% wet/dry mix
//
//  AcidDelayReverb delayReverb(GetSamplerRate(), delayTimeMs, feedback, wetLevel);
//  delayReverb.processSample(inputSample);
//
//  return 0;
//}

// ChatGpt Stereo
//#include <iostream>
//#include <vector>
//#include <cmath>
//
//class AcidDelayReverb
//{
//public:
//  AcidDelayReverb(float sampleRate, float delayTimeMs, float feedback, float wetLevel) :
//    sampleRate(sampleRate),
//    feedback(feedback),
//    wetLevel(wetLevel)
//  {
//    setDelayTime(delayTimeMs);
//  }
//
//  void setDelayTime(float delayTimeMs)
//  {
//    delayTimeSamples = static_cast<int>((delayTimeMs / 1000.0f) * sampleRate);
//    buffer.resize(delayTimeSamples + 1, 0.0f);
//    writeIndex = 0;
//    readIndex = 0;
//  }
//
//  void setFeedback(float newFeedback) { feedback = std::clamp(newFeedback, 0.0f, 0.99f); }
//
//  void setWetLevel(float newWetLevel) { wetLevel = std::clamp(newWetLevel, 0.0f, 1.0f); }
//
//  float processSample(float input)
//  {
//    // Read the delayed sample
//    float delayedSample = buffer[readIndex];
//
//    // Calculate the output (dry + wet mix)
//    float output = (1.0f - wetLevel) * input + wetLevel * delayedSample;
//
//    // Write the input + feedback to the buffer
//    buffer[writeIndex] = input + delayedSample * feedback;
//
//    // Increment and wrap indices
//    incrementIndices();
//
//    return output;
//  }
//
//private:
//  float sampleRate;
//  float feedback;
//  float wetLevel;
//  int delayTimeSamples;
//  std::vector<float> buffer;
//  int writeIndex;
//  int readIndex;
//
//  void incrementIndices()
//  {
//    writeIndex = (writeIndex + 1) % buffer.size();
//    readIndex = (readIndex + 1) % buffer.size();
//  }
//};
//
//int
//main()
//{
//  const float sampleRate = 44100.0f;  // Standard audio sample rate
//  const float delayTimeMs = 300.0f;   // 300ms delay
//  const float feedback = 0.7f;        // Feedback for reverb-like effect
//  const float wetLevel = 0.5f;        // 50% wet/dry mix
//
//  AcidDelayReverb delayReverb(sampleRate, delayTimeMs, feedback, wetLevel);
//
//  // Example processing: a single sine wave
//  const int numSamples = 44100;  // 1 second of audio
//  float frequency = 440.0f;      // A4 note
//  float amplitude = 0.5f;
//
//  for (int i = 0; i < numSamples; ++i)
//  {
//    float inputSample = amplitude * sinf(2.0f * M_PI * frequency * i / sampleRate);
//    float outputSample = delayReverb.processSample(inputSample);
//
//    // Send the output to your audio playback or save it
//    std::cout << outputSample << "\n";
//  }
//
//  return 0;
//}
