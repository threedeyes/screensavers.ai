/*
 * Lava Lamp Screensaver
 *
 * This screensaver simulates the mesmerizing motion of a lava lamp,
 * creating a relaxing and nostalgic ambiance on your screen.
 *
 * ©2024 Claude 3.5 Sonnet by Anthropic
 *
 * Designed and implemented by Claude, an AI assistant created by Anthropic,
 * demonstrating the capabilities of artificial intelligence in software development.
 * The code was generated based on the user's requirements and best practices for Haiku OS development.
 */

#include <ScreenSaver.h>
#include <LayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <StringView.h>
#include <TextView.h>
#include <ScrollView.h>
#include <CheckBox.h>
#include <PopUpMenu.h>
#include <TabView.h>
#include <Message.h>
#include <Slider.h>
#include <Screen.h>
#include <Bitmap.h>
#include <GLView.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>

class LavaLampScreenSaver;
class LavaLampConfigView;
class LavaLampGLView;

enum ColorMode {
    COLOR_MODE_LAVA,
    COLOR_MODE_RANDOM,
    COLOR_MODE_DYNAMIC
};

struct Blob {
    float x, y;
    float vx, vy;
    float radius;
    uint32_t color;
};

struct Bubble {
    float x, y;
    float speed;
    float size;
};

class LavaLampScreenSaver : public BScreenSaver {
public:
    LavaLampScreenSaver(BMessage* archive, image_id image);
    void StartConfig(BView* view);
    status_t SaveState(BMessage* into) const;
    void RestoreState(BMessage* from);
    status_t StartSaver(BView* view, bool preview);
    void Draw(BView* view, int32 frame);

    // Setters
    void SetColorMode(ColorMode mode);
    void SetDesktopBackground(bool enabled);
    void SetBubbles(bool enabled);
    void SetBlobSize(float size);
    void SetBlobCount(int count);
    void SetBubbleCount(int count);
    void SetSpeed(float speed);
    void SetLastTab(int tab);

    // Getters
    ColorMode GetColorMode() const { return fColorMode; }
    bool GetDesktopBackground() const { return fDesktopBackground; }
    bool GetBubbles() const { return fBubbles; }
    float GetBlobSize() const { return fBlobSize; }
    int GetBlobCount() const { return fBlobCount; }
    int GetBubbleCount() const { return fBubbleCount; }
    float GetSpeed() const { return fSpeed; }
    int GetLastTab() const { return fLastTab; }

private:
    LavaLampGLView* fGLView;
    ColorMode fColorMode;
    bool fDesktopBackground;
    bool fBubbles;
    float fBlobSize;
    int fBlobCount;
    int fBubbleCount;
    float fSpeed;
    int fLastTab;
};


class LavaLampConfigView : public BView {
public:
    LavaLampConfigView(BRect frame, LavaLampScreenSaver* saver);
    void AttachedToWindow() override;
    void MessageReceived(BMessage* message) override;

    enum {
		MSG_COLOR_MODE = 'colm',
        MSG_DESKTOP_BG = 'dtdb',
        MSG_BUBBLES = 'bubl',
        MSG_BLOB_SIZE = 'blbs',
        MSG_BLOB_COUNT = 'blbc',
        MSG_BUBBLE_COUNT = 'bubc',
        MSG_SPEED = 'sped'
    };

private:
    LavaLampScreenSaver* fSaver;
    BTabView* fTabView;
	BMenuField* fColorModeMenu;
    BCheckBox* fDesktopBackgroundCB;    
    BCheckBox* fBubblesCB;
    BSlider* fBlobSizeSlider;
    BSlider* fBlobCountSlider;
    BSlider* fBubbleCountSlider;
    BSlider* fSpeedSlider;
};

class LavaLampGLView : public BGLView {
public:
    LavaLampGLView(BRect frame,  bool preview);
    void AttachedToWindow() override;
    void Draw();
    void Update();
    void SetColorMode(ColorMode mode);
    void SetDesktopBackground(bool enabled);
    void SetBubbles(bool enabled);
    void SetBlobSize(float size);
    void SetBlobCount(int count);
    void SetBubbleCount(int count);
    void SetSpeed(float speed);

private:
    static const int TEXTURE_WIDTH = 256;
    static const int TEXTURE_HEIGHT = 256;

