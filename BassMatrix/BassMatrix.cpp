#include "BassMatrix.h"
#include "IPlug_include_in_plug_src.h"
#include "BassMatrixControls.h"
#include "open303/Source/DSPCode/rosic_Open303.h"

#if IPLUG_EDITOR
#include "IControls.h"
#endif

BassMatrix::BassMatrix(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets)), mLastSamplePos(0), mStartSyncWithHost(false), mCurrentPattern(0)
{
  GetParam(kParamCutOff)->InitDouble("Cut off", 500.0, 314.0, 2394.0, 1.0, "Hz");
  GetParam(kParamResonance)->InitDouble("Resonace", 50.0, 0.0, 100.0, 1.0, "%");
  GetParam(kParamWaveForm)->InitDouble("Waveform", 0.0, 0.0, 1.0, 0.1, "|\\|\\ |_|_%");
  GetParam(kParamTuning)->InitDouble("Tuning", 440.0, 400.0, 480.0, 1.0, "%");
  GetParam(kParamEnvMode)->InitDouble("Env mode", 25.0, 0.0, 100.0, 1.0, "%");
  GetParam(kParamDecay)->InitDouble("Decay", 400.0, 200.0, 2000.0, 1.0, "ms");
  GetParam(kParamAccent)->InitDouble("Accent", 50.0, 0.0, 100.0, 1.0, "%");
  GetParam(kParamVolume)->InitDouble("Volume", -17.0, -100.0, 0.0, 0.1, "dB");
  GetParam(kParamTempo)->InitDouble("Tempo", 120.0, 0.0, 300.0, 1.0, "bpm");
  GetParam(kParamDrive)->InitDouble("Drive", 36.9, 0.0, 50.0, 1.0, "bpm");

  GetParam(kParamStop)->InitBool("Stop", false);
  GetParam(kParamHostSync)->InitBool("Host Sync", false);
  GetParam(kParamKeySync)->InitBool("Key Sync", false);
#ifdef VST3_API
  GetParam(kParamInternalSync)->InitBool("Internal Sync", false);
  GetParam(kParamMidiPlay)->InitBool("Midi Play", true);
#else
  GetParam(kParamInternalSync)->InitBool("Internal Sync", true);
  GetParam(kParamMidiPlay)->InitBool("Midi Play", false);
#endif

  // This value set here have not so much relevance, since we tell the sequencer to
  // randomize the current pattern and then tells the gui to update.
  for (int i = kBtnSeq0; i < kBtnSeq0 + kNumberOfSeqButtons; ++i)
  {
    GetParam(i)->InitBool(("Sequencer button " + std::to_string(i - kBtnSeq0)).c_str(), (i - kBtnSeq0) / 16 == 5 || (i - kBtnSeq0) / 16 == 16);
  }

  for (int i = kBtnPtnC; i < kBtnPtnC + 12; ++i)
  {
    GetParam(i)->InitBool(("Pattern button" + std::to_string(i - kBtnPtnC)).c_str(), i == kBtnPtnC);
  }

  GetParam(kBtnPtnOct2)->InitBool("Octav 2", true);
  GetParam(kBtnPtnOct3)->InitBool("Octav 3", false);

  GetParam(kKnobLoopSize)->InitInt("Loop size", 1, 1, 24);

  GetParam(kParamCopy)->InitBool("Pattern copy", false);
  GetParam(kParamClear)->InitBool("Pattern clear", false);
  GetParam(kParamRandomize)->InitBool("Pattern randomize", false);


#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    const IRECT bounds = pGraphics->GetBounds();
    const IRECT innerBounds = bounds.GetPadded(-10.f);
    const IRECT sliderBounds = innerBounds.GetFromLeft(150).GetMidVPadded(100);
    const IRECT versionBounds = innerBounds.GetFromTRHC(300, 20);
    const IRECT titleBounds = innerBounds.GetCentredInside(200, 50);

//    if (pGraphics->NControls()) {
//      pGraphics->GetBackgroundControl()->SetTargetAndDrawRECTs(bounds);
//      return;
//    }

//    pGraphics->SetLayoutOnResize(true);
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
//    pGraphics->AttachPanelBackground(COLOR_RED);
    pGraphics->LoadBitmap(BACKGROUND_FN, 1, true);
    pGraphics->AttachBackground(BACKGROUND_FN);

    const IBitmap knobRotateBitmap = pGraphics->LoadBitmap(PNG6062_FN, 127);
    const IBitmap knobLittleBitmap = pGraphics->LoadBitmap(PNGFX1LITTLE_FN, 127);
    const IBitmap knobBigBitmap = pGraphics->LoadBitmap(PNGFX1BIG_FN, 61);
    pGraphics->AttachControl(new IBKnobControl(210, 30, knobLittleBitmap, kParamWaveForm));
    pGraphics->AttachControl(new IBKnobControl(310, 30, knobLittleBitmap, kParamTuning));
    pGraphics->AttachControl(new IBKnobControl(410, 30, knobLittleBitmap, kParamCutOff));
    pGraphics->AttachControl(new IBKnobControl(510, 30, knobLittleBitmap, kParamResonance));
    pGraphics->AttachControl(new IBKnobControl(610, 30, knobLittleBitmap, kParamEnvMode));
    pGraphics->AttachControl(new IBKnobControl(710, 30, knobLittleBitmap, kParamDecay));
    pGraphics->AttachControl(new IBKnobControl(810, 30, knobLittleBitmap, kParamAccent));

    pGraphics->AttachControl(new IBKnobControl(0 +  210 - 175, 130, knobBigBitmap, kParamTempo));
//    pGraphics->AttachControl(new IBKnobControl(510, 130, knobBigBitmap, kParamDrive));
    pGraphics->AttachControl(new IBKnobControl(1130 - 210, 130, knobBigBitmap, kParamVolume));

    // Led buttons
    const IBitmap ledBtnBitmap = pGraphics->LoadBitmap(PNGBTNLED_FN, 2, true);
    for (int i = 0; i < 16; i++)
    {
        pGraphics->AttachControl(new SeqLedBtnControl(120.f + i * (ledBtnBitmap.W() / 2 + 6), 310.f, ledBtnBitmap, kLedBtn0 + i, open303Core), kCtrlTagLedSeq0 + i, "Sequencer");
    }

    // Sequencer tones buttons
    const IBitmap btnSeqBitmap = pGraphics->LoadBitmap(PNGBTNSEQ_FN, 2, true);
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < kNumberOfNoteBtns; j++)
        {
            pGraphics->AttachControl(new SeqNoteBtnControl(130.f + i * (btnSeqBitmap.W() / 2 + 26), 380.f + j * (btnSeqBitmap.H() + 1),
                btnSeqBitmap, kBtnSeq0 + 16 * j + i),
                kCtrlTagBtnSeq0 + 16 * j + i, "Sequencer");
        }
    }

    // Properties buttons
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            pGraphics->AttachControl(new SeqNoteBtnControl(130.f + i * (btnSeqBitmap.W() / 2 + 26), 660.f + j * (btnSeqBitmap.H() + 1),
                btnSeqBitmap, kBtnProp0 + 16 * j + i),
                kCtrlTagBtnProp0 + 16 * j + i, "Sequencer");
        }
    }

    const IBitmap btnStopBitmap = pGraphics->LoadBitmap(PNGSTOP_FN, 2, true);
    pGraphics->AttachControl(new SyncBtnControl(317, 790, btnStopBitmap, kParamStop, kCtrlTagStop), kCtrlTagStop);
    const IBitmap btnHostSyncBitmap = pGraphics->LoadBitmap(PNGHOSTSYNC_FN, 2, true);
    pGraphics->AttachControl(new SyncBtnControl(417, 790, btnHostSyncBitmap, kParamHostSync, kCtrlTagHostSync), kCtrlTagHostSync);
    const IBitmap btnKeySyncBitmap = pGraphics->LoadBitmap(PNGKEYSYNC_FN, 2, true);
    pGraphics->AttachControl(new SyncBtnControl(517, 790, btnKeySyncBitmap, kParamKeySync, kCtrlTagKeySync), kCtrlTagKeySync);
    const IBitmap btnInternalSyncBitmap = pGraphics->LoadBitmap(PNGINTERNALSYNC_FN, 2, true);
    pGraphics->AttachControl(new SyncBtnControl(617, 790, btnInternalSyncBitmap, kParamInternalSync, kCtrlTagInternalSync), kCtrlTagInternalSync);
    const IBitmap btnMidiPlayBitmap = pGraphics->LoadBitmap(PNGMIDIPLAY_FN, 2, true);
    pGraphics->AttachControl(new SyncBtnControl(717, 790, btnMidiPlayBitmap, kParamMidiPlay, kCtrlTagMidiPlay), kCtrlTagMidiPlay);

    // Pattern controls
    const IBitmap btnPatternOctav2Bitmap = pGraphics->LoadBitmap(PNGBTNPATOCTAV2_FN, 2, true);
    pGraphics->AttachControl(new PatternBtnControl(485, 160, btnPatternOctav2Bitmap, kBtnPtnOct2, kCtrlTagBtnPtnOct2, open303Core), kCtrlTagBtnPtnOct2);
    const IBitmap btnPatternOctav3Bitmap = pGraphics->LoadBitmap(PNGBTNPATOCTAV3_FN, 2, true);
    pGraphics->AttachControl(new PatternBtnControl(485.f + btnPatternOctav2Bitmap.FW() + 10, 160, btnPatternOctav3Bitmap, kBtnPtnOct3, kCtrlTagBtnPtnOct3, open303Core), kCtrlTagBtnPtnOct3);
    IBitmap btnPatternBitmap[12] ;
    btnPatternBitmap[0] = pGraphics->LoadBitmap(PNGBTNPATC_FN, 2, true);
    btnPatternBitmap[1] = pGraphics->LoadBitmap(PNGBTNPATCc_FN, 2, true);
    btnPatternBitmap[2] = pGraphics->LoadBitmap(PNGBTNPATD_FN, 2, true);
    btnPatternBitmap[3] = pGraphics->LoadBitmap(PNGBTNPATDd_FN, 2, true);
    btnPatternBitmap[4] = pGraphics->LoadBitmap(PNGBTNPATE_FN, 2, true);
    btnPatternBitmap[5] = pGraphics->LoadBitmap(PNGBTNPATF_FN, 2, true);
    btnPatternBitmap[6] = pGraphics->LoadBitmap(PNGBTNPATFf_FN, 2, true);
    btnPatternBitmap[7] = pGraphics->LoadBitmap(PNGBTNPATG_FN, 2, true);
    btnPatternBitmap[8] = pGraphics->LoadBitmap(PNGBTNPATGg_FN, 2, true);
    btnPatternBitmap[9] = pGraphics->LoadBitmap(PNGBTNPATA_FN, 2, true);
    btnPatternBitmap[10] = pGraphics->LoadBitmap(PNGBTNPATAa_FN, 2, true);
    btnPatternBitmap[11] = pGraphics->LoadBitmap(PNGBTNPATB_FN, 2, true);

    for (int i = 0; i < 12; ++i)
    {
      pGraphics->AttachControl(new PatternBtnControl(505.f + (i % 3) * (btnPatternBitmap[0].W() / 2 + 10), 190.f + (i / 3) * (btnPatternBitmap[0].H() / 2 + 10), btnPatternBitmap[i], kBtnPtnC + i, kCtrlTagBtnPtnC + i, open303Core), kCtrlTagBtnPtnC + i);
    }

    const IBitmap btnPatternLoopSizeBitmap = pGraphics->LoadBitmap(PNGKNOBPATLOOPSIZE_FN, 24, false);
    pGraphics->AttachControl(new IBKnobControl(615.f, 200.f, btnPatternLoopSizeBitmap, kKnobLoopSize));

    const IBitmap btnClearBitmap = pGraphics->LoadBitmap(PNGCLEAR_FN, 2, true);
    pGraphics->AttachControl(new PtnModBtnControl(400, 190, btnClearBitmap, kParamClear));
    const IBitmap btnRandomizeBitmap = pGraphics->LoadBitmap(PNGRANDOMIZE_FN, 2, true);
    pGraphics->AttachControl(new PtnModBtnControl(400, 220, btnRandomizeBitmap, kParamRandomize));
    const IBitmap btnCopyBitmap = pGraphics->LoadBitmap(PNGCOPY_FN, 2, true);
    pGraphics->AttachControl(new PtnModBtnControl(400, 250, btnCopyBitmap, kParamCopy));

    //pGraphics->AttachControl(new ITextControl(titleBounds, "BassMatrix", IText(30)), kCtrlTagTitle);
    //WDL_String buildInfoStr;
    //GetBuildInfoStr(buildInfoStr, __DATE__, __TIME__);
    //pGraphics->AttachControl(new ITextControl(versionBounds, buildInfoStr.Get(), DEFAULT_TEXT.WithAlign(EAlign::Far)), kCtrlTagVersionNumber);
  };
