#include "BassMatrix.h"
#include "IPlug_include_in_plug_src.h"
#include "BassMatrixControls.h"
#include "open303/Source/DSPCode/rosic_Open303.h"

#if IPLUG_EDITOR
#include "IControls.h"
#endif

BassMatrix::BassMatrix(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
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

  GetParam(kParamHostSync)->InitBool("Host Sync", false);
  GetParam(kParamKeySync)->InitBool("Key Sync", false);
  GetParam(kParamInternalSync)->InitBool("Internal Sync", true);
  GetParam(kParamMidiPlay)->InitBool("Midi Play", false);

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

    pGraphics->AttachControl(new IBKnobControl(210, 130, knobBigBitmap, kParamTempo));
//    pGraphics->AttachControl(new IBKnobControl(510, 130, knobBigBitmap, kParamDrive));
    pGraphics->AttachControl(new IBKnobControl(810 - 75, 130, knobBigBitmap, kParamVolume));

    // Led buttons
    const IBitmap ledBtnBitmap = pGraphics->LoadBitmap(PNGBTNLED_FN, 2, true);
    for (int i = 0; i < 16; i++)
    {
        pGraphics->AttachControl(new SeqLedBtnControl(130.f + i * (ledBtnBitmap.W() / 2 + 6), 310.f, ledBtnBitmap, kLedBtn0 + i, open303Core), kCtrlTagLedSeq0 + i, "Sequencer");
    }

    // Sequencer tones buttons
    const IBitmap btnSeqBitmap = pGraphics->LoadBitmap(PNGBTNSEQ_FN, 2, true);
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < kNumberOfNoteBtns; j++)
        {
            pGraphics->AttachControl(new SeqNoteBtnControl(140.f + i * (btnSeqBitmap.W() / 2 + 26), 380.f + j * (btnSeqBitmap.H() + 1),
                btnSeqBitmap, kBtnSeq0 + 16 * j + i),
                kCtrlTagBtnSeq0 + 16 * j + i, "Sequencer");
        }
    }

    // Properties buttons
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            pGraphics->AttachControl(new SeqNoteBtnControl(140.f + i * (btnSeqBitmap.W() / 2 + 26), 660.f + j * (btnSeqBitmap.H() + 1),
                btnSeqBitmap, kBtnProp0 + 16 * j + i),
                kCtrlTagBtnProp0 + 16 * j + i, "Sequencer");
        }
    }

    const IBitmap btnHostSyncBitmap = pGraphics->LoadBitmap(PNGHOSTSYNC_FN, 2, true);
    pGraphics->AttachControl(new SyncBtnControl(140, 800, btnHostSyncBitmap, kParamHostSync, kCtrlTagHostSync), kCtrlTagHostSync);
    const IBitmap btnKeySyncBitmap = pGraphics->LoadBitmap(PNGKEYSYNC_FN, 2, true);
    pGraphics->AttachControl(new SyncBtnControl(250, 800, btnKeySyncBitmap, kParamKeySync, kCtrlTagKeySync), kCtrlTagKeySync);
    const IBitmap btnInternalSyncBitmap = pGraphics->LoadBitmap(PNGINTERNALSYNC_FN, 2, true);
    pGraphics->AttachControl(new SyncBtnControl(360, 800, btnInternalSyncBitmap, kParamInternalSync, kCtrlTagInternalSync), kCtrlTagInternalSync);
    const IBitmap btnMidiPlayBitmap = pGraphics->LoadBitmap(PNGMIDIPLAY_FN, 2, true);
    pGraphics->AttachControl(new SyncBtnControl(470, 800, btnMidiPlayBitmap, kParamMidiPlay, kCtrlTagMidiPlay), kCtrlTagMidiPlay);

    // Pattern controls
    const IBitmap btnPatternOctav2Bitmap = pGraphics->LoadBitmap(PNGBTNPATOCTAV2_FN, 2, true);
    pGraphics->AttachControl(new PatternBtnControl(485, 160, btnPatternOctav2Bitmap, kBtnPtnOct2, kCtrlTagBtnPtnOct2), kCtrlTagBtnPtnOct2);
    const IBitmap btnPatternOctav3Bitmap = pGraphics->LoadBitmap(PNGBTNPATOCTAV3_FN, 2, true);
    pGraphics->AttachControl(new PatternBtnControl(485.f + btnPatternOctav2Bitmap.FW() + 10, 160, btnPatternOctav3Bitmap, kBtnPtnOct3, kCtrlTagBtnPtnOct3), kCtrlTagBtnPtnOct3);
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
      pGraphics->AttachControl(new PatternBtnControl(505.f + (i % 3) * (btnPatternBitmap[0].W() / 2 + 10), 190.f + (i / 3) * (btnPatternBitmap[0].H() / 2 + 10), btnPatternBitmap[i], kBtnPtnC + i, kCtrlTagBtnPtnC + i), kCtrlTagBtnPtnC + i);
    }

    const IBitmap btnPatternLoopSizeBitmap = pGraphics->LoadBitmap(PNGKNOBPATLOOPSIZE_FN, 24, false);
    pGraphics->AttachControl(new IBKnobControl(615.f, 200.f, btnPatternLoopSizeBitmap, kKnobLoopSize));

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
    if (!GetTransportIsRunning())
    {
      *out01++ = *out02++ = 0.0;
      return; // Silence
    }
  }

  if ((open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::RUN ||
    open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::HOST_SYNC) &&
    !open303Core.sequencer.isRunning())
  {
    open303Core.noteOn(36, 64, 0.0);
  }

  if (open303Core.sequencer.getSequencerMode() != rosic::AcidSequencer::OFF)
  {
    if (open303Core.sequencer.getUpdateSequenserGUI())
    {
      open303Core.sequencer.setUpdateSequenserGUI(false);
      std::array<bool, kNumberOfSeqButtons> seq;
      rosic::AcidPattern* pattern = open303Core.sequencer.getPattern(open303Core.sequencer.getActivePattern());

      for (int i = 0; i < kNumberOfSeqButtons - kNumberOfPropButtons; ++i)
      {
        seq[i] = pattern->getNote(i % 16)->key == 11 - i / 16;
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

      mSequencerSender.PushData({ kCtrlTagBtnSeq0, {seq} });

    }
  }

  for (int offset = 0; offset < nFrames; ++offset)
  {
    if (open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::HOST_SYNC)
    {
      if (GetSamplePos() < 0.0) // At least Cubase can give a negative sample pos in the beginning.
      {
        *out01++ = *out02++ = 0.0;
        break; // Next frame
      }
    }

    //if (open303Core.sequencer.getSequencerMode() == rosic::AcidSequencer::HOST_SYNC &&
    //    GetSamplePos() + offset != mLastSamplePos + 1) // Transport has changed
    //{
    //  double maxSamplePos = GetSamplesPerBeat() * 4.0;
    //  int currentSampleInSequence = static_cast<int>(GetSamplePos()) % static_cast<int>(maxSamplePos);
    //  double samplesPerStep = maxSamplePos / 16.0;
    //  int currentStepInSequence = (int)((double)currentSampleInSequence / samplesPerStep);
    //  open303Core.sequencer.setStep(currentStepInSequence, 0);
    //}

    while (!mMidiQueue.Empty())
    {
      IMidiMsg msg = mMidiQueue.Peek();
      if (msg.mOffset > offset) break;

      if (msg.StatusMsg() == IMidiMsg::kNoteOn)
      {
        open303Core.noteOn(msg.NoteNumber(), 64, 0.0);
      }
      else if (msg.StatusMsg() == IMidiMsg::kNoteOff)
      {
        open303Core.noteOn(msg.NoteNumber(), 0, 0.0);
      }

      mMidiQueue.Remove();
    }

//    mLastSamplePos = GetSamplePos();

    *out01++ = *out02++ = open303Core.getSample();
  }
  mMidiQueue.Flush(nFrames);
}