    float fWidth, fHeight;
    bool fPreview;
    std::vector<Blob> blobs;
    std::vector<Bubble> bubbles;
    std::mt19937 rng;
    GLuint textureId;
    GLuint fBackgroundTextureId;
    std::vector<uint32_t> textureData;
    float colorPhase;
    ColorMode fColorMode;
    bool fDesktopBackground;
    bool fBubbles;
    int fBlobCount;
    float fBlobSize;
    int fBubbleCount;
    float fSpeed;
    float fScale;
    BScreen* fScreen;
    BBitmap* fBackgroundBitmap;

    float randomFloat();
    uint32_t randomColor();
    void initBlobs();
    void initBubbles();
    void createTexture();
    void createBackgroundTexture();
    void drawBackground();
    void updateMetaballsTexture();
    void drawMetaballsTexture();
    void updateBlobColors();
    void updateBubbles();
    void drawBubbles();
    uint32_t blendColors(uint32_t color1, uint32_t color2, float t);
    uint32_t hsvToRgb(float h, float s, float v);
};

// Implementation of LavaLampScreenSaver methods

LavaLampScreenSaver::LavaLampScreenSaver(BMessage* archive, image_id image)
    : BScreenSaver(archive, image),
      fGLView(nullptr),
	  fColorMode(COLOR_MODE_LAVA),
      fDesktopBackground(true),
      fBubbles(true),
      fBlobSize(10.0f),
      fBlobCount(10),
      fBubbleCount(100),
      fSpeed(1.0f) {
	RestoreState(archive);
}

void LavaLampScreenSaver::StartConfig(BView* view) {
    if (view) {
        view->AddChild(new LavaLampConfigView(view->Bounds(), this));
    }
}

status_t LavaLampScreenSaver::SaveState(BMessage* into) const {
    if (into) {
        into->AddInt32("color_mode", static_cast<int32>(fColorMode));
        into->AddBool("desktop_bg", fDesktopBackground);
        into->AddBool("bubbles", fBubbles);
        into->AddFloat("blob_size", fBlobSize);
        into->AddInt32("blob_count", fBlobCount);
        into->AddInt32("bubble_count", fBubbleCount);
        into->AddFloat("speed", fSpeed);
        into->AddInt32("last_tab", fLastTab);
    }
    return B_OK;
}

void LavaLampScreenSaver::RestoreState(BMessage* from) {
    if (from != nullptr) {
		int32 mode;
        if (from->FindInt32("color_mode", &mode) == B_OK)
            fColorMode = static_cast<ColorMode>(mode);
        else
        	fColorMode = COLOR_MODE_LAVA;

        if (from->FindBool("desktop_bg", &fDesktopBackground) != B_OK)
        	fDesktopBackground = true;

        if (from->FindBool("bubbles", &fBubbles) != B_OK)
        	fBubbles = true;

        if (from->FindFloat("blob_size", &fBlobSize) != B_OK)
        	fBlobSize = 10.0f;

        if (from->FindInt32("blob_count", &fBlobCount) != B_OK)
        	fBlobCount = 10;

        if (from->FindInt32("bubble_count", &fBubbleCount) != B_OK)
        	fBubbleCount = 100;

        if (from->FindFloat("speed", &fSpeed) != B_OK)
        	fSpeed = 1.0f;

        if (from->FindInt32("last_tab", &fLastTab) != B_OK)
        	fLastTab = 0;
    }
}

status_t LavaLampScreenSaver::StartSaver(BView* view, bool preview) {
	view->SetViewColor(0, 0, 0);
    if (view) {
        fGLView = new LavaLampGLView(view->Bounds(), preview);
        fGLView->SetViewColor(0, 0, 0);
        fGLView->SetDesktopBackground(fDesktopBackground);
        fGLView->SetColorMode(fColorMode);
        fGLView->SetBubbles(fBubbles);
        fGLView->SetBubbleCount(fBubbleCount);
        fGLView->SetBlobSize(fBlobSize);
        fGLView->SetBlobCount(fBlobCount);
        fGLView->SetSpeed(fSpeed);
        view->AddChild(fGLView);
    }
    SetTickSize(25000);
    return B_OK;
}

