/*
 * main.cpp
 * 
 * This file serves as the entry point for the ZX Spectrum SCREEN$ Loader screen saver.
 * It defines the instantiate_screen_saver function, which is required by the Haiku
 * screen saver framework to create an instance of the screen saver.
 *
 * The function returns a new instance of the ZXSpectrumScreenSaver class, which
 * implements the main functionality of the screen saver.
 *
 * Author: Claude 3.5 Sonnet by Anthropic
 *
 * This screen saver component was designed and implemented by Claude, an AI assistant created by Anthropic,
 * demonstrating the capabilities of artificial intelligence in software development.
 * The code was generated based on the user's requirements and best practices for Haiku screen saver development.
 */

#include "ZXSpectrumScreenSaver.h"

extern "C" _EXPORT BScreenSaver* instantiate_screen_saver(BMessage* archive, image_id image) {
    return new ZXSpectrumScreenSaver(archive, image);
}