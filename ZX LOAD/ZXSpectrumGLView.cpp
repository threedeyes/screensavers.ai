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

#include <Screen.h>
#include <Bitmap.h>
#include <cmath>
#include <algorithm>

#include "ZXSpectrumGLView.h"
#include "ZXSpectrumFont.h"
#include "ZXArtLoader.h"

const Color ZXSpectrumGLView::zxColors[16] = {
    {0x00, 0x00, 0x00}, {0x00, 0x00, 0xD7}, {0xD7, 0x00, 0x00}, {0xD7, 0x00, 0xD7},
    {0x00, 0xD7, 0x00}, {0x00, 0xD7, 0xD7}, {0xD7, 0xD7, 0x00}, {0xD7, 0xD7, 0xD7},
    {0x00, 0x00, 0x00}, {0x00, 0x00, 0xFF}, {0xFF, 0x00, 0x00}, {0xFF, 0x00, 0xFF},
    {0x00, 0xFF, 0x00}, {0x00, 0xFF, 0xFF}, {0xFF, 0xFF, 0x00}, {0xFF, 0xFF, 0xFF}
};

ZXSpectrumGLView::ZXSpectrumGLView(BRect frame, bool preview)
    : BGLView(frame.InsetBySelf(-1, -1), "ZXSpectrumGLView", B_FOLLOW_ALL, B_WILL_DRAW, BGL_RGB | BGL_DOUBLE | BGL_DEPTH),
      isPreview(preview), fWidth(frame.Width()), fHeight(frame.Height()), loadStage(0), loadProgress(0), frameCount(0),
      borderColor(7), borderStripeWidth(0.05f), pilotToneOffset(0.0f), fImageSource(ImageSource::Zxart),
      fSmoothImageEffect(true), fScanLinesEffect(true), fVignetteEffect(true), fAnalogNoiseEffect(true),
	  fCRTCurvatureEffect(true), fAnalogDriftEffect(false), fGlowLinesEffect(true) {
    srand(time(NULL));
    zxMemory.resize(6912, 0);
    imageBuffer.resize(6912, 0);
    
    LoadImageAsync();
}

ZXSpectrumGLView::~ZXSpectrumGLView() {
    if (loadingThread.joinable()) {
        loadingThread.join();
    }
    
    CleanupGL();
}

void ZXSpectrumGLView::AttachedToWindow() {
    BGLView::AttachedToWindow();
    LockGL();

    InitializeOpenGL();
    SetupShaders();
    SetupBuffers();
    
    UnlockGL();
}

void ZXSpectrumGLView::InitializeOpenGL() {
    glViewport(0, 0, fWidth, fHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void ZXSpectrumGLView::SetupShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const char* fragmentShaderSource = R"(
		#version 330 core
		in vec2 TexCoord;
		out vec4 FragColor;
		
		uniform sampler2D screenTexture;
		uniform vec2 screenSize;
		uniform vec4 screenRect;
		uniform vec3 borderColor;
		uniform bool useScanlines;
		uniform bool useSmoothing;
		uniform float time;
		uniform float pilotToneOffset;
		uniform bool pilotToneActive;
		uniform vec3 pilotToneColor1;
		uniform vec3 pilotToneColor2;
		uniform bool flashBorderActive;
		uniform bool randomBorderActive;
		uniform bool loadingStripesActive;
		uniform float borderStripeWidth;
		uniform bool isPreview;
		uniform bool useVignetteEffect;
		uniform bool useAnalogNoiseEffect;
		uniform bool useCRTCurvatureEffect;
		uniform bool useAnalogDriftEffect;
		uniform bool useGlowLinesEffect;
		
		vec2 curveRemapping(vec2 uv) {
		    uv = uv * 2.0 - 1.0;
		    vec2 offset = abs(uv.yx) / vec2(7.0, 5.0);
		    uv = uv + uv * offset * offset;
		    uv = uv * 0.5 + 0.5;
		    return uv;
		}
		
		vec4 sampleColor(vec2 uv) {
		    if (uv.x >= screenRect.x && uv.x <= screenRect.x + screenRect.z &&
		        uv.y >= screenRect.y && uv.y <= screenRect.y + screenRect.w) {
		        vec2 texCoord = (uv - screenRect.xy) / screenRect.zw;
		        return texture(screenTexture, texCoord);
		    } else {
		        vec4 color = vec4(borderColor, 1.0);
		        
		        if (pilotToneActive && borderStripeWidth > 0.0) {
		            float stripePosition = mod(uv.y - pilotToneOffset, borderStripeWidth * 2.0);
		            if (stripePosition < borderStripeWidth) {
		                color.rgb = pilotToneColor1;
		            } else {
		                color.rgb = pilotToneColor2;
		            }
		        } else if (randomBorderActive || loadingStripesActive) {
		            float stripePosition = mod(uv.y + pilotToneOffset, borderStripeWidth * 2.0);
		            if (stripePosition < borderStripeWidth) {
		                color.rgb = vec3(1.0, 1.0, 0.0);
		            } else {
		                color.rgb = vec3(0.0, 0.0, 1.0);
		            }
		        }
		        
		        return color;
		    }
		}
		
		void main() {
		    vec2 uv = gl_FragCoord.xy / screenSize;
		    vec2 originalUV = uv;
		    
		    if (useCRTCurvatureEffect) {
		        uv = curveRemapping(uv);
		    }
		    
		    if (useAnalogDriftEffect) {
		        float drift = sin(uv.y * 10.0 + time) * 0.001;
		        uv.x += drift;
		    }
		    
		    vec4 color = sampleColor(uv);
		    
		    if (useSmoothing) {
		        vec2 texelSize = 1.0 / screenSize;
		        vec4 c1 = sampleColor(uv + vec2(-texelSize.x, 0));
		        vec4 c2 = sampleColor(uv + vec2(texelSize.x, 0));
		        vec4 c3 = sampleColor(uv + vec2(0, -texelSize.y));
		        vec4 c4 = sampleColor(uv + vec2(0, texelSize.y));
		        color = (color + c1 + c2 + c3 + c4) / 5.0;
		    }
		    
		    if (useScanlines) {
		    	float period = 1.0;
		    	if (isPreview)period = 3.0;
		        float scanline = sin(uv.y * screenSize.y * period) * 0.04;
		        color -= vec4(scanline);
		    }
		    
		    if (useVignetteEffect) {
		        float vignette = length(vec2(0.5) - originalUV);
		        color *= 1.0 - vignette * 0.5;
		    }
		    
		    if (useAnalogNoiseEffect) {
		        float noise = fract(sin(dot(uv, vec2(12.9898, 78.233) * time)) * 43758.5453);
		        color += vec4(noise) * 0.05;
		    }
		    
		    if (useGlowLinesEffect) {
				float pilotToneNoise = sin((uv.y - time * 0.5f) * 15.0) * 0.1;
				color += vec4(pilotToneNoise * 0.4f);
			}
		    
		    FragColor = color;
		}
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    CheckShaderCompileStatus(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    CheckShaderCompileStatus(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    CheckShaderLinkStatus(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void ZXSpectrumGLView::SetupBuffers() {
    float vertices[] = {
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ZXSpectrumGLView::Draw(BRect updateRect) {
    LockGL();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    switch (loadStage) {
        case 0: DrawPowerOn(); break;
        case 1: DrawLoadScreen(); break;
        case 2: DrawBorderFlash(); break;
        case 3: DrawPilotTone(); break;
        case 4: DrawRandomBorder(); break;
        case 5: DrawByteScreen(); break;
        case 6: DrawPilotTone(); break;
        case 7: DrawImageLoading(); break;
        case 8: DrawBorderFlash(); break;
        case 9: DrawPilotTone(); break;
        default: DrawRandomBorder(); break;
    }
    
    UpdateTexture();
    RenderFrame();
    
    SwapBuffers();
    UnlockGL();

    loadProgress++;
    if (loadProgress >= GetStageLength(loadStage)) {
        loadStage++;
        loadProgress = 0;
        if (loadStage == 3 || loadStage == 6) {
            pilotToneOffset = 0.0f;
        }
    }

    frameCount++;
}

void ZXSpectrumGLView::UpdateTexture() {
    std::vector<uint8_t> displayBuffer(256 * 192 * 3, 0);

    for (int y = 0; y < 192; ++y) {
        for (int x = 0; x < 256; ++x) {
            int pixelAddr = ((y & 0xC0) << 5) | ((y & 0x07) << 8) | ((y & 0x38) << 2) | (x >> 3);
            int attrAddr = 6144 + ((y >> 3) << 5) + (x >> 3);
            int bit = 7 - (x & 0x07);

            uint8_t pixelByte = zxMemory[pixelAddr];
            uint8_t attrByte = zxMemory[attrAddr];

            uint8_t ink = attrByte & 0x07;
            uint8_t paper = (attrByte >> 3) & 0x07;
            bool bright = (attrByte & 0x40) != 0;
            bool flash = (attrByte & 0x80) != 0;

            if (flash && (frameCount % 32) < 16) {
                std::swap(ink, paper);
            }

            if (bright) {
                ink += 8;
                paper += 8;
            }

            const Color& color = zxColors[(pixelByte & (1 << bit)) ? ink : paper];

            int bufferPos = ((191 - y) * 256 + x) * 3;
            displayBuffer[bufferPos] = color.r;
            displayBuffer[bufferPos + 1] = color.g;
            displayBuffer[bufferPos + 2] = color.b;
        }
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 192, 0, GL_RGB, GL_UNSIGNED_BYTE, displayBuffer.data());
}

void ZXSpectrumGLView::RenderFrame() {
    glUseProgram(shaderProgram);
    
    UpdateShaderUniforms();
    
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
}

void ZXSpectrumGLView::UpdateShaderUniforms() {
    glUseProgram(shaderProgram);

    float aspectRatio = 4.0f / 3.0f;
    float screenWidth, screenHeight;
    float screenSize = isPreview ? 0.78f : 0.82f;
    if (fWidth / fHeight > aspectRatio) {
        screenHeight = fHeight * screenSize;
        screenWidth = screenHeight * aspectRatio;
    } else {
        screenWidth = fWidth * screenSize;
        screenHeight = screenWidth / aspectRatio;
    }
    float screenX = (fWidth - screenWidth) / 2.0f;
    float screenY = (fHeight - screenHeight) / 2.0f;

    const Color& redColor = zxColors[2];
    const Color& cyanColor = zxColors[5];
    const Color& borderCol = zxColors[borderColor];

    glUniform1i(glGetUniformLocation(shaderProgram, "screenTexture"), 0);
    glUniform2f(glGetUniformLocation(shaderProgram, "screenSize"), fWidth, fHeight);
    glUniform4f(glGetUniformLocation(shaderProgram, "screenRect"), screenX / fWidth, screenY / fHeight, screenWidth / fWidth, screenHeight / fHeight);
    glUniform3f(glGetUniformLocation(shaderProgram, "borderColor"), borderCol.r / 255.0f, borderCol.g / 255.0f, borderCol.b / 255.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "pilotToneColor1"), redColor.r / 255.0f, redColor.g / 255.0f, redColor.b / 255.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "pilotToneColor2"), cyanColor.r / 255.0f, cyanColor.g / 255.0f, cyanColor.b / 255.0f);
    glUniform1i(glGetUniformLocation(shaderProgram, "useScanlines"), fScanLinesEffect);
    glUniform1i(glGetUniformLocation(shaderProgram, "useSmoothing"), fSmoothImageEffect);
    glUniform1i(glGetUniformLocation(shaderProgram, "useVignetteEffect"), fVignetteEffect);
    glUniform1i(glGetUniformLocation(shaderProgram, "useAnalogNoiseEffect"), fAnalogNoiseEffect);
    glUniform1i(glGetUniformLocation(shaderProgram, "useCRTCurvatureEffect"), fCRTCurvatureEffect);
    glUniform1i(glGetUniformLocation(shaderProgram, "useAnalogDriftEffect"), fAnalogDriftEffect);
    glUniform1i(glGetUniformLocation(shaderProgram, "useGlowLinesEffect"), fGlowLinesEffect);
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), frameCount / 60.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "pilotToneOffset"), pilotToneOffset);
    glUniform1i(glGetUniformLocation(shaderProgram, "pilotToneActive"), loadStage == 3 || loadStage == 6 || loadStage == 9);    
    glUniform1i(glGetUniformLocation(shaderProgram, "flashBorderActive"), loadStage == 2 || loadStage == 8);
    glUniform1i(glGetUniformLocation(shaderProgram, "randomBorderActive"), loadStage == 4 || loadStage > 9);
    glUniform1i(glGetUniformLocation(shaderProgram, "loadingStripesActive"), loadStage == 7);
    glUniform1f(glGetUniformLocation(shaderProgram, "borderStripeWidth"), borderStripeWidth);    
    glUniform1i(glGetUniformLocation(shaderProgram, "isPreview"), isPreview);    
}

void ZXSpectrumGLView::LoadImageAsync() {
    if (loadingThread.joinable()) {
        loadingThread.join();
    }
    imageLoaded = false;
    loadingError = false;
    fileName = "Desktop.scr";
    loadingThread = std::thread([this]() {
        if (fImageSource == ImageSource::Screenshot) {                          
            CaptureScreen();
            ConvertToZXSpectrum();
        } else {
            LoadImageFromZXArt();
        }
        if (!loadingError)
            imageLoaded = true;
    });
}

void ZXSpectrumGLView::LoadImageFromZXArt() {
    ZXArtCollection collection;
    auto files = collection.getFiles(rand() % 500, 1, SortType::Votes, SortDirection::Descending);
    if (files.size() > 0) {
        files[0]->downloadFile();
        fileName = files[0]->getTransliteratedTitle();
        std::vector<uint8_t> tempBuffer = files[0]->getFileData();
        imageBuffer = std::move(tempBuffer);
    } else {
        loadingError = true;
    }
}

void ZXSpectrumGLView::CaptureScreen() {
    BScreen screen;
    BRect screenFrame = screen.Frame();
    screenshot = new BBitmap(screenFrame, B_RGB32);
    screen.ReadBitmap(screenshot);
}

void ZXSpectrumGLView::GaussianBlur(std::vector<Color>& image, int width, int height, float sigma) {
	std::vector<Color> tempImage = image;
	int kernelSize = static_cast<int>(ceil(sigma * 3) * 2 + 1);
	std::vector<float> kernel(kernelSize);
	float sum = 0.0f;
	
	for (int i = 0; i < kernelSize; ++i) {
		float x = i - kernelSize / 2;
		kernel[i] = exp(-(x * x) / (2 * sigma * sigma));
		sum += kernel[i];
	}
	
	for (int i = 0; i < kernelSize; ++i) {
		kernel[i] /= sum;
	}
	
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			float r = 0, g = 0, b = 0;
			for (int i = 0; i < kernelSize; ++i) {
				int sampleX = std::min(std::max(x + i - kernelSize / 2, 0), width - 1);
				const Color& sampleColor = image[y * width + sampleX];
				r += sampleColor.r * kernel[i];
				g += sampleColor.g * kernel[i];
				b += sampleColor.b * kernel[i];
			}
			tempImage[y * width + x] = Color(r, g, b);
		}
	}
	
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			float r = 0, g = 0, b = 0;
			for (int i = 0; i < kernelSize; ++i) {
				int sampleY = std::min(std::max(y + i - kernelSize / 2, 0), height - 1);
				const Color& sampleColor = tempImage[sampleY * width + x];
				r += sampleColor.r * kernel[i];
				g += sampleColor.g * kernel[i];
				b += sampleColor.b * kernel[i];
			}
			image[y * width + x] = Color(r, g, b);
		}
	}
}