void LavaLampScreenSaver::Draw(BView* view, int32 frame) {
    if (fGLView) {
        fGLView->Update();
        fGLView->Draw();
    }
}

void LavaLampScreenSaver::SetColorMode(ColorMode mode) {
    fColorMode = mode;
    if (fGLView) fGLView->SetColorMode(mode);
}

void LavaLampScreenSaver::SetDesktopBackground(bool enabled) {
    fDesktopBackground = enabled;
    if (fGLView) fGLView->SetDesktopBackground(enabled);
}

void LavaLampScreenSaver::SetBubbles(bool enabled) {
    fBubbles = enabled;
    if (fGLView) fGLView->SetBubbles(enabled);
}

void LavaLampScreenSaver::SetBlobSize(float size) {
    fBlobSize = size;
    if (fGLView) fGLView->SetBlobSize(size);
}

void LavaLampScreenSaver::SetBlobCount(int count) {
    fBlobCount = count;
    if (fGLView) fGLView->SetBlobCount(count);
}

void LavaLampScreenSaver::SetBubbleCount(int count) {
    fBubbleCount = count;
    if (fGLView) fGLView->SetBubbleCount(count);
}

void LavaLampScreenSaver::SetSpeed(float speed) {
    fSpeed = speed;
    if (fGLView) fGLView->SetSpeed(speed);
}

void LavaLampScreenSaver::SetLastTab(int tab) {
    fLastTab = tab;
}

// Implementation of LavaLampConfigView methods

