/*
 * ZXSpectrumScreenSaver.h
 * 
 * This file declares the ZXSpectrumScreenSaver class, which is the main class for the ZX Spectrum SCREEN$ Loader screen saver.
 * It defines the interface for managing the screen saver's lifecycle, including configuration, rendering, and state management.
 *
 * The class acts as a bridge between the Haiku screen saver framework and the custom ZXSpectrumGLView
 * that handles the actual rendering of the ZX Spectrum-style graphics and effects.
 *
 * Author: Claude 3.5 Sonnet by Anthropic
 *
 * This screen saver was designed and implemented by Claude, an AI assistant created by Anthropic,
 * demonstrating the capabilities of artificial intelligence in software development.
 * The code was generated based on the user's requirements and best practices for Haiku development.
 */

#ifndef ZX_SPECTRUM_SCREEN_SAVER_H
#define ZX_SPECTRUM_SCREEN_SAVER_H

#include <ScreenSaver.h>
#include "ZXSpectrumGLView.h"

class ZXSpectrumScreenSaver : public BScreenSaver {
public:
    ZXSpectrumScreenSaver(BMessage* archive, image_id image);
    void StartConfig(BView* view);
    status_t StartSaver(BView* view, bool preview);
    void Draw(BView* view, int32 frame);
    void StopSaver();
    status_t SaveState(BMessage* into) const;
    void RestoreState(BMessage* from);

    void SetImageSource(ZXSpectrumGLView::ImageSource source);
	void SetSmoothImage(bool smooth);
    void SetScanLines(bool scanlines);
    void SetVignetteEffect(bool enable);
    void SetAnalogNoiseEffect(bool enable);
    void SetCRTCurvatureEffect(bool enable);
    void SetAnalogDriftEffect(bool enable);
    void SetGlowLinesEffect(bool enable);

	ZXSpectrumGLView::ImageSource GetImageSource() const { return fSource; }
    bool GetSmoothImage() const { return fSmoothImage; }
    bool GetScanLines() const {return fScanLines; }
    bool GetVignetteEffect() const { return fVignetteEffect; }
    bool GetAnalogNoiseEffect() const { return fAnalogNoiseEffect; }
    bool GetCRTCurvatureEffect() const { return fCRTCurvatureEffect; }
    bool GetAnalogDriftEffect() const { return fAnalogDriftEffect; }
    bool GetGlowLinesEffect() const { return fGlowLinesEffect; }

private:
    ZXSpectrumGLView* fGLView;
    ZXSpectrumGLView::ImageSource fSource;
    bool fSmoothImage;
    bool fScanLines;
    bool fVignetteEffect;
    bool fAnalogNoiseEffect;
    bool fCRTCurvatureEffect;
    bool fAnalogDriftEffect;
    bool fGlowLinesEffect;
};

#endif // ZX_SPECTRUM_SCREEN_SAVER_H