void ZXSpectrumGLView::FloydSteinbergDithering(std::vector<Color>& image, int width, int height) {
	const float errorCoeff = 0.8f;

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			Color& oldColor = image[y * width + x];
			int closestColorIndex = FindClosestColor(oldColor);
			const Color& newColor = zxColors[closestColorIndex];
			
			float errorR = (oldColor.r - newColor.r) * errorCoeff;
			float errorG = (oldColor.g - newColor.g) * errorCoeff;
			float errorB = (oldColor.b - newColor.b) * errorCoeff;
			
			if (x + 1 < width) {
				Color& pixel = image[y * width + (x + 1)];
				pixel.r = std::min(255.0f, std::max(0.0f, pixel.r + errorR * 7 / 16));
				pixel.g = std::min(255.0f, std::max(0.0f, pixel.g + errorG * 7 / 16));
				pixel.b = std::min(255.0f, std::max(0.0f, pixel.b + errorB * 7 / 16));
			}

			if (y + 1 < height) {
				if (x > 0) {
					Color& pixel = image[(y + 1) * width + (x - 1)];
					pixel.r = std::min(255.0f, std::max(0.0f, pixel.r + errorR * 3 / 16));
					pixel.g = std::min(255.0f, std::max(0.0f, pixel.g + errorG * 3 / 16));
					pixel.b = std::min(255.0f, std::max(0.0f, pixel.b + errorB * 3 / 16));
				}

				Color& pixel = image[(y + 1) * width + x];
				pixel.r = std::min(255.0f, std::max(0.0f, pixel.r + errorR * 5 / 16));
				pixel.g = std::min(255.0f, std::max(0.0f, pixel.g + errorG * 5 / 16));
				pixel.b = std::min(255.0f, std::max(0.0f, pixel.b + errorB * 5 / 16));

				if (x + 1 < width) {
					Color& pixel = image[(y + 1) * width + (x + 1)];
					pixel.r = std::min(255.0f, std::max(0.0f, pixel.r + errorR * 1 / 16));
					pixel.g = std::min(255.0f, std::max(0.0f, pixel.g + errorG * 1 / 16));
					pixel.b = std::min(255.0f, std::max(0.0f, pixel.b + errorB * 1 / 16));
				}
			}
			oldColor = newColor;
		}
	}
}

void ZXSpectrumGLView::ConvertToZXSpectrum() {
    BRect bounds = screenshot->Bounds();
    int sourceWidth = bounds.IntegerWidth() + 1;
    int sourceHeight = bounds.IntegerHeight() + 1;
    
    std::vector<Color> image;
    image.reserve(sourceWidth * sourceHeight);
    
    uint32* bits = (uint32*)screenshot->Bits();
    int32 bpr = screenshot->BytesPerRow() / 4;
    
    for (int y = 0; y < sourceHeight; ++y) {
        for (int x = 0; x < sourceWidth; ++x) {
            uint32 pixel = bits[y * bpr + x];
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;
            image.emplace_back(r, g, b);
        }
    }

	GaussianBlur(image, sourceWidth, sourceHeight, 0.8f);
	FloydSteinbergDithering(image, sourceWidth, sourceHeight);

	float scaleX = sourceWidth / 256.0f;
	float scaleY = sourceHeight / 192.0f;
	
	for (int blockY = 0; blockY < 24; ++blockY) {
		for (int blockX = 0; blockX < 32; ++blockX) {
			std::vector<Color> block;
			block.reserve(64);
			
			for (int y = 0; y < 8; ++y) {
				for (int x = 0; x < 8; ++x) {
					int sourceX = static_cast<int>((blockX * 8 + x) * scaleX);
					int sourceY = static_cast<int>((blockY * 8 + y) * scaleY);
					block.push_back(image[sourceY * sourceWidth + sourceX]);
				}
			}
			
			uint8_t ink, paper;
			bool bright;
			FindOptimalInkPaper(block, ink, paper, bright);
			
			int attrAddr = 6144 + blockY * 32 + blockX;
			imageBuffer[attrAddr] = (bright ? 0x40 : 0) | (paper << 3) | ink;
			
			for (int y = 0; y < 8; ++y) {
				int addr = ((blockY * 8 + y) & 0xC0) << 5 | ((blockY * 8 + y) & 0x07) << 8 | ((blockY * 8 + y) & 0x38) << 2 | blockX;
				uint8_t byte = 0;
				for (int x = 0; x < 8; ++x) {
					Color pixelColor = block[y * 8 + x];
					int colorIndex = FindClosestColor(pixelColor);
					if ((colorIndex % 8) == ink) {
						byte |= (1 << (7 - x));
					}
				}
				imageBuffer[addr] = byte;
			}
		}
	}
	delete screenshot;
}