LavaLampConfigView::LavaLampConfigView(BRect frame, LavaLampScreenSaver* saver)
    : BView(frame, "LavaLampConfigView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
      fSaver(saver) {
    SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

    // Create tab view
    fTabView = new BTabView("tabView", B_WIDTH_FROM_WIDEST);

    // Create tabs
    BView* generalTab = new BView("General", B_WILL_DRAW);
    BView* blobsTab = new BView("Blobs", B_WILL_DRAW);
    BView* bubblesTab = new BView("Bubbles", B_WILL_DRAW);

    // Set up layouts for each tab
    generalTab->SetLayout(new BGroupLayout(B_VERTICAL));
    blobsTab->SetLayout(new BGroupLayout(B_VERTICAL));
    bubblesTab->SetLayout(new BGroupLayout(B_VERTICAL));

    // General Tab
    BGroupLayout* generalLayout = dynamic_cast<BGroupLayout*>(generalTab->GetLayout());
    generalLayout->SetInsets(B_USE_DEFAULT_SPACING);

    // Create title
    BStringView* titleView = new BStringView("titleView", "Lava Lamp Screensaver");
    titleView->SetFont(be_bold_font);

    // Create desktop background checkbox
    fDesktopBackgroundCB = new BCheckBox(
        "desktopBg", 
        "Use desktop as background", 
        new BMessage(MSG_DESKTOP_BG));
    fDesktopBackgroundCB->SetValue(fSaver->GetDesktopBackground());

    // Create speed slider
    fSpeedSlider = new BSlider("speed", "Animation Speed:", 
        new BMessage(MSG_SPEED), 1, 20, B_HORIZONTAL);
    fSpeedSlider->SetValue(fSaver->GetSpeed() * 5);
    fSpeedSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
    fSpeedSlider->SetHashMarkCount(10);
    fSpeedSlider->SetLimitLabels("0%", "100%");

    // Create info text view
    BRect textRect = Bounds();
    textRect.InsetBy(5, 5);
    BTextView* infoTextView = new BTextView(textRect, "infoTextView", 
        textRect, B_FOLLOW_ALL_SIDES);
    infoTextView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
    infoTextView->MakeEditable(false);
    infoTextView->SetStylable(true);

    infoTextView->Insert("©2024 Claude 3.5 Sonnet by Anthropic\n\n");
    infoTextView->Insert("This screensaver simulates the mesmerizing motion of a lava lamp, ");
    infoTextView->Insert("creating a relaxing and nostalgic ambiance on your screen.\n\n");
    infoTextView->Insert("This screen saver was designed and implemented by Claude, an AI assistant created by Anthropic, ");
    infoTextView->Insert("demonstrating the capabilities of artificial intelligence in software development.\n");
    infoTextView->Insert("The code was generated based on the user's requirements and best practices for Haiku development.\n");

    BScrollView* infoScrollView = new BScrollView("infoScrollView", infoTextView, 
        B_WILL_DRAW | B_FRAME_EVENTS, false, true);

    generalLayout->AddView(titleView);
    generalLayout->AddView(fDesktopBackgroundCB);
    generalLayout->AddView(fSpeedSlider);
    generalLayout->AddView(infoScrollView);

    // Blobs Tab
    BGroupLayout* blobsLayout = dynamic_cast<BGroupLayout*>(blobsTab->GetLayout());
    blobsLayout->SetInsets(B_USE_DEFAULT_SPACING);

    // Create color mode menu
    BPopUpMenu* colorMenu = new BPopUpMenu("Color Mode");
    colorMenu->AddItem(new BMenuItem("Lava", new BMessage(MSG_COLOR_MODE)));
    colorMenu->AddItem(new BMenuItem("Random", new BMessage(MSG_COLOR_MODE)));
    colorMenu->AddItem(new BMenuItem("Dynamic", new BMessage(MSG_COLOR_MODE)));
    
    // Set the correct item as marked based on current mode
    BMenuItem* item = colorMenu->ItemAt(static_cast<int32>(fSaver->GetColorMode()));
    if (item) item->SetMarked(true);
    
    fColorModeMenu = new BMenuField("colorMode", "Color Mode:", colorMenu);

    // Create blob settings
    fBlobCountSlider = new BSlider("blobCount", "Blob count:", 
        new BMessage(MSG_BLOB_COUNT), 1, 30, B_HORIZONTAL);
    fBlobCountSlider->SetValue(fSaver->GetBlobCount());
    fBlobCountSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
    fBlobCountSlider->SetHashMarkCount(30);
    fBlobCountSlider->SetLimitLabels("1", "30");

    fBlobSizeSlider = new BSlider("blobSize", "Blob size:", 
        new BMessage(MSG_BLOB_SIZE), 5, 20, B_HORIZONTAL);
    fBlobSizeSlider->SetValue(fSaver->GetBlobSize());
    fBlobSizeSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
    fBlobSizeSlider->SetHashMarkCount(15);
    fBlobSizeSlider->SetLimitLabels("5%", "20%");

    blobsLayout->AddView(fColorModeMenu);
    blobsLayout->AddView(fBlobCountSlider);
    blobsLayout->AddView(fBlobSizeSlider);
    blobsLayout->AddItem(BSpaceLayoutItem::CreateGlue());

    // Bubbles Tab
    BGroupLayout* bubblesLayout = dynamic_cast<BGroupLayout*>(bubblesTab->GetLayout());
    bubblesLayout->SetInsets(B_USE_DEFAULT_SPACING);

    fBubblesCB = new BCheckBox("bubbles", "Enable Bubbles", new BMessage(MSG_BUBBLES));
    fBubblesCB->SetValue(fSaver->GetBubbles());
    
    fBubbleCountSlider = new BSlider("bubbleCount", "Bubble count:", 
        new BMessage(MSG_BUBBLE_COUNT), 1, 500, B_HORIZONTAL);
    fBubbleCountSlider->SetValue(fSaver->GetBubbleCount());
    fBubbleCountSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
    fBubbleCountSlider->SetHashMarkCount(25);
    fBubbleCountSlider->SetLimitLabels("1", "500");

    bubblesLayout->AddView(fBubblesCB);
    bubblesLayout->AddView(fBubbleCountSlider);
    bubblesLayout->AddItem(BSpaceLayoutItem::CreateGlue());

    // Add tabs to tab view
    fTabView->AddTab(generalTab);
    fTabView->AddTab(blobsTab);
    fTabView->AddTab(bubblesTab);
    fTabView->Select(fSaver->GetLastTab());

    // Set up main layout
    BGroupLayout* mainLayout = new BGroupLayout(B_VERTICAL);
    SetLayout(mainLayout);
    mainLayout->SetInsets(B_USE_DEFAULT_SPACING);

    // Add tab view to main layout
    mainLayout->AddView(fTabView);
}

void LavaLampConfigView::AttachedToWindow() {
    fColorModeMenu->Menu()->SetTargetForItems(this);
    fDesktopBackgroundCB->SetTarget(this);
    fBubblesCB->SetTarget(this);
    fBlobSizeSlider->SetTarget(this);
    fBlobCountSlider->SetTarget(this);
    fBubbleCountSlider->SetTarget(this);
    fSpeedSlider->SetTarget(this);
}

void LavaLampConfigView::MessageReceived(BMessage* message) {
	message->PrintToStream();
    switch (message->what) {
		case MSG_COLOR_MODE: {
            BMenuItem* item = fColorModeMenu->Menu()->FindMarked();
            if (item) {
                int32 index = fColorModeMenu->Menu()->IndexOf(item);
                fSaver->SetColorMode(static_cast<ColorMode>(index));
                fSaver->SetLastTab(fTabView->Selection());
            }
            break;
        }
        case MSG_DESKTOP_BG:
			fSaver->SetDesktopBackground(message->FindInt32("be:value") == B_CONTROL_ON);
			fSaver->SetLastTab(fTabView->Selection());
			break;
        case MSG_BUBBLES:
			fSaver->SetBubbles(message->FindInt32("be:value") == B_CONTROL_ON);
			fSaver->SetLastTab(fTabView->Selection());
			break;
        case MSG_BLOB_SIZE:
            fSaver->SetBlobSize(message->FindInt32("be:value"));
            fSaver->SetLastTab(fTabView->Selection());
            break;
        case MSG_BLOB_COUNT:
            fSaver->SetBlobCount(message->FindInt32("be:value"));
            fSaver->SetLastTab(fTabView->Selection());
            break;
        case MSG_BUBBLE_COUNT:
            fSaver->SetBubbleCount(message->FindInt32("be:value"));
            fSaver->SetLastTab(fTabView->Selection());
            break;
        case MSG_SPEED:
            fSaver->SetSpeed(message->FindInt32("be:value") / 5.0f);
            fSaver->SetLastTab(fTabView->Selection());
            break;
        default:
            BView::MessageReceived(message);
    }
}

// Implementation of LavaLampGLView methods

LavaLampGLView::LavaLampGLView(BRect frame, bool preview)
    : BGLView(frame, "LavaLampGLView", B_FOLLOW_ALL, B_WILL_DRAW, BGL_RGB | BGL_DOUBLE | BGL_DEPTH),
      fWidth(frame.Width() + 1),
      fHeight(frame.Height() + 1),
      fPreview(preview),
      fBlobSize(10.0f),
      fBlobCount(10),
      fBubbleCount(100),
      fSpeed(1.0f),
      fBackgroundTextureId(0),
      colorPhase(0.0f),
      fColorMode(COLOR_MODE_LAVA),
      fDesktopBackground(true),
      fBubbles(false),
      fScreen(nullptr),
      fBackgroundBitmap(nullptr) {
    std::random_device rd;
    rng = std::mt19937(rd());

    fScale = fWidth / BScreen(B_MAIN_SCREEN_ID).Frame().Width();

    initBlobs();
    initBubbles();
    createTexture();
}

void LavaLampGLView::AttachedToWindow() {
    LockGL();
    glViewport(0, 0, fWidth, fHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, fWidth, fHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    fScreen = new BScreen(B_MAIN_SCREEN_ID);
    fBackgroundBitmap = new BBitmap(fScreen->Frame(), B_RGB32);
    fScreen->ReadBitmap(fBackgroundBitmap);
    createBackgroundTexture();

    UnlockGL();

    Draw();
}

void LavaLampGLView::Draw() {
    LockGL();

    glClear(GL_COLOR_BUFFER_BIT);

    drawBackground();
    updateMetaballsTexture();
    drawMetaballsTexture();
    if (fBubbles) {
        drawBubbles();
    }

    SwapBuffers();
    UnlockGL();
}

void LavaLampGLView::Update() {
    // Blob interaction
    for (size_t i = 0; i < blobs.size(); ++i) {
        for (size_t j = i + 1; j < blobs.size(); ++j) { 
            Blob &blob1 = blobs[i];
            Blob &blob2 = blobs[j];

            float dx = blob2.x - blob1.x;
            float dy = blob2.y - blob1.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < (blob1.radius + blob2.radius) * 0.8f) { 
                float force = (200.0f * fScale ) / (distance * distance + 1.0f); 

                blob1.vx -= force * dx / distance;
                blob1.vy -= force * dy / distance;
                blob2.vx += force * dx / distance;
                blob2.vy += force * dy / distance;
            }
        }
    }

    // Blob interaction with bubbles
    for (Blob &blob : blobs) {
        for (Bubble &bubble : bubbles) {
            float dx = bubble.x - blob.x;
            float dy = bubble.y - blob.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < (blob.radius + bubble.size) * 0.8f) { 
                float force = (5000.0f * fScale * fScale) / (distance * distance + 1.0f);
                bubble.x += force * dx / distance;
                bubble.y += force * dy / distance;
            }
        }
    }

    // Updating the positions and speeds of blobs
    for (auto& blob : blobs) {
        blob.x += blob.vx * fSpeed * fScale;
        blob.y += blob.vy * fSpeed * fScale;

        // Speed ​​limit
        float maxSpeed = 5.0f * fSpeed;
        blob.vx = std::max(std::min(blob.vx, maxSpeed), -maxSpeed);
        blob.vy = std::max(std::min(blob.vy, maxSpeed), -maxSpeed);

        // Processing of reflection with the edges of the screen for blobs
        if (blob.x < 0 || blob.x > fWidth) blob.vx *= -1;
        if (blob.y < 0 || blob.y > fHeight) blob.vy *= -1;
    }

    // Updating the positions of bubbles
    if (fBubbles) {
        for (auto& bubble : bubbles) {
            bubble.y -= bubble.speed * fSpeed * fScale;
            if (bubble.y < 0) {
                bubble.y = fHeight;
                bubble.x = randomFloat() * fWidth;
            }
        }
    }

    // Blob flowers update (dynamic mode)
    if (fColorMode == COLOR_MODE_DYNAMIC) {
        colorPhase += 0.01f * fSpeed;
        if (colorPhase > 2 * M_PI) colorPhase -= 2 * M_PI;
        updateBlobColors();
    }
}