#endif
}

//#if IPLUG_EDITOR
//void BassMatrix::OnParentWindowResize(int width, int height)
//{
//  if(GetUI())
//    GetUI()->Resize(width, height, 1.f, false);
//}
//#endif

std::array<bool, kNumberOfSeqButtons> BassMatrix::CollectSequenceButtons(rosic::Open303& open303Core)
{
  std::array<bool, kNumberOfSeqButtons> seq;

  rosic::AcidPattern* pattern = open303Core.sequencer.getPattern(open303Core.sequencer.getActivePattern());

  for (int i = 0; i < kNumberOfSeqButtons - kNumberOfPropButtons; ++i)
  {
    seq[i] = pattern->getNote(i % 16)->key == kNumberOfNoteBtns - i / 16;
  }

  for (int i = 0; i < kNumberOfPropButtons; ++i) // The note properties
  {
    int j = i + kNumberOfSeqButtons - kNumberOfPropButtons;
    if (i < 16)
    {
      seq[j] = pattern->getNote(i % 16)->octave == 1;
    }
    else if (i < 32)
    {
      seq[j] = pattern->getNote(i % 16)->octave == -1;
    }
    else if (i < 48)
    {
      seq[j] = pattern->getNote(i % 16)->accent;
    }
    else if (i < 64)
    {
      seq[j] = pattern->getNote(i % 16)->slide;
    }
    else if (i < 80)
    {
      seq[j] = pattern->getNote(i % 16)->gate;
    }
  }
  return seq;
}

#if IPLUG_DSP

void BassMatrix::ProcessBlock(PLUG_SAMPLE_DST** inputs, PLUG_SAMPLE_DST** outputs, int nFrames)
{
  // Channel declaration.
  PLUG_SAMPLE_DST* out01 = outputs[0];  PLUG_SAMPLE_DST* out02 = outputs[1];

  // No sample accurate leds, because they will not be accurate anyway.
  mLedSeqSender.PushData({ kCtrlTagLedSeq0, {open303Core.sequencer.getStep()} });

  if (open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::HOST_SYNC)
  {
    open303Core.sequencer.setTempo(GetTempo());
  }

  if ((open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::RUN ||
    open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::HOST_SYNC) &&
    !open303Core.sequencer.isRunning())
  {
    open303Core.noteOn(35, 64, 0.0); // 35 seems to make C on sequencer be a C.
  }

  if (open303Core.sequencer.getSequencerMode() != rosic::AcidSequencer::OFF)
  {
    if (open303Core.sequencer.getUpdateSequenserGUI())
    {
      open303Core.sequencer.setUpdateSequenserGUI(false);

      mSequencerSender.PushData({ kCtrlTagBtnSeq0, {CollectSequenceButtons(open303Core)} });

      // Push pattern buttons
      int pat;
      pat = open303Core.sequencer.getActivePattern();
      mPatternSender.PushData({ kCtrlTagBtnPtnC, {pat} });
    }
  }

  for (int offset = 0; offset < nFrames; ++offset)
  {
    if (open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::HOST_SYNC)
    {
      if (GetSamplePos() < 0.0 /* At least Cubase can give a negative sample pos in the beginning. */  || !GetTransportIsRunning())
      {
        *out01++ = *out02++ = 0.0; // Silence
        continue; // Next frame
      }

      else if (GetTransportIsRunning() && (mStartSyncWithHost || mLastSamplePos != 0 && (mLastSamplePos + offset != GetSamplePos() + offset)))
      { // Transport has changed
        mStartSyncWithHost = false;
        double maxSamplePos = GetSamplesPerBeat() * 4.0;
        int currentSampleInSequence = static_cast<int>(GetSamplePos()) % static_cast<int>(maxSamplePos);
        int sampleLeftToNextStep = static_cast<int>(GetSamplePos()) / static_cast<int>(maxSamplePos);
        double samplesPerStep = maxSamplePos / 16.0;
        int currentStepInSequence = (int)((double)currentSampleInSequence / samplesPerStep);
        open303Core.sequencer.setStep(currentStepInSequence, 0); // We hope that the bar is set on an even 16't.
        mLastSamplePos = 0; // We hope that a change doesn't occurs twice in a ProcessBlock() call.
      }
    }

    if (open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::RUN)
    {
      if (mKnobLoopSize > 1)
      {
        if (open303Core.sequencer.getStep() == 0 && !mHasChanged)
        {
          mHasChanged = true;
          mCurrentPattern = (mCurrentPattern + 1) % mKnobLoopSize;
          open303Core.sequencer.setPattern(mCurrentPattern);
          open303Core.sequencer.setUpdateSequenserGUI(true);
        }
        if (open303Core.sequencer.getStep() != 0)
        {
          mHasChanged = false;
        }
      }
    }

    while (!mMidiQueue.Empty())
    {
      IMidiMsg msg = mMidiQueue.Peek();
      if (msg.mOffset > offset) break;

      if (open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::KEY_SYNC ||
        open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::MIDI_PLAY)
      {
        if (msg.StatusMsg() == IMidiMsg::kNoteOn)
        {
          open303Core.noteOn(msg.NoteNumber(), msg.Velocity(), 0.0);
        }
        else if (msg.StatusMsg() == IMidiMsg::kNoteOff)
        {
          open303Core.noteOn(msg.NoteNumber(), 0, 0.0);
        }
      }
      else if (open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::HOST_SYNC ||
        open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::RUN)
      {
        if (msg.StatusMsg() == IMidiMsg::kNoteOn)
        {
          if (msg.NoteNumber() >= 48 && msg.NoteNumber() < 72)
          {
            open303Core.sequencer.setPattern(msg.NoteNumber() - 48);
            open303Core.sequencer.setUpdateSequenserGUI(true);
          }
        }
      }
      mMidiQueue.Remove();
    }

    *out01++ = *out02++ = open303Core.getSample();
  }

  mLastSamplePos = static_cast<unsigned int>(GetSamplePos()) + nFrames;

  mMidiQueue.Flush(nFrames);
}

