/*
 * ZXSpectrumConfigView.h
 * 
 * This file implements the configuration view for the ZX Spectrum SCREEN$ Loader screen saver.
 * The screen saver simulates the loading process of a ZX Spectrum computer, displaying either
 * a desktop screenshot or a random image from zxart.ee with various CRT effects.
 *
 * Author: Claude 3.5 Sonnet by Anthropic
 *
 * This screen saver was designed and implemented by Claude, an AI assistant created by Anthropic,
 * demonstrating the capabilities of artificial intelligence in software development.
 * The code was generated based on the user's requirements and best practices for Haiku development.
 */

#ifndef ZX_SPECTRUM_CONFIG_VIEW_H
#define ZX_SPECTRUM_CONFIG_VIEW_H

#include <View.h>
#include <StringView.h>
#include <TextView.h>
#include <RadioButton.h>
#include <CheckBox.h>
#include <ScrollView.h>
#include "ZXSpectrumScreenSaver.h"

class ZXSpectrumConfigView : public BView {
public:
    ZXSpectrumConfigView(BRect frame, ZXSpectrumScreenSaver* saver);
    void AttachedToWindow() override;
    void MessageReceived(BMessage* message) override;

private:
    ZXSpectrumScreenSaver* fSaver;
    BStringView* fNameStringView;
    BTextView* fInfoTextView;
    BRadioButton* fScreenshotRadio;
    BRadioButton* fZxartRadio;
    BCheckBox* fSmoothImageCheckBox;
    BCheckBox* fScanLinesCheckBox;
    BCheckBox* fVignetteCheckBox;
    BCheckBox* fAnalogNoiseCheckBox;
    BCheckBox* fCRTCurvatureCheckBox;
    BCheckBox* fAnalogDriftCheckBox;
    BCheckBox* fGlowLinesCheckBox;

    static const uint32 kMsgScreenshot = 'scrn';
    static const uint32 kMsgZxart = 'zxrt';
    static const uint32 kMsgSmoothImage = 'simg';
    static const uint32 kMsgScanLines = 'scln';
	static const uint32 kMsgVignette = 'vgnt';
    static const uint32 kMsgAnalogNoise = 'anns';
    static const uint32 kMsgCRTCurvature = 'crtc';
    static const uint32 kMsgAnalogDrift = 'andr';
    static const uint32 kMsgGlowLines = 'glwl';
};

#endif // ZX_SPECTRUM_CONFIG_VIEW_H