void LavaLampGLView::SetColorMode(ColorMode mode) {
	fColorMode = mode;
	initBlobs();
}

void LavaLampGLView::SetBubbles(bool enabled) {
	fBubbles = enabled;
	initBubbles();
}

void LavaLampGLView::SetBlobCount(int count) {
	fBlobCount = count;
	initBlobs();
}

void LavaLampGLView::SetBlobSize(float size) {
    fBlobSize = size;
    initBlobs();
}

void LavaLampGLView::SetBubbleCount(int count) {
	fBubbleCount = count;
	initBubbles();
}

void LavaLampGLView::SetSpeed(float speed) {
	fSpeed = speed;
}

void LavaLampGLView::SetDesktopBackground(bool enabled) {
    fDesktopBackground = enabled;
    if (enabled && !fBackgroundTextureId) {
        createBackgroundTexture();
    }
}

float LavaLampGLView::randomFloat() {
    return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng);
}

uint32_t LavaLampGLView::randomColor() {
    switch (fColorMode) {
        case COLOR_MODE_LAVA:
        {
			uint8_t r = static_cast<uint8_t>(randomFloat() * 155 + 100);
			uint8_t g = static_cast<uint8_t>(randomFloat() * 100);
			uint8_t b = static_cast<uint8_t>(randomFloat() * 50);
			return (r << 24) | (g << 16) | (b << 8) | 0xFF;
        }
        case COLOR_MODE_DYNAMIC:
        {
            return hsvToRgb(randomFloat(), 0.8f, 1.0f);
        }
        case COLOR_MODE_RANDOM:
        default:
        {
            uint8_t r = static_cast<uint8_t>(randomFloat() * 255);
            uint8_t g = static_cast<uint8_t>(randomFloat() * 255);
            uint8_t b = static_cast<uint8_t>(randomFloat() * 255);
            return (r << 24) | (g << 16) | (b << 8) | 0xFF;
        }
    }
}

void LavaLampGLView::initBlobs() {
    blobs.clear();
    float baseRadius = (fWidth * fBlobSize) / 100.0f;  // Convert percentage to actual size

    for (int i = 0; i < fBlobCount; ++i) {
        Blob blob;
        blob.x = randomFloat() * fWidth;
        blob.y = randomFloat() * fHeight;
        blob.vx = (randomFloat() - 0.5f) * 2.0f;
        blob.vy = (randomFloat() - 0.5f) * 2.0f;
        // Vary the radius around the base size
        blob.radius = baseRadius * (0.5f + randomFloat());
        blob.color = randomColor();
        blobs.push_back(blob);
    }
}

void LavaLampGLView::initBubbles() {
    bubbles.clear();
    for (int i = 0; i < fBubbleCount; ++i) {
        Bubble bubble;
        bubble.x = randomFloat() * fWidth;
        bubble.y = randomFloat() * fHeight;
        bubble.speed = 0.5f + randomFloat() * 1.0f;        

        if (fPreview)
        	bubble.size = randomFloat() * 2.0f;
		else
        	bubble.size = (2.0f + randomFloat() * 5.0f) * fScale;

        bubbles.push_back(bubble);
    }
}

void LavaLampGLView::createTexture() {
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    textureData.resize(TEXTURE_WIDTH * TEXTURE_HEIGHT);
}