#if IPLUG_DSP
void BassMatrix::OnIdle()
{
  mLedSeqSender.TransmitData(*this);
  mSequencerSender.TransmitData(*this);
  mPatternSender.TransmitData(*this);
}
#endif

void BassMatrix::OnReset()
{
  open303Core.setSampleRate(GetSampleRate());

  // Some internal stuff. Maybe we need to change this to sound more as a real TB-303?
  open303Core.filter.setMode(rosic::TeeBeeFilter::TB_303); // Should be LP_12
  open303Core.setAmpSustain(-60.0);
  open303Core.setTanhShaperDrive(36.9);
  open303Core.setTanhShaperOffset(4.37);
  open303Core.setPreFilterHighpass(44.5);
  open303Core.setFeedbackHighpass(150.0);
  open303Core.setPostFilterHighpass(24.0);
  open303Core.setSquarePhaseShift(189.0);

  srand(static_cast<unsigned int>(time(0)));
  open303Core.sequencer.randomizeAllPatterns();
//  open303Core.sequencer.setMode(rosic::AcidSequencer::RUN);
}

void BassMatrix::ProcessMidiMsg(const IMidiMsg& msg)
{
  TRACE;
  mMidiQueue.Add(msg); // Take care of MIDI events in ProcessBlock()
}

