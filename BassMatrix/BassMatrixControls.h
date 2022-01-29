#pragma once

#if IPLUG_EDITOR
#include "IControls.h"
#endif

#include "BassMatrix.h"
#include "open303/Source/DSPCode/rosic_Open303.h"

// A button control that can take a message from the DSP
class SeqLedBtnControl : public IBSwitchControl
{
public:
  SeqLedBtnControl(float x, float y, const IBitmap& bitmap, int paramIdx, rosic::Open303& in303);
  void OnMsgFromDelegate(int msgTag, int dataSize, const void* pData) override;
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;

private:
  // the embedded core dsp object:
  rosic::Open303& open303Core;
};

// A button control that can take a message from the DSP
class SeqNoteBtnControl : public IBSwitchControl
{
public:
  SeqNoteBtnControl(float x, float y, const IBitmap& bitmap, int paramIdx);
  void OnMsgFromDelegate(int msgTag, int dataSize, const void* pData) override;
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
protected:
  int mParamIdx;
};

// The buttons to choose sync mode.
class SyncBtnControl : public IBSwitchControl
{
public:
  SyncBtnControl(float x, float y, const IBitmap& bitmap, int paramIdx, int ctrlTag);
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
protected:
  int mParamIdx;
  int mCtrlTag;
};

// The buttons to choose pattern.
class PatternBtnControl : public IBSwitchControl
{
public:
  PatternBtnControl(float x, float y, const IBitmap& bitmap, int paramIdx, int ctrlTag, rosic::Open303& open303Core);
  void OnMsgFromDelegate(int msgTag, int dataSize, const void* pData) override;
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
  void CreateContextMenu(IPopupMenu& contextMenu) override;
  void OnContextSelection(int itemSelected) override;

protected:
  int mParamIdx;
  int mCtrlTag;
  int mOctav2Selected;
  int mOctav3Selected;
private:
  // the embedded core dsp object:
  rosic::Open303& open303Core;
};