void LavaLampGLView::createBackgroundTexture() {
    if (fBackgroundBitmap) {
        glGenTextures(1, &fBackgroundTextureId);
        glBindTexture(GL_TEXTURE_2D, fBackgroundTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fBackgroundBitmap->Bounds().Width() + 1, 
                     fBackgroundBitmap->Bounds().Height() + 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, 
                     fBackgroundBitmap->Bits());
    }
}

void LavaLampGLView::drawBackground() {
    if (fDesktopBackground && fBackgroundTextureId) {
        // Screenshot texture
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, fBackgroundTextureId);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(0, 0);
        glTexCoord2f(1, 0); glVertex2f(fWidth, 0);
        glTexCoord2f(1, 1); glVertex2f(fWidth, fHeight);
        glTexCoord2f(0, 1); glVertex2f(0, fHeight);
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
    } else {
        // Gradient
        glBegin(GL_QUADS);
        // Top part (darker)
        glColor3f(0.05f, 0.05f, 0.1f);
        glVertex2f(0, 0);
        glVertex2f(fWidth, 0);
        // Middle part (lighter)
        glColor3f(0.1f, 0.1f, 0.2f);
        glVertex2f(fWidth, fHeight / 2);
        glVertex2f(0, fHeight / 2);
        glEnd();

        glBegin(GL_QUADS);
        // Middle part (lighter)
        glColor3f(0.1f, 0.1f, 0.2f);
        glVertex2f(0, fHeight / 2);
        glVertex2f(fWidth, fHeight / 2);
        // Bottom part (darker)
        glColor3f(0.05f, 0.05f, 0.1f);
        glVertex2f(fWidth, fHeight);
        glVertex2f(0, fHeight);
        glEnd();
    }
}

