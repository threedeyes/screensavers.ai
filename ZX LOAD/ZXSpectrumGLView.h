/*
 * ZXSpectrumGLView.cpp
 *
 * This file implements the OpenGL view for the ZX Spectrum SCREEN$ Loader screen saver.
 * It simulates the ZX Spectrum loading process, renders images with various CRT effects,
 * and handles both desktop screenshots and images from zxart.ee.
 *
 * Author: Claude 3.5 Sonnet by Anthropic
 *
 * This screen saver was designed and implemented by Claude, an AI assistant created by Anthropic,
 * demonstrating the capabilities of artificial intelligence in software development.
 * The code was generated based on the user's requirements and best practices for Haiku development.
 */

#ifndef ZX_SPECTRUM_GL_VIEW_H
#define ZX_SPECTRUM_GL_VIEW_H

#define GL_VERSION_4_6 1
#define GL_GLEXT_PROTOTYPES 1

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <GLView.h>

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>

struct Color {
    uint8_t r, g, b;
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
};

class ZXSpectrumGLView : public BGLView {
public:
    enum class ImageSource {
        Screenshot,
        Zxart
    };

    ZXSpectrumGLView(BRect frame, bool preview);
    ~ZXSpectrumGLView();
    void AttachedToWindow() override;
    void Draw(BRect updateRect) override;
    void FrameResized(float width, float height) override;

    void SetImageSource(ImageSource source) { fImageSource = source; }
	bool GetSmoothImage() const { return fSmoothImageEffect; }

    void SetSmoothImage(bool smooth) { fSmoothImageEffect = smooth; }
	ImageSource GetImageSource() const { return fImageSource; }

    void SetScanLine(bool scanlines) { fScanLinesEffect = scanlines; }
    bool GetScanline() const { return fScanLinesEffect; }

	void SetVignetteEffect(bool enable) { fVignetteEffect = enable; }
    bool GetVignetteEffect() const { return fVignetteEffect; }

    void SetAnalogNoiseEffect(bool enable) { fAnalogNoiseEffect = enable; }
    bool GetAnalogNoiseEffect() const { return fAnalogNoiseEffect; }

    void SetCRTCurvatureEffect(bool enable) { fCRTCurvatureEffect = enable; }
    bool GetCRTCurvatureEffect() const { return fCRTCurvatureEffect; }

    void SetAnalogDriftEffect(bool enable) { fAnalogDriftEffect = enable; }
    bool GetAnalogDriftEffect() const { return fAnalogDriftEffect; }

    void SetGlowLinesEffect(bool enable) { fGlowLinesEffect = enable; }
    bool GetGlowLinesEffect() const { return fGlowLinesEffect; }

private:
    bool isPreview;
    float fWidth, fHeight;
    int loadStage, loadProgress;
    int frameCount;
    int borderColor;
    float borderStripeWidth;
    float pilotToneOffset;
    std::vector<uint8_t> imageBuffer;
    std::vector<uint8_t> zxMemory;
    std::string fileName;
    BBitmap* screenshot;
    ImageSource fImageSource;
    std::thread loadingThread;
    std::mutex imageMutex;
    std::atomic<bool> imageLoaded{false};
    std::atomic<bool> loadingError{false};    

	bool fSmoothImageEffect;
    bool fScanLinesEffect;
	bool fVignetteEffect;
    bool fAnalogNoiseEffect;
    bool fCRTCurvatureEffect;
    bool fAnalogDriftEffect;
    bool fGlowLinesEffect;

    // OpenGL related members
    GLuint vao, vbo, ebo;
    GLuint texture;
    GLuint shaderProgram;

    static const Color zxColors[16];

    void InitializeOpenGL();
    void SetupShaders();
    void SetupBuffers();
    void UpdateTexture();
    void RenderFrame();
    void UpdateShaderUniforms();
    void CleanupGL();

    void LoadImageAsync();
    void LoadImageFromZXArt();
    void CaptureScreen();
    void ConvertToZXSpectrum();
    void ClearScreen();
    void DrawPowerOn();
    void DrawLoadScreen();
    void DrawBorderFlash();
    void DrawPilotTone();
    void DrawRandomBorder();
    void DrawByteScreen();
    void DrawImageLoading();

    void GaussianBlur(std::vector<Color>& image, int width, int height, float sigma = 0.8f);
    void FloydSteinbergDithering(std::vector<Color>& image, int width, int height);

    int GetStageLength(int stage);
    float ColorDistance(const Color& c1, const Color& c2);
    int FindClosestColor(const Color& color);
    void PrintChar(char c, int x, int y, uint8_t ink, uint8_t paper, bool bright = false, bool flash = false);
    void PrintText(const char* text, int x, int y, uint8_t ink, uint8_t paper, bool bright = false, bool flash = false);
    void FindOptimalInkPaper(const std::vector<Color>& block, uint8_t& ink, uint8_t& paper, bool& bright);

    void CheckShaderCompileStatus(GLuint shader);
    void CheckShaderLinkStatus(GLuint program);
};

#endif // ZX_SPECTRUM_GL_VIEW_H
