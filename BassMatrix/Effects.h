#include <algorithm>  // For std::min, std::clamp
#include <utility>    // For std::pair, std::make_pair
#include <cstring>    // For memory operations
#include <cmath>      // For mathematical operations

// From Amazon Q
class AcidStereoDelay
{
private:
  static const int MAX_DELAY_LENGTH = 192000;  // 4 seconds at 48kHz
  double *delayBufferL;
  double *delayBufferR;
  int writeIndex;
  int delayLengthL;
  int delayLengthR;
  double feedback;
  double dryMix;
  double wetMix;
  double sampleRate;

  // Simple low-pass filter for delay feedback
  double filterStateL;
  double filterStateR;
  double filterCoeff;

public:
  AcidStereoDelay(double sampleRate = 44100.0f);
  ~AcidStereoDelay();
  void setDelayTime(double timeMs);
  void setFeedback(double fb);
  void setWetDryMix(double wet, double dry);

  // Process a single mono sample and return stereo output
  std::pair<double, double> process(double input);
};

// Simple reverb to complement the delay
class SimpleAcidReverb
{
private:
  static const int NUM_DELAYS = 8;
  double *delays[NUM_DELAYS];
  int delayLengths[NUM_DELAYS];
  int writeIndices[NUM_DELAYS];
  double feedback;
  double wetMix;
  static const int MAX_REVERB_LENGTH = 8192;

public:
  SimpleAcidReverb();
  ~SimpleAcidReverb();
  std::pair<double, double> process(double input);
};

std::pair<double, double> processDelayReverbAudioBlock(double sampleRate, double input);