void LavaLampGLView::updateMetaballsTexture() {
    float scaleX = fWidth / TEXTURE_WIDTH;
    float scaleY = fHeight / TEXTURE_HEIGHT;

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < TEXTURE_HEIGHT; ++y) {
        for (int x = 0; x < TEXTURE_WIDTH; ++x) {
            float sum = 0;
            uint32_t blendedColor = 0;
            float realX = x * scaleX;
            float realY = y * scaleY;

            for (const auto& blob : blobs) {
                float dx = realX - blob.x;
                float dy = realY - blob.y;
                float d2 = dx*dx + dy*dy;
                float contribution = blob.radius * blob.radius / d2;
                sum += contribution;
                
                if (contribution > 0.01f) {
                    float weight = contribution / (sum + 0.01f);
                    blendedColor = blendColors(blendedColor, blob.color, weight);
                }
            }
            
            uint32_t color = 0;
            if (sum > 1.0f) {
                float alpha = std::min((sum - 1.0f) * 255, 255.0f);
                color = (static_cast<uint32_t>(alpha) << 24) | (blendedColor & 0x00FFFFFF);
            }
            textureData[y * TEXTURE_WIDTH + x] = color;
        }
    }

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data());
}

void LavaLampGLView::drawMetaballsTexture() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(fWidth, 0);
    glTexCoord2f(1, 1); glVertex2f(fWidth, fHeight);
    glTexCoord2f(0, 1); glVertex2f(0, fHeight);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void LavaLampGLView::updateBlobColors() {
    for (auto& blob : blobs) {
        float hue = fmodf(colorPhase + blob.x / fWidth, 1.0f);
        blob.color = hsvToRgb(hue, 0.8f, 1.0f);
    }
}

void LavaLampGLView::updateBubbles() {
    for (auto& bubble : bubbles) {
        bubble.y -= bubble.speed * fSpeed;
        if (bubble.y < 0) {
            bubble.y = fHeight;
            bubble.x = randomFloat() * fWidth;
        }
    }
}

void LavaLampGLView::drawBubbles() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& bubble : bubbles) {
        glPushMatrix();
        glTranslatef(bubble.x, bubble.y, 0);
        
        // Draw main bubble body
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
        glVertex2f(0, 0);  // Center
        for (int i = 0; i <= 360; i += 10) {
            float angle = i * M_PI / 180.0f;
            glColor4f(1.0f, 1.0f, 1.0f, 0.1f);  // Fade out at edges
            glVertex2f(cos(angle) * bubble.size, sin(angle) * bubble.size);
        }
        glEnd();
        
        // Draw highlight
        float highlightSize = bubble.size * 0.3f;
        float offsetX = -bubble.size * 0.2f;
        float offsetY = -bubble.size * 0.2f;
        
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
        glVertex2f(offsetX, offsetY);  // Highlight center
        for (int i = 0; i <= 360; i += 10) {
            float angle = i * M_PI / 180.0f;
            glColor4f(1.0f, 1.0f, 1.0f, 0.0f);  // Fade out highlight
            glVertex2f(offsetX + cos(angle) * highlightSize, 
                       offsetY + sin(angle) * highlightSize);
        }
        glEnd();
        
        glPopMatrix();
    }

    glDisable(GL_BLEND);
}

uint32_t LavaLampGLView::blendColors(uint32_t color1, uint32_t color2, float t) {
    uint8_t r1 = (color1 >> 24) & 0xFF;
    uint8_t g1 = (color1 >> 16) & 0xFF;
    uint8_t b1 = (color1 >> 8) & 0xFF;

    uint8_t r2 = (color2 >> 24) & 0xFF;
    uint8_t g2 = (color2 >> 16) & 0xFF;
    uint8_t b2 = (color2 >> 8) & 0xFF;

    uint8_t r = r1 + (r2 - r1) * t;
    uint8_t g = g1 + (g2 - g1) * t;
    uint8_t b = b1 + (b2 - b1) * t;

    return (r << 24) | (g << 16) | (b << 8) | 0xFF;
}

uint32_t LavaLampGLView::hsvToRgb(float h, float s, float v) {
    float r, g, b;
    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    uint8_t rr = static_cast<uint8_t>(r * 255);
    uint8_t gg = static_cast<uint8_t>(g * 255);
    uint8_t bb = static_cast<uint8_t>(b * 255);

    return (rr << 24) | (gg << 16) | (bb << 8) | 0xFF;
}

// Function to create an instance of the screensaver
extern "C" _EXPORT BScreenSaver *instantiate_screen_saver(BMessage *message, image_id image) {
    return new LavaLampScreenSaver(message, image);
}
