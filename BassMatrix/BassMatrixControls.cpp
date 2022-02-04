#include "BassMatrixControls.h"

SeqLedBtnControl::SeqLedBtnControl(float x, float y, const IBitmap& bitmap, int paramIdx, rosic::Open303& in303) :
  IBSwitchControl(x, y, bitmap, paramIdx), open303Core(in303)
{
}

void SeqLedBtnControl::OnMsgFromDelegate(int msgTag, int dataSize, const void* pData)
{
  if (!IsDisabled() && msgTag == ISender<>::kUpdateMessage)
  {
    IByteStream stream(pData, dataSize);

    int pos = 0;
    ISenderData<1, int> d;
    pos = stream.Get(&d, pos);

    // Turn off all leds
    for (int i = 0; i < 16; i++)
    {
      IControl* pControlOff = GetUI()->GetControlWithTag(kCtrlTagLedSeq0 + i);
      double before = pControlOff->GetValue();
      pControlOff->SetValue(0.0);
      if (before != pControlOff->GetValue())
      {
        pControlOff->SetDirty(true);
      }
    }

    int step = d.vals[0];
    if (step == 0) { step = 15; }
    else { step = (step - 1); }

    assert(step >= 0 && step <= 15);

    IControl* pControlOn = GetUI()->GetControlWithTag(kCtrlTagLedSeq0 + step);
    double before = pControlOn->GetValue();
    pControlOn->SetValue(1.0);
    if (before != pControlOn->GetValue())
    {
      pControlOn->SetDirty(true);
    }

    SetDirty(false);
  }
}
void SeqLedBtnControl::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  return;
}


// A button control that can take a message from the DSP
SeqNoteBtnControl::SeqNoteBtnControl(float x, float y, const IBitmap& bitmap, int paramIdx) :
  IBSwitchControl(x, y, bitmap, paramIdx), mParamIdx(paramIdx)
{
}

void SeqNoteBtnControl::SetSequencerButtons(std::array<bool, kNumberOfSeqButtons> sequencer, IGraphics* ui)
{
  for (int i = 0; i < kNumberOfSeqButtons; i++)
  {
    IControl* pControlBtn = ui->GetControlWithTag(kCtrlTagBtnSeq0 + i);
    double before = pControlBtn->GetValue();
    pControlBtn->SetValue(sequencer[i] ? 1.0 : 0.0);
    if (before != pControlBtn->GetValue())
    {
      pControlBtn->SetDirty(true);
    }
  }
}

void SeqNoteBtnControl::OnMsgFromDelegate(int msgTag, int dataSize, const void* pData)
{
  if (!IsDisabled() && msgTag == ISender<>::kUpdateMessage)
  {
    IByteStream stream(pData, dataSize);
    int pos = 0;
    ISenderData<1, std::array<bool, kNumberOfSeqButtons>> d;
    pos = stream.Get(&d, pos);
    std::array<bool, kNumberOfSeqButtons> sequencer = d.vals[0];

    SeqNoteBtnControl::SetSequencerButtons(sequencer, GetUI());

    SetDirty(false);
  }
}

