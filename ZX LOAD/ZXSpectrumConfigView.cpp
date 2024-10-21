/*
 * ZXSpectrumConfigView.cpp
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

#include "ZXSpectrumConfigView.h"
#include <LayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <GridView.h>

ZXSpectrumConfigView::ZXSpectrumConfigView(BRect frame, ZXSpectrumScreenSaver* saver)
    : BView(frame, "ZXSpectrumConfigView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
      fSaver(saver) {
    SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

    fNameStringView = new BStringView("nameString", "ZX Spectrum SCREEN$ Loader");
    fNameStringView->SetFont(be_bold_font);
    
    // Add radio buttons for image source selection
    fScreenshotRadio = new BRadioButton("screenshotRadio", "Desktop", new BMessage(kMsgScreenshot));
    fZxartRadio = new BRadioButton("zxartRadio", "Random image from zxart.ee", new BMessage(kMsgZxart));
    fSmoothImageCheckBox = new BCheckBox("smoothImage", "Smooth", new BMessage(kMsgSmoothImage));
    fScanLinesCheckBox = new BCheckBox("scanLines", "Scanlines", new BMessage(kMsgScanLines));
    fVignetteCheckBox = new BCheckBox("vignette", "Vignette", new BMessage(kMsgVignette));
    fAnalogNoiseCheckBox = new BCheckBox("analogNoise", "Analog Noise", new BMessage(kMsgAnalogNoise));
    fCRTCurvatureCheckBox = new BCheckBox("crtCurvature", "CRT Curvature", new BMessage(kMsgCRTCurvature));
    fAnalogDriftCheckBox = new BCheckBox("analogDrift", "Analog Drift", new BMessage(kMsgAnalogDrift));
    fGlowLinesCheckBox = new BCheckBox("glowLines", "Glow Lines", new BMessage(kMsgGlowLines));
	
	BGridLayout* fEffectsLayout = BGridLayoutBuilder(B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
        .Add(fSmoothImageCheckBox, 0, 0)
        .Add(fScanLinesCheckBox, 1, 0)
        .Add(fVignetteCheckBox, 0, 1)
        .Add(fAnalogNoiseCheckBox, 1, 1)
        .Add(fCRTCurvatureCheckBox, 0, 2)
        .Add(fAnalogDriftCheckBox, 1, 2)
        .Add(fGlowLinesCheckBox, 0, 3);

    BRect textRect = Bounds();
    textRect.InsetBy(5, 5);
    fInfoTextView = new BTextView(textRect, "infoTextView", textRect, B_FOLLOW_ALL_SIDES);
    fInfoTextView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
    fInfoTextView->MakeEditable(false);
    fInfoTextView->SetStylable(true);

    fInfoTextView->Insert("©2024 Claude 3.5 Sonnet by Anthropic\n\n");
    fInfoTextView->Insert("This screen saver simulates the loading process of a ZX Spectrum computer. ");
    fInfoTextView->Insert("It can display either a screenshot of your current desktop or a random image from zxart.ee.\n\n");
    fInfoTextView->Insert("This screen saver was designed and implemented by Claude, an AI assistant created by Anthropic, ");
	fInfoTextView->Insert("demonstrating the capabilities of artificial intelligence in software development.\n");
	fInfoTextView->Insert("The code was generated based on the user's requirements and best practices for Haiku development.\n");

    BScrollView* infoScrollView = new BScrollView("infoScrollView", fInfoTextView, B_WILL_DRAW | B_FRAME_EVENTS, false, true);

    BGroupLayout* layout = new BGroupLayout(B_VERTICAL);
    SetLayout(layout);

    layout->SetInsets(B_USE_DEFAULT_SPACING);

    layout->AddView(fNameStringView);
    layout->AddView(fScreenshotRadio);
    layout->AddView(fZxartRadio);
    layout->AddItem(fEffectsLayout);
    layout->AddView(infoScrollView);
}

void ZXSpectrumConfigView::AttachedToWindow() {
    fScreenshotRadio->SetTarget(this);
    fZxartRadio->SetTarget(this);
    fSmoothImageCheckBox->SetTarget(this);
    fScanLinesCheckBox->SetTarget(this);
	fVignetteCheckBox->SetTarget(this);
    fAnalogNoiseCheckBox->SetTarget(this);
    fCRTCurvatureCheckBox->SetTarget(this);
    fAnalogDriftCheckBox->SetTarget(this);
    fGlowLinesCheckBox->SetTarget(this);
    
    if (fSaver->GetImageSource() == ZXSpectrumGLView::ImageSource::Screenshot) {
        fScreenshotRadio->SetValue(B_CONTROL_ON);
    } else {
        fZxartRadio->SetValue(B_CONTROL_ON);
    }
    fSmoothImageCheckBox->SetValue(fSaver->GetSmoothImage());
    fScanLinesCheckBox->SetValue(fSaver->GetScanLines());
	fVignetteCheckBox->SetValue(fSaver->GetVignetteEffect());
    fAnalogNoiseCheckBox->SetValue(fSaver->GetAnalogNoiseEffect());
    fCRTCurvatureCheckBox->SetValue(fSaver->GetCRTCurvatureEffect());
    fAnalogDriftCheckBox->SetValue(fSaver->GetAnalogDriftEffect());
    fGlowLinesCheckBox->SetValue(fSaver->GetGlowLinesEffect());
}

void ZXSpectrumConfigView::MessageReceived(BMessage* message) {
    switch (message->what) {
        case kMsgScreenshot:
            fSaver->SetImageSource(ZXSpectrumGLView::ImageSource::Screenshot);
            break;
        case kMsgZxart:
            fSaver->SetImageSource(ZXSpectrumGLView::ImageSource::Zxart);
            break;
        case kMsgSmoothImage:
            fSaver->SetSmoothImage(fSmoothImageCheckBox->Value() == B_CONTROL_ON);
            break;
        case kMsgScanLines:
            fSaver->SetScanLines(fScanLinesCheckBox->Value() == B_CONTROL_ON);
            break;
        case kMsgVignette:
            fSaver->SetVignetteEffect(fVignetteCheckBox->Value() == B_CONTROL_ON);
            break;
        case kMsgAnalogNoise:
            fSaver->SetAnalogNoiseEffect(fAnalogNoiseCheckBox->Value() == B_CONTROL_ON);
            break;
        case kMsgCRTCurvature:
            fSaver->SetCRTCurvatureEffect(fCRTCurvatureCheckBox->Value() == B_CONTROL_ON);
            break;
        case kMsgAnalogDrift:
            fSaver->SetAnalogDriftEffect(fAnalogDriftCheckBox->Value() == B_CONTROL_ON);
            break;
        case kMsgGlowLines:
            fSaver->SetGlowLinesEffect(fGlowLinesCheckBox->Value() == B_CONTROL_ON);
            break;
        default:
            BView::MessageReceived(message);
    }
}