#if IPLUG_DSP
void BassMatrix::OnIdle()
{
  mLedSeqSender.TransmitData(*this);
  mSequencerSender.TransmitData(*this);
}
#endif

void BassMatrix::OnReset()
{
  open303Core.setSampleRate(GetSampleRate());

  open303Core.filter.setMode(rosic::TeeBeeFilter::TB_303); // Should be LP_12
  open303Core.setAmpSustain(-60.0);
  open303Core.setTanhShaperDrive(36.9);
  open303Core.setTanhShaperOffset(4.37);
  open303Core.setPreFilterHighpass(44.5);
  open303Core.setFeedbackHighpass(150.0);
  open303Core.setPostFilterHighpass(24.0);
  open303Core.setSquarePhaseShift(189.0);

  srand(static_cast<unsigned int>(time(0)));
  open303Core.sequencer.randomize();

  //open303Core.setTuning(440.0);
  //open303Core.setCutoff(1000.0);
  //open303Core.setResonance(50.0);
  //open303Core.setEnvMod(0.25);
  //open303Core.setDecay(400.0);
  //open303Core.setAccent(0.5);
  //open303Core.setVolume(-6.0);
  //open303Core.setWaveform(0.0); // Default  open303Core.setWaveform(0.85);

  open303Core.sequencer.setMode(rosic::AcidSequencer::RUN);
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
      open303Core.sequencer.setUpdateSequenserGUI(true);
    }
    return;
  }

  switch (paramIdx) {
  case kBtnPtnOct2:
    if (value == 1.0)
    {
      open303Core.sequencer.setPatternMultiplier(0);
      open303Core.sequencer.setUpdateSequenserGUI(true);
    }
    break;
  case kBtnPtnOct3:
    if (value == 1.0)
    {
      open303Core.sequencer.setPatternMultiplier(1);
      open303Core.sequencer.setUpdateSequenserGUI(true);
    }
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
  case kParamHostSync:
    if (value == 1.0)
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::HOST_SYNC);
    }
    else
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::OFF);
    }
    break;
  case kParamInternalSync:
    if (value == 1.0)
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::RUN);
    }
    else
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::OFF);
    }
    break;
  case kParamKeySync:
    if (value == 1.0)
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::KEY_SYNC);
    }
    else
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::OFF);
    }
    break;
  case kParamMidiPlay:
    if (value == 1.0)
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::OFF);
    }
    else
    {
      open303Core.sequencer.setMode(rosic::AcidSequencer::OFF);
    }
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