void SeqNoteBtnControl::OnMouseDown(float x, float y, const IMouseMod& mod)
{
//  IBSwitchControl::OnMouseDown(x, y, mod);
  if (mParamIdx - kBtnSeq0 < kNumberOfSeqButtons - kNumberOfPropButtons)
  {
    // For the notes. Turn off all note buttons on the same column and then turn on the button just pressed.
    for (int row = 0; row < kNumberOfNoteBtns; ++row)
    {
      int col = (mParamIdx - kBtnSeq0) % 16;
      IControl* pControlBtn = GetUI()->GetControlWithTag(kCtrlTagBtnSeq0 + col + 16 * row);
      double before = pControlBtn->GetValue();
      pControlBtn->SetValue(0.0);
      if (before != 0.0)
      {
        pControlBtn->SetDirty(true);
      }
      if (kBtnSeq0 + col + row * 16 == mParamIdx)
      {
        pControlBtn->SetValue(1.0);
        if (before != 1.0)
        {
          pControlBtn->SetDirty(true);
        }
      }
    }
  }
  else
  {
    int col = (mParamIdx - kBtnSeq0) % 16;
    int row = (mParamIdx - kBtnSeq0) / 16;
    IControl* pControlBtn = GetUI()->GetControlWithTag(kCtrlTagBtnProp0 + (row - kNumberOfNoteBtns) * 16 + col);
    if (row == kNumberOfNoteBtns || row == kNumberOfNoteBtns + 1) // Up or down
    { // Up is pressed.
      IControl* pControlBtnUp;
      IControl* pControlBtnDown;

      if (row == kNumberOfNoteBtns)
      {
        pControlBtnUp = pControlBtn;
        pControlBtnDown = GetUI()->GetControlWithTag(kCtrlTagBtnProp0 + col + 16);
      }
      else
      {
        pControlBtnUp = GetUI()->GetControlWithTag(kCtrlTagBtnProp0 + col);
        pControlBtnDown = pControlBtn;
      }

      if (row == kNumberOfNoteBtns) // Up
      {
        if (1.0 == pControlBtnUp->GetValue())
        { // We wants neither up or down
          pControlBtnUp->SetValue(0.0);
          pControlBtnDown->SetValue(0.0);
        }
        else if (0.0 == pControlBtnUp->GetValue())
        {
          pControlBtnDown->SetValue(0.0);
          pControlBtnUp->SetValue(1.0);
        }
      }
      else // Down
      {
        if (1.0 == pControlBtnDown->GetValue())
        { // We wants neither up or down
          pControlBtnUp->SetValue(0.0);
          pControlBtnDown->SetValue(0.0);
        }
        else if (0.0 == pControlBtnDown->GetValue())
        {
          pControlBtnDown->SetValue(1.0);
          pControlBtnUp->SetValue(0.0);
        }
      }

      // The order of the calls to SetDirty() matters. In case one button is lit
      // and the other goes off. We want one who is lit to come last, so the
      // SetDirty() for that control should come last.
      if (pControlBtnDown->GetValue() == 1.0)
      {
        pControlBtnUp->SetDirty(true);
        pControlBtnDown->SetDirty(true);
      }
      else
      {
        pControlBtnDown->SetDirty(true);
        pControlBtnUp->SetDirty(true);
      }
    }
    else // Accent, glide or gate
    {
      pControlBtn->SetValue(pControlBtn->GetValue() == 1.0 ? 0.0 : 1.0);
      pControlBtn->SetDirty(true);
    }
  }
}


SyncBtnControl::SyncBtnControl(float x, float y, const IBitmap& bitmap, int paramIdx, int ctrlTag) :
  IBSwitchControl(x, y, bitmap, paramIdx), mParamIdx(paramIdx), mCtrlTag(ctrlTag)
{
}

void SyncBtnControl::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  IBSwitchControl::OnMouseDown(x, y, mod);
  IControl* pControlStopBtn = GetUI()->GetControlWithTag(kCtrlTagStop);
  IControl* pControlHostSyncBtn = GetUI()->GetControlWithTag(kCtrlTagHostSync);
  IControl* pControlKeySyncBtn = GetUI()->GetControlWithTag(kCtrlTagKeySync);
  IControl* pControlInternalSyncBtn = GetUI()->GetControlWithTag(kCtrlTagInternalSync);
  IControl* pControlMidiPlayBtn = GetUI()->GetControlWithTag(kCtrlTagMidiPlay);
  double stBefore = pControlStopBtn->GetValue();
  double hsBefore = pControlHostSyncBtn->GetValue();
  double ksBefore = pControlKeySyncBtn->GetValue();
  double isBefore = pControlInternalSyncBtn->GetValue();
  double mpBefore = pControlMidiPlayBtn->GetValue();
  pControlStopBtn->SetValue(0.0);
  pControlHostSyncBtn->SetValue(0.0);
  pControlKeySyncBtn->SetValue(0.0);
  pControlInternalSyncBtn->SetValue(0.0);
  pControlMidiPlayBtn->SetValue(0.0);
  if (pControlStopBtn->GetValue() != stBefore) { pControlStopBtn->SetDirty(true); }
  if (pControlHostSyncBtn->GetValue() != hsBefore) { pControlHostSyncBtn->SetDirty(true); }
  if (pControlKeySyncBtn->GetValue() != ksBefore) { pControlKeySyncBtn->SetDirty(true); }
  if (pControlInternalSyncBtn->GetValue() != isBefore) { pControlInternalSyncBtn->SetDirty(true); }
  if (pControlMidiPlayBtn->GetValue() != mpBefore) { pControlMidiPlayBtn->SetDirty(true); }
  IControl* pControlBtn = GetUI()->GetControlWithTag(mCtrlTag);
  pControlBtn->SetValue(1.0);
}


PatternBtnControl::PatternBtnControl(float x, float y, const IBitmap& bitmap, int paramIdx, int ctrlTag, rosic::Open303& in303) :
  IBSwitchControl(x, y, bitmap, paramIdx), mParamIdx(paramIdx), mCtrlTag(ctrlTag), mOctav2Selected(true), mOctav3Selected(false), open303Core(in303)
{
}