void ZXSpectrumGLView::ClearScreen() {
    std::fill(zxMemory.begin(), zxMemory.end(), 0x00);
    std::fill(zxMemory.begin() + 6144, zxMemory.end(), 0x38);
}

void ZXSpectrumGLView::DrawPowerOn() {
    if (loadProgress < 10) {
        if (!isPreview)
        	std::fill(zxMemory.begin(), zxMemory.end(), 0x02);
    } else if (loadProgress < 20) {
        int fillEnd = std::min((loadProgress - 10) * 700, 6912);
        std::fill(zxMemory.begin(), zxMemory.begin() + fillEnd, 0x00);
    } else {
        ClearScreen();
        PrintText("\x7F 1982 Sinclair Research Ltd", 0, 23, 0, 7);
    }
    borderColor = 7;
}

void ZXSpectrumGLView::DrawLoadScreen() {
    ClearScreen();
    if (loadProgress > 0) PrintText("C", 0, 23, 0, 7, false, true);
    if (loadProgress > 50) { PrintText("LOAD ", 0, 23, 0, 7); PrintText("L", 5, 23, 0, 7, false, true); }
    if (loadProgress > 75) { PrintText("\"", 5, 23, 0, 7); PrintText("L", 6, 23, 0, 7, false, true); }
    if (loadProgress > 100) { PrintText("\"", 6, 23, 0, 7); PrintText("L", 7, 23, 0, 7, false, true); }
    if (loadProgress > 110) { PrintText(" ", 7, 23, 0, 7); PrintText("L", 8, 23, 0, 7, false, true); }
    if (loadProgress > 120) { PrintText("E", 8, 23, 0, 7, false, true); }
    if (loadProgress > 150) { PrintText("SCREEN$", 8, 23, 0, 7); PrintText("L", 16, 23, 0, 7, false, true); }
    if (loadProgress > 190) ClearScreen();
    borderColor = 7;
}

