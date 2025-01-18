# IPLUG2_ROOT should point to the top level IPLUG2 folder from the project folder
# By default, that is three directories up from /Examples/BassMatrix/config
IPLUG2_ROOT = ../../iPlug2
include ../../iPlug2/common-web.mk

SRC += $(PROJECT_ROOT)/BassMatrix.cpp
SRC += $(PROJECT_ROOT)/BassMatrixControls.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/GlobalFunctions.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_AcidPattern.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_AcidSequencer.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_AnalogEnvelope.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_BiquadFilter.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_BlendOscillator.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_Complex.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_DecayEnvelope.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_EllipticQuarterBandFilter.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_FourierTransformerRadix2.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_FunctionTemplates.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_LeakyIntegrator.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_MidiNoteEvent.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_MipMappedWaveTable.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_NumberManipulations.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_OnePoleFilter.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_Open303.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_RealFunctions.cpp
SRC += $(PROJECT_ROOT)/open303/Source/DSPCode/rosic_TeeBeeFilter.cpp

WAM_SRC += $(IPLUG_EXTRAS_PATH)/Synth/*.cpp

WAM_CFLAGS +=  -I$(IPLUG_SYNTH_PATH)

WEB_CFLAGS += -DIGRAPHICS_NANOVG -DIGRAPHICS_GLES2

WAM_LDFLAGS += -O3 -s EXPORT_NAME="'ModuleFactory'" -s ASSERTIONS=0

WEB_LDFLAGS += -O3 -s ASSERTIONS=0

WEB_LDFLAGS += $(NANOVG_LDFLAGS)
