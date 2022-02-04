#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
#include "open303/Source/DSPCode/rosic_Open303.h"
#include "IPlugProcessor.h"
#include "IPlugEditorDelegate.h"
#include "IPlugAPIBase.h"

const int kNumPresets = 1;
const int kNumberOfPropButtons = 5 * 16;
const int kNumberOfNoteBtns = 12;
const int kNumberOfSeqButtons = kNumberOfNoteBtns * 16 + kNumberOfPropButtons;

enum EParams
{
	kParamCutOff = 0,
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

	kBtnSeq0,

	kBtnProp0 = kBtnSeq0 + 16 * kNumberOfNoteBtns,

	kLedBtn0 = kBtnProp0 + kNumberOfPropButtons,

	kBtnPtnC = kLedBtn0 + 16,
	kBtnPtnOct2 = kBtnPtnC + 12,
	kBtnPtnOct3,
	kKnobLoopSize,
	kParamCopy,
	kParamClear,
	kParamRandomize,

	kNumParams
};

enum ECtrlTags
{
  kCtrlTagVersionNumber = 0,
  kCtrlTagBtnSeq0,
  kCtrlTagBtnProp0   = kCtrlTagBtnSeq0 + 16 * kNumberOfNoteBtns,
  kCtrlTagLedSeq0    = kCtrlTagBtnProp0 + 16 * 5,
  kCtrlTagBtnPtnC    = kCtrlTagLedSeq0 + 16,
  kCtrlTagBtnPtnOct2 = kCtrlTagBtnPtnC + 12,
  kCtrlTagBtnPtnOct3,
	kCtrlTagStop,
  kCtrlTagHostSync,
  kCtrlTagKeySync,
  kCtrlTagInternalSync,
  kCtrlTagMidiPlay,

  kNumCtrlTags
};

using namespace iplug;
using namespace igraphics;

class BassMatrix final : public Plugin
{
public:
  BassMatrix(const InstanceInfo& info);

#if IPLUG_EDITOR
//  void OnParentWindowResize(int width, int height) override;
  bool OnHostRequestingSupportedViewConfiguration(int width, int height) override { return true; }
#endif

#if IPLUG_DSP
	void ProcessMidiMsg(const IMidiMsg& msg) override;
	void OnReset() override;
	void OnParamChange(int paramIdx) override;
	void OnIdle() override;
	bool OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) override;
#endif
	static std::array<bool, kNumberOfSeqButtons> CollectSequenceButtons(rosic::Open303& open303Core);
#if IPLUG_DSP // http://bit.ly/2S64BDd
	void ProcessBlock(PLUG_SAMPLE_DST** inputs, PLUG_SAMPLE_DST** outputs, int nFrames) override;
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
	int mHasChanged;
};