void ZXSpectrumGLView::DrawBorderFlash() {
    borderColor = loadProgress % 40 < 20 ? 2 : 5;
    borderStripeWidth = 0.0f;
}

void ZXSpectrumGLView::DrawPilotTone() {
    pilotToneOffset += 0.005f;
    if (pilotToneOffset > 1.0f) {
        pilotToneOffset -= 1.0f;
    }
    borderStripeWidth = 0.05f;
}

void ZXSpectrumGLView::DrawRandomBorder() {
    borderColor = rand() % 8;
    borderStripeWidth = (rand() % 100) / 1000.0f + 0.01f;
}

void ZXSpectrumGLView::DrawByteScreen() {
	if (loadingError) {
        ClearScreen();
        PrintText("R Tape loading error, 0:1", 0, 23, 0, 7);
        loadProgress = 0;
        borderStripeWidth = 0.0f;
        borderColor = 7;
    } else {
	    ClearScreen();
	    PrintText("Bytes:", 0, 1, 0, 7);
	    PrintText(fileName.c_str(), 7, 1, 0, 7);
	    borderColor = loadProgress % 40 < 15 ? 2 : 5;
    }
}

void ZXSpectrumGLView::DrawImageLoading() {
    if (imageLoaded) {
    	pilotToneOffset += 0.001f;
        uint32_t copyEnd = std::min(loadProgress * 10, 6912);
        std::copy(imageBuffer.begin(), imageBuffer.begin() + copyEnd, zxMemory.begin());
        
        if (copyEnd < 6912) {
            uint8_t currentByte = (copyEnd < imageBuffer.size()) ? imageBuffer[copyEnd] : 0;
            borderStripeWidth = (currentByte / 255.0f) * 0.1f + 0.01f;
        } else {
            borderStripeWidth = 0.05f;
        }
    } else {
        borderStripeWidth = 0.05f;
    }
    borderColor = 5;
}