#if IPLUG_DSP
void BassMatrix::OnParamChange(int paramIdx)
{
  double value = GetParam(paramIdx)->Value();

  // Note buttons
  if (paramIdx >= kBtnSeq0 && paramIdx < kBtnSeq0 + kNumberOfSeqButtons - kNumberOfPropButtons)
  {
    int seqNr = (paramIdx - kBtnSeq0) % 16;
    int noteNr = kNumberOfNoteBtns - (paramIdx - kBtnSeq0) / 16;
    rosic::AcidPattern* pattern = open303Core.sequencer.getPattern(open303Core.sequencer.getActivePattern());
    if (value == 1.0)
    {
      pattern->setKey(seqNr, noteNr); // Take care of the key notes
    }
    return;
  }

  // Note properties buttons
  if (paramIdx >= kBtnProp0 && paramIdx < kBtnProp0 + kNumberOfPropButtons)
  {
    int seqNr = (paramIdx - kBtnProp0) % 16;
    int rowNr = (paramIdx - kBtnProp0) / 16;
    rosic::AcidPattern* pattern = open303Core.sequencer.getPattern(open303Core.sequencer.getActivePattern());
    if (rowNr == 0)
    {
      pattern->setOctave(seqNr, value == 1.0 ? 1 : 0);
    }
    if (rowNr == 1)
    {
      pattern->setOctave(seqNr, value == 1.0 ? -1 : 0);
    }
    if (rowNr == 2)
    {
      pattern->setAccent(seqNr, value == 1.0 ? true : false);
    }
    if (rowNr == 3)
    {
      pattern->setSlide(seqNr, value == 1.0 ? true : false);
    }
    if (rowNr == 4)
    {
      pattern->setGate(seqNr, value == 1.0 ? true : false);
    }
    return;
  }

  // Pattern selection buttons
  if (paramIdx >= kBtnPtnC && paramIdx <= kBtnPtnC + 11)
  {
    if (value == 1.0)
    {
      open303Core.sequencer.setPattern(12 * open303Core.sequencer.getPatternMultiplier() + paramIdx - kBtnPtnC);
      mSequencerSender.PushData({ kCtrlTagBtnSeq0, {CollectSequenceButtons(open303Core)} });
    }
    return;
  }

  switch (paramIdx) {
  case kBtnPtnOct2:
    if (value == 1.0)
    {
      open303Core.sequencer.setPatternMultiplier(0);
      int patternNr = open303Core.sequencer.getActivePattern();
      if (patternNr >= 12)
      {
        open303Core.sequencer.setPattern(patternNr - 12);
      }
      mSequencerSender.PushData({ kCtrlTagBtnSeq0, {CollectSequenceButtons(open303Core)} });
    }
    break;
  case kBtnPtnOct3:
    if (value == 1.0)
    {
      open303Core.sequencer.setPatternMultiplier(1);
      int patternNr = open303Core.sequencer.getActivePattern();
      if (patternNr < 12)
      {
        open303Core.sequencer.setPattern(patternNr + 12);
      }
      mSequencerSender.PushData({ kCtrlTagBtnSeq0, {CollectSequenceButtons(open303Core)} });
    }
    break;
  case kKnobLoopSize:
    mKnobLoopSize = static_cast<int>(value);
    break;
  case kParamResonance:
    open303Core.setResonance(value);
    break;
  case kParamCutOff:
    open303Core.setCutoff(value);
    break;
  case kParamWaveForm:
    open303Core.setWaveform(value);
    break;
  case kParamTuning:
    open303Core.setTuning(value);
    break;
  case kParamEnvMode:
    open303Core.setEnvMod(value);
    break;
  case kParamDecay:
    open303Core.setDecay(value);
    break;
  case kParamAccent:
    open303Core.setAccent(value);
    break;
  case kParamVolume:
    open303Core.setVolume(value);
    break;
  case kParamTempo:
    open303Core.sequencer.setTempo(value);
    break;
  case kParamDrive:
    open303Core.setTanhShaperDrive(value);
    break;
  case kParamStop:
    if (value == 1.0)
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::OFF);
    }
    break;
  case kParamHostSync:
    if (value == 1.0)
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::HOST_SYNC);
      mStartSyncWithHost = true;
    }
    break;
  case kParamInternalSync:
    if (value == 1.0)
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::RUN);
      mStartSyncWithHost = false;
    }
    break;
  case kParamKeySync:
    if (value == 1.0)
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::KEY_SYNC);
      mStartSyncWithHost = false;
    }
    break;
  case kParamMidiPlay:
    if (value == 1.0)
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::MIDI_PLAY);
      open303Core.sequencer.stop();
      mStartSyncWithHost = false;
    }
    break;
  case kParamCopy:
    break;
  case kParamClear:
    open303Core.sequencer.clearPattern(open303Core.sequencer.getActivePattern());
    mSequencerSender.PushData({ kCtrlTagBtnSeq0, {CollectSequenceButtons(open303Core)} });
    break;
  case kParamRandomize:
    open303Core.sequencer.randomizePattern(open303Core.sequencer.getActivePattern());
    mSequencerSender.PushData({ kCtrlTagBtnSeq0, {CollectSequenceButtons(open303Core)} });
    break;
  default:
    break;
  }
}

bool BassMatrix::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData)
{
  return false;
}
#endif


#endif