void PatternBtnControl::OnMsgFromDelegate(int msgTag, int dataSize, const void* pData)
{
  if (!IsDisabled() && msgTag == ISender<>::kUpdateMessage)
  {
    IByteStream stream(pData, dataSize);
    int pos = 0;
    ISenderData<1, int> d;
    pos = stream.Get(&d, pos);
    int pattern = d.vals[0];

    for (int i = 0; i < kCtrlTagBtnPtnOct3 - kCtrlTagBtnPtnC; i++)
    {
      IControl* pControlBtn = GetUI()->GetControlWithTag(kCtrlTagBtnPtnC + i);
      double before = pControlBtn->GetValue();
      pControlBtn->SetValue(pattern % 12 == i ? 1.0 : 0.0);
      if (before != pControlBtn->GetValue())
      {
        pControlBtn->SetDirty(true);
      }
    }
    GetUI()->GetControlWithTag(kCtrlTagBtnPtnOct2)->SetValue(pattern < 12 ? 1.0 : 0.0);
    GetUI()->GetControlWithTag(kCtrlTagBtnPtnOct3)->SetValue(pattern >= 12 ? 1.0 : 0.0);
    GetUI()->GetControlWithTag(kCtrlTagBtnPtnOct2)->SetDirty(true);
    GetUI()->GetControlWithTag(kCtrlTagBtnPtnOct3)->SetDirty(true);
    SetDirty(false);
  }
}

void PatternBtnControl::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  IBSwitchControl::OnMouseDown(x, y, mod);

  if (mod.L)
  {
    // If it is any of the octave button that has been pressed
    if (mCtrlTag == kCtrlTagBtnPtnOct2 || mCtrlTag == kCtrlTagBtnPtnOct3)
    {
      IControl* pControlOctav2 = GetUI()->GetControlWithTag(kCtrlTagBtnPtnOct2);
      IControl* pControlOctav3 = GetUI()->GetControlWithTag(kCtrlTagBtnPtnOct3);
      double oct2Before = pControlOctav2->GetValue();
      double oct3Before = pControlOctav3->GetValue();
      pControlOctav2->SetValue(mCtrlTag == kCtrlTagBtnPtnOct2 ? 1.0 : 0.0);
      pControlOctav3->SetValue(mCtrlTag == kCtrlTagBtnPtnOct3 ? 1.0 : 0.0);
      if (pControlOctav2->GetValue() != oct2Before) { pControlOctav2->SetDirty(true); }
      if (pControlOctav3->GetValue() != oct3Before) { pControlOctav3->SetDirty(true); }
    }
    else
    {
      for (int i = 0; i < 12; ++i)
      {
        double before = GetUI()->GetControlWithTag(kCtrlTagBtnPtnC + i)->GetValue();
        GetUI()->GetControlWithTag(kCtrlTagBtnPtnC + i)->SetValue(0.0);
        GetUI()->GetControlWithTag(kCtrlTagBtnPtnC + i)->SetValue(kCtrlTagBtnPtnC + i == mCtrlTag ? 1.0 : 0.0);
        GetUI()->GetControlWithTag(kCtrlTagBtnPtnC + i)->SetDirty(before == GetUI()->GetControlWithTag(kCtrlTagBtnPtnC + i)->GetValue());
      }
    }
  }
}

void PatternBtnControl::CreateContextMenu(IPopupMenu & contextMenu)
{
  contextMenu.AddItem("Clear pattern");
  contextMenu.AddItem("Randomize pattern");

  std::vector<std::string> notes = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  for (int i = 2; i <= 3; ++i)
  {
    for (auto note : notes)
    {
      std::string s = "Copy to pattern " + note + " on octav " + std::to_string(i);
      contextMenu.AddItem(s.c_str());
    }
  }

  return;

}

void PatternBtnControl::OnContextSelection(int itemSelected)
{
  switch (itemSelected)
  {
  case 0:
    open303Core.sequencer.clearPattern(12 * open303Core.sequencer.getPatternMultiplier() + mCtrlTag - kCtrlTagBtnPtnC);
    SeqNoteBtnControl::SetSequencerButtons(BassMatrix::CollectSequenceButtons(open303Core), GetUI());
    break;
  case 1:
    open303Core.sequencer.randomizePattern(12 * open303Core.sequencer.getPatternMultiplier() + mCtrlTag - kCtrlTagBtnPtnC);
    SeqNoteBtnControl::SetSequencerButtons(BassMatrix::CollectSequenceButtons(open303Core), GetUI());
    break;
  default:
    open303Core.sequencer.copyPattern(12 * open303Core.sequencer.getPatternMultiplier() + mCtrlTag - kCtrlTagBtnPtnC, itemSelected - 2);
    break;
  }
  return;
}

PtnModBtnControl::PtnModBtnControl(float x, float y, const IBitmap& bitmap, int paramIdx) :
  IBSwitchControl(x, y, bitmap, paramIdx)
{
}

void PtnModBtnControl::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  IBSwitchControl::OnMouseDown(x, y, mod);
  SetValue(0.);
  return;
}