int ZXSpectrumGLView::GetStageLength(int stage) {
    switch (stage) {
        case 0: return 100;     // Power on
        case 1: return 200;     // LOAD ""
        case 2: return 50;      // Border flash
        case 3: return 120;     // Pilot tone
        case 4: return 10;      // Random lines
        case 5: return 100;     // Bytes: [Filename]
        case 6: return 120;     // Pilot tone
        case 7: return 700;     // Image loading
        case 8: return 50;      // Border flash
        case 9: return 120;     // Pilot tone
        default: return std::numeric_limits<int>::max();
    }
}

float ZXSpectrumGLView::ColorDistance(const Color& c1, const Color& c2) {
    float dr = c1.r - c2.r;
    float dg = c1.g - c2.g;
    float db = c1.b - c2.b;
    return std::sqrt(dr*dr + dg*dg + db*db);
}

int ZXSpectrumGLView::FindClosestColor(const Color& color) {
    int closestIndex = 0;
    float minDistance = std::numeric_limits<float>::max();
    
    for (int i = 0; i < 16; ++i) {
        float distance = ColorDistance(color, zxColors[i]);
        if (distance < minDistance) {
            minDistance = distance;
            closestIndex = i;
        }
    }
    
    return closestIndex;
}

void ZXSpectrumGLView::PrintChar(char c, int x, int y, uint8_t ink, uint8_t paper, bool bright, bool flash) {
    if (x < 0 || x >= 32 || y < 0 || y >= 24) {
        return;
    }

    uint8_t charIndex = static_cast<uint8_t>(c) - 32;
    if (charIndex >= 96) {
        return;
    }

    for (int row = 0; row < 8; ++row) {
        uint8_t charRow = zx_font[charIndex][row];
        for (int col = 0; col < 8; ++col) {
            int pixelX = x * 8 + col;
            int pixelY = y * 8 + row;
            int pixelAddr = ((pixelY & 0xC0) << 5) | ((pixelY & 0x07) << 8) | ((pixelY & 0x38) << 2) | (pixelX >> 3);
            int bit = 7 - (pixelX & 0x07);

            if (charRow & (1 << bit)) {
                zxMemory[pixelAddr] |= (1 << bit);
            } else {
                zxMemory[pixelAddr] &= ~(1 << bit);
            }
        }
    }

    int attrAddr = 6144 + y * 32 + x;
    zxMemory[attrAddr] = (paper << 3) | ink | (bright << 6) | (flash << 7);
}

void ZXSpectrumGLView::PrintText(const char* text, int x, int y, uint8_t ink, uint8_t paper, bool bright, bool flash) {
    int currentX = x;
    int currentY = y;
    
    for (int i = 0; text[i] != '\0'; ++i) {
        if (currentX >= 32) {
            currentX = 0;
            currentY++;
            if (currentY >= 24)
                break;
        }
        
        PrintChar(text[i], currentX, currentY, ink, paper, bright, flash);
        currentX++;
    }
}

void ZXSpectrumGLView::FindOptimalInkPaper(const std::vector<Color>& block, uint8_t& ink, uint8_t& paper, bool& bright) {
    std::vector<int> colorCounts(16, 0);
    for (const auto& color : block) {
        ++colorCounts[FindClosestColor(color)];
    }
    
    int maxCount = 0, secondMaxCount = 0;
    int paperIndex = 0, inkIndex = 0;
    
    for (int i = 0; i < 16; ++i) {
        if (colorCounts[i] > maxCount) {
            secondMaxCount = maxCount;
            inkIndex = paperIndex;
            maxCount = colorCounts[i];
            paperIndex = i;
        } else if (colorCounts[i] > secondMaxCount) {
            secondMaxCount = colorCounts[i];
            inkIndex = i;
        }
    }
    
    bright = (paperIndex > 7) || (inkIndex > 7);
    ink = inkIndex % 8;
    paper = paperIndex % 8;
}

void ZXSpectrumGLView::CheckShaderCompileStatus(GLuint shader) {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void ZXSpectrumGLView::CheckShaderLinkStatus(GLuint program) {
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}

void ZXSpectrumGLView::FrameResized(float width, float height) {
    LockGL();
    glViewport(0, 0, width, height);
    fWidth = width;
    fHeight = height;
    UnlockGL();
}

void ZXSpectrumGLView::CleanupGL() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shaderProgram);
}
