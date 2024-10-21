/*
 * ZXSpectrumScreenSaver.cpp
 * 
 * This file implements the main screen saver class for the ZX Spectrum SCREEN$ Loader screen saver.
 * It manages the lifecycle of the screen saver, including initialization, configuration, drawing,
 * and state management. The class interacts with the ZXSpectrumGLView to render the screen saver content.
 *
 * The screen saver simulates the loading process of a ZX Spectrum computer, displaying either
 * a desktop screenshot or a random image from zxart.ee with various CRT effects.
 *
 * Author: Claude 3.5 Sonnet by Anthropic
 *
 * This screen saver was designed and implemented by Claude, an AI assistant created by Anthropic,
 * demonstrating the capabilities of artificial intelligence in software development.
 * The code was generated based on the user's requirements and best practices for Haiku development.
 */

#include "ZXSpectrumScreenSaver.h"
#include "ZXSpectrumConfigView.h"

ZXSpectrumScreenSaver::ZXSpectrumScreenSaver(BMessage* archive, image_id image)
    : BScreenSaver(archive, image), fGLView(nullptr) {
    RestoreState(archive);
}

void ZXSpectrumScreenSaver::StartConfig(BView* view) {
    ZXSpectrumConfigView* configView = new ZXSpectrumConfigView(view->Bounds(), this);
    view->AddChild(configView);
}

status_t ZXSpectrumScreenSaver::StartSaver(BView* view, bool preview) {
    if (!fGLView) {
        BRect bounds = view->Bounds();
        fGLView = new ZXSpectrumGLView(bounds, preview);

        fGLView->SetImageSource(static_cast<ZXSpectrumGLView::ImageSource>(fSource));
        fGLView->SetSmoothImage(fSmoothImage);
        fGLView->SetScanLine(fScanLines);
        fGLView->SetVignetteEffect(fVignetteEffect);
        fGLView->SetAnalogNoiseEffect(fAnalogNoiseEffect);
        fGLView->SetCRTCurvatureEffect(fCRTCurvatureEffect);
        fGLView->SetAnalogDriftEffect(fAnalogDriftEffect);
        fGLView->SetGlowLinesEffect(fGlowLinesEffect);

        view->AddChild(fGLView);
    }
    SetTickSize(25000);
    return B_OK;
}

void ZXSpectrumScreenSaver::Draw(BView* view, int32 frame) {
    if (fGLView) {
        fGLView->Draw(view->Bounds());
    }
}

void ZXSpectrumScreenSaver::StopSaver() {
}

status_t ZXSpectrumScreenSaver::SaveState(BMessage* into) const {
    into->AddInt32("imageSource", static_cast<int32>(fSource));
    into->AddBool("smooth", fSmoothImage);
    into->AddBool("scanline", fScanLines);
    into->AddBool("vignette", fVignetteEffect);
    into->AddBool("analogNoise", fAnalogNoiseEffect);
    into->AddBool("crtCurvature", fCRTCurvatureEffect);
    into->AddBool("analogDrift", fAnalogDriftEffect);
    into->AddBool("glowLines", fGlowLinesEffect);
    return B_OK;
}

void ZXSpectrumScreenSaver::RestoreState(BMessage* from) {
	if (from != nullptr) {
		int32 source;
	    if (from->FindInt32("imageSource", &source) == B_OK)
	    	fSource = static_cast<ZXSpectrumGLView::ImageSource>(source);
	   	else
			fSource = ZXSpectrumGLView::ImageSource::Screenshot;

	    if (from->FindBool("smooth", &fSmoothImage) != B_OK)
			fSmoothImage = false;

	    if (from->FindBool("scanline", &fScanLines) != B_OK)
			fScanLines = false;

        if (from->FindBool("vignette", &fVignetteEffect) != B_OK)
            fVignetteEffect = false;

        if (from->FindBool("analogNoise", &fAnalogNoiseEffect) != B_OK)
            fAnalogNoiseEffect = false;

        if (from->FindBool("crtCurvature", &fCRTCurvatureEffect) != B_OK)
            fCRTCurvatureEffect = false;

        if (from->FindBool("analogDrift", &fAnalogDriftEffect) != B_OK)
            fAnalogDriftEffect = false;

        if (from->FindBool("glowLines", &fGlowLinesEffect) != B_OK)
            fGlowLinesEffect = false;
	} else {
		fSource = ZXSpectrumGLView::ImageSource::Screenshot;
		fSmoothImage = false;
		fScanLines = false;
        fVignetteEffect = false;
        fAnalogNoiseEffect = false;
        fCRTCurvatureEffect = false;
        fAnalogDriftEffect = false;
        fGlowLinesEffect = false;
	}	
}

void ZXSpectrumScreenSaver::SetImageSource(ZXSpectrumGLView::ImageSource source) {
	fSource = source;
    if (fGLView)fGLView->SetImageSource(fSource);
}

void ZXSpectrumScreenSaver::SetSmoothImage(bool smooth) {
	fSmoothImage = smooth;
	if (fGLView)fGLView->SetSmoothImage(fSmoothImage);
}

void ZXSpectrumScreenSaver::SetScanLines(bool scanlines) {
	fScanLines = scanlines;
	if (fGLView)fGLView->SetScanLine(fScanLines);
}

void ZXSpectrumScreenSaver::SetVignetteEffect(bool enable) {
    fVignetteEffect = enable;
    if (fGLView) fGLView->SetVignetteEffect(fVignetteEffect);
}

void ZXSpectrumScreenSaver::SetAnalogNoiseEffect(bool enable) {
    fAnalogNoiseEffect = enable;
    if (fGLView) fGLView->SetAnalogNoiseEffect(fAnalogNoiseEffect);
}

void ZXSpectrumScreenSaver::SetCRTCurvatureEffect(bool enable) {
    fCRTCurvatureEffect = enable;
    if (fGLView) fGLView->SetCRTCurvatureEffect(fCRTCurvatureEffect);
}

void ZXSpectrumScreenSaver::SetAnalogDriftEffect(bool enable) {
    fAnalogDriftEffect = enable;
    if (fGLView) fGLView->SetAnalogDriftEffect(fAnalogDriftEffect);
}

void ZXSpectrumScreenSaver::SetGlowLinesEffect(bool enable) {
    fGlowLinesEffect = enable;
    if (fGLView) fGLView->SetGlowLinesEffect(fGlowLinesEffect);
}