/*
 * Snowfall Screensaver for Haiku
 *
 * A customizable snowfall screensaver with interactive snowdrifts.
 * This screensaver simulates falling snowflakes with adjustable parameters
 * such as snowflake count, size, wind speed, and fall speed. It also
 * features an option to display accumulating snowdrifts.
 *
 * Author: Claude (AI Assistant by Anthropic, version 3.5)
 *
 * This code was generated by the Claude AI to demonstrate
 * the capabilities of artificial intelligence in software development
 * for the Haiku operating system.
 */

#include <ScreenSaver.h>
#include <View.h>
#include <GLView.h>
#include <GL/gl.h>
#include <Window.h>
#include <Box.h>
#include <StringView.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <StringView.h>
#include <Slider.h>
#include <CheckBox.h>
#include <TextView.h>
#include <ScrollView.h>
#include <StringView.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cmath>
#include <algorithm>

constexpr int MAX_SNOWFLAKES = 500;
constexpr float MAX_FPS = 60.0f;
constexpr float BRANCH_ANGLE = M_PI / 6.0f;
constexpr float LENGTH_DECREASE = 0.6f;
constexpr int MIN_BRANCH_LEVELS = 1;
constexpr int MAX_BRANCH_LEVELS = 3;

struct Snowflake {
    float x, y;
    float size;
    float speed;
    float angle;
    float angularSpeed;
    float windOffset;
    int branches;
    int branchLevels;
};

class SnowflakeScreenSaver;

class SnowflakeConfigView : public BView {
public:
    SnowflakeConfigView(BRect frame, SnowflakeScreenSaver* saver);
    void AttachedToWindow() override;
    void MessageReceived(BMessage* message) override;

private:
    SnowflakeScreenSaver* fSaver;
    BStringView* fNameStringView;
    BSlider* fCountSlider;
    BSlider* fSizeSlider;
    BSlider* fWindSlider;
    BSlider* fFallSlider;
    BCheckBox* fShowSnowdriftsCheckBox;
	BTextView* fInfoTextView;
};

class SnowflakeScreenSaver : public BScreenSaver {
public:
    SnowflakeScreenSaver(BMessage* message, image_id id);
    void Draw(BView* view, int32 frame) override;
    status_t StartSaver(BView* view, bool preview) override;
    void StopSaver() override;
    void StartConfig(BView* view) override;
    status_t SaveState(BMessage* into) const override;
    void RestoreState(BMessage* from);

    void SetSnowflakeCount(int32 count) { snowflakeCount = std::min(count, MAX_SNOWFLAKES); ApplySettings(); }
    void SetMaxSnowflakeSize(float size) { maxSnowflakeSize = size; minSnowflakeSize = size * 0.5f; ApplySettings(); }
    void SetWindAmplitude(float amplitude) { windAmplitude = amplitude; ApplySettings(); }
    void SetMaxSnowflakeSpeed(float speed) { maxSnowflakeSpeed = speed; minSnowflakeSpeed = speed * 0.5f; ApplySettings(); }
	void SetShowSnowdrifts(bool show) { showSnowdrifts = show; snowdrifts.clear(); ApplySettings(); }
    void ApplySettings();

    int32 GetSnowflakeCount() const { return snowflakeCount; }
    float GetMaxSnowflakeSize() const { return maxSnowflakeSize; }
    float GetWindAmplitude() const { return windAmplitude; }
    float GetMaxSnowflakeSpeed() const { return maxSnowflakeSpeed; }
	bool GetShowSnowdrifts() const { return showSnowdrifts; }

private:
    std::vector<Snowflake> snowflakes;
    std::vector<float> snowdrifts;
    bigtime_t lastFrameTime;
    BGLView* glView;
    int windowWidth, windowHeight;
    float snowflakeResetHeight;

    int32 snowflakeCount;
    float minSnowflakeSize;
    float maxSnowflakeSize;
    float windAmplitude;
    float minSnowflakeSpeed;
    float maxSnowflakeSpeed;
    float maxAngularSpeed;
	bool showSnowdrifts;
	bool seetleActive;

    void InitializeSnowflakes();
    void UpdateSnowflakes(float deltaTime);
    void UpdateSnowdrifts();
    void DrawBranch(float length, int level);
    void DrawSnowflakes();
    void DrawSnowdrifts();
};

extern "C" _EXPORT BScreenSaver* instantiate_screen_saver(BMessage* msg, image_id id)
{
    return new SnowflakeScreenSaver(msg, id);
}

SnowflakeConfigView::SnowflakeConfigView(BRect frame, SnowflakeScreenSaver* saver)
    : BView(frame, "SnowflakeConfigView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
      fSaver(saver)
{
    SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

    fNameStringView = new BStringView("nameString", "A beautiful snowfall screen saver");
    fNameStringView->SetFont(be_bold_font);

    fCountSlider = new BSlider("countSlider", "Snowflake Count:", new BMessage('SNCN'), 50, MAX_SNOWFLAKES, B_HORIZONTAL);
    fSizeSlider = new BSlider("sizeSlider", "Snowflake Size:", new BMessage('SNSZ'), 1, 20, B_HORIZONTAL);
    fWindSlider = new BSlider("windSlider", "Wind Speed:", new BMessage('WNSP'), 0, 200, B_HORIZONTAL);
    fFallSlider = new BSlider("fallSlider", "Fall Speed:", new BMessage('FLSP'), 50, 1500, B_HORIZONTAL);
    fShowSnowdriftsCheckBox = new BCheckBox("showSnowdriftsCheckBox", "Show Snowdrifts", new BMessage('SNDR'));

    // Create info text view
    BRect textRect = Bounds();
    textRect.InsetBy(5, 5);
    fInfoTextView = new BTextView(textRect, "infoTextView", textRect, B_FOLLOW_ALL_SIDES);
    fInfoTextView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
    fInfoTextView->MakeEditable(false);
    fInfoTextView->SetStylable(true);

	// Set the info text
    fInfoTextView->Insert("©2024 Claude 3.5 Sonnet by Anthropic\n\n");
    fInfoTextView->Insert("This screen saver was designed and implemented by Claude, an AI assistant created by Anthropic, ");
    fInfoTextView->Insert("demonstrating the capabilities of artificial intelligence in software development.\n");
    fInfoTextView->Insert("The code was generated based on the user's requirements and best practices for Haiku development.\n");

    BScrollView* infoScrollView = new BScrollView("infoScrollView", fInfoTextView, B_WILL_DRAW | B_FRAME_EVENTS, false, true);

    BGroupLayout* layout = new BGroupLayout(B_VERTICAL);
    SetLayout(layout);

    layout->SetInsets(B_USE_DEFAULT_SPACING);

    layout->AddView(fNameStringView);
    layout->AddView(fCountSlider);
    layout->AddView(fSizeSlider);
    layout->AddView(fWindSlider);
    layout->AddView(fFallSlider);
    layout->AddView(fShowSnowdriftsCheckBox);
    layout->AddView(infoScrollView);
}

void SnowflakeConfigView::AttachedToWindow()
{
    fCountSlider->SetTarget(this);
    fSizeSlider->SetTarget(this);
    fWindSlider->SetTarget(this);
    fFallSlider->SetTarget(this);
    fShowSnowdriftsCheckBox->SetTarget(this);

    fCountSlider->SetValue(fSaver->GetSnowflakeCount());
    fSizeSlider->SetValue(static_cast<int32>(fSaver->GetMaxSnowflakeSize()));
    fWindSlider->SetValue(static_cast<int32>(fSaver->GetWindAmplitude()));
    fFallSlider->SetValue(static_cast<int32>(fSaver->GetMaxSnowflakeSpeed()));
    fShowSnowdriftsCheckBox->SetValue(fSaver->GetShowSnowdrifts());
}

void SnowflakeConfigView::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case 'SNCN': fSaver->SetSnowflakeCount(fCountSlider->Value()); break;
        case 'SNSZ': fSaver->SetMaxSnowflakeSize(fSizeSlider->Value()); break;
        case 'WNSP': fSaver->SetWindAmplitude(fWindSlider->Value()); break;
        case 'FLSP': fSaver->SetMaxSnowflakeSpeed(fFallSlider->Value()); break;
        case 'SNDR': fSaver->SetShowSnowdrifts(fShowSnowdriftsCheckBox->Value() == B_CONTROL_ON); break;
        default: BView::MessageReceived(message); break;
    }   
}

SnowflakeScreenSaver::SnowflakeScreenSaver(BMessage* message, image_id id)
    : BScreenSaver(message, id), lastFrameTime(0), glView(nullptr),
      snowflakeCount(250),
      minSnowflakeSize(2.0f),
      maxSnowflakeSize(10.0f),
      windAmplitude(90.0f),
      minSnowflakeSpeed(80.0f),
      maxSnowflakeSpeed(250.0f),
      maxAngularSpeed(80.0f),
	  showSnowdrifts(true),
	  seetleActive(false)
{
    srand(static_cast<unsigned int>(time(nullptr)));
    RestoreState(message);
    snowflakes.reserve(MAX_SNOWFLAKES);
}

void SnowflakeScreenSaver::StartConfig(BView* view)
{
	SnowflakeConfigView *configView = new SnowflakeConfigView(view->Bounds(), this);
    view->AddChild(configView);
}

void SnowflakeScreenSaver::Draw(BView* view, int32 frame)
{
    if (!glView) return;

    bigtime_t currentTime = system_time();
    float deltaTime = (currentTime - lastFrameTime) / 1000000.0f;
    lastFrameTime = currentTime;

    glView->LockGL();
    
    glClear(GL_COLOR_BUFFER_BIT);

    UpdateSnowflakes(deltaTime);
    if (showSnowdrifts) {
        UpdateSnowdrifts();
    }	
    DrawSnowflakes();
	if (showSnowdrifts) {
		DrawSnowdrifts();
	}

    glView->SwapBuffers();
    glView->UnlockGL();
}

status_t SnowflakeScreenSaver::StartSaver(BView* view, bool preview)
{
    windowWidth = view->Bounds().IntegerWidth() + 1;
    windowHeight = view->Bounds().IntegerHeight() + 1;
    snowflakeResetHeight = windowHeight;

    InitializeSnowflakes();     

    lastFrameTime = system_time();

    SetTickSize(1000000 / MAX_FPS);
    
    BRect bounds = view->Bounds();
    glView = new BGLView(bounds, "OpenGL", B_FOLLOW_ALL_SIDES, 0, BGL_RGB | BGL_DOUBLE | BGL_DEPTH);
    view->AddChild(glView);
    
    glView->LockGL();
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glView->UnlockGL();

    return B_OK;
}

void SnowflakeScreenSaver::StopSaver()
{
}

void SnowflakeScreenSaver::ApplySettings()
{
    InitializeSnowflakes();
}

void SnowflakeScreenSaver::InitializeSnowflakes()
{
    snowflakes.clear();
    snowflakes.resize(snowflakeCount);
    
    float k = 1920.0f / windowWidth;

    for (auto& snowflake : snowflakes) {
        snowflake.x = static_cast<float>(rand() % windowWidth);
        snowflake.y = static_cast<float>(rand() % windowHeight);
        snowflake.size = (minSnowflakeSize + static_cast<float>(rand()) / RAND_MAX * (maxSnowflakeSize - minSnowflakeSize)) / k;
        snowflake.speed = (minSnowflakeSpeed + static_cast<float>(rand()) / RAND_MAX * (maxSnowflakeSpeed - minSnowflakeSpeed)) / k;
        snowflake.angle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
        snowflake.angularSpeed = static_cast<float>(rand()) / RAND_MAX * 2 * maxAngularSpeed - maxAngularSpeed;
        snowflake.windOffset = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
        snowflake.branches = 5 + rand() % 5;
        snowflake.branchLevels = MIN_BRANCH_LEVELS + rand() % (MAX_BRANCH_LEVELS - MIN_BRANCH_LEVELS + 1);
    }

    snowdrifts.resize(windowWidth, 0);    
}

void SnowflakeScreenSaver::UpdateSnowflakes(float deltaTime)
{
    double currentTime = system_time() / 1000000.0;
    double k = 1920.0 / windowWidth;
    
    for (auto& snowflake : snowflakes) {
        snowflake.x += (windAmplitude / k) * std::cos(currentTime + snowflake.windOffset) * deltaTime;
        snowflake.angle += snowflake.angularSpeed * deltaTime;
        snowflake.y -= snowflake.speed * deltaTime;

		if (showSnowdrifts) {
	        if (snowflake.x >= 0 && snowflake.x < windowWidth && snowflake.y < windowHeight) {
	            int xPos = static_cast<int>(snowflake.x);
	            if (xPos >= 0 && xPos < windowWidth && snowflake.y < snowdrifts[xPos]) {
	            	for (int xDrift = xPos - snowflake.size; xDrift <= xPos + snowflake.size; xDrift++) {
	            		if (xDrift >= 0 && xDrift < windowWidth) {
	            			snowdrifts[xDrift]++;
	            		}
	            	}
	                snowflake.y = snowflakeResetHeight;
	                snowflake.x = static_cast<float>(rand() % windowWidth);
	                snowflake.windOffset = static_cast<float>(rand() % 100);
	                continue;
	            }
	        }
		}

        if (snowflake.x < -100 || snowflake.x > windowWidth + 100 || snowflake.y < 0) {
            snowflake.y = snowflakeResetHeight;
            snowflake.x = static_cast<float>(rand() % windowWidth);
            snowflake.windOffset = static_cast<float>(rand() % 100);
        }
    }
}

void SnowflakeScreenSaver::DrawSnowflakes()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    for (const auto& snowflake : snowflakes) {
        glPushMatrix();
        glTranslatef(snowflake.x, snowflake.y, 0);
        glRotatef(snowflake.angle * 180.0f / M_PI, 0, 0, 1);

        float branchLength = snowflake.size;
        for (int i = 0; i < snowflake.branches; ++i) {
            glPushMatrix();
            glRotatef(i * 360.0f / snowflake.branches, 0, 0, 1);
            DrawBranch(branchLength, snowflake.branchLevels);
            glPopMatrix();
        }

        glPopMatrix();
    }
}

void SnowflakeScreenSaver::DrawBranch(float length, int level)
{
    if (level == 0) return;

    glBegin(GL_LINES);
    glVertex2f(0, 0);
    glVertex2f(length, 0);
    glEnd();

    glPushMatrix();
    glTranslatef(length, 0, 0);
    glRotatef(BRANCH_ANGLE * 180.0f / M_PI, 0, 0, 1);
    DrawBranch(length * LENGTH_DECREASE, level - 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(length, 0, 0);
    glRotatef(-BRANCH_ANGLE * 180.0f / M_PI, 0, 0, 1);
    DrawBranch(length * LENGTH_DECREASE, level - 1);
    glPopMatrix();
}

void SnowflakeScreenSaver::DrawSnowdrifts()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int x = 0; x < windowWidth; ++x) {
        glVertex2f(x, 0);
        glVertex2f(x, snowdrifts[x]);
    }
    glEnd();
}

void SnowflakeScreenSaver::UpdateSnowdrifts()
{
    const float maxHeight = windowHeight * 0.5f;
    const float settleFactor = 0.33f;
    bool stopSettle = true;

    for (int i = 0; i < windowWidth; ++i) {
        if (snowdrifts[i] > maxHeight) {
            seetleActive = true;
        }
        if (snowdrifts[i] > maxHeight * settleFactor && seetleActive) {
        	stopSettle = false;  	
        }
    }
    
    if (stopSettle)
    	seetleActive = false;

    if (seetleActive) {
        for (int i = 0; i < windowWidth; ++i) {
            if (snowdrifts[i] > maxHeight * settleFactor * settleFactor)
            	snowdrifts[i] -= rand() % 3;
        }

        std::vector<float> smoothedDrifts = snowdrifts;
        const int smoothRadius = 5;
        for (int i = 0; i < windowWidth; ++i) {
            float sum = 0;
            int count = 0;
            for (int j = -smoothRadius; j <= smoothRadius; ++j) {
                int index = i + j;
                if (index >= 0 && index < windowWidth) {
                    sum += snowdrifts[index];
                    count++;
                }
            }
            smoothedDrifts[i] = sum / count;
        }
        snowdrifts = smoothedDrifts;
    }

    for (int i = 1; i < windowWidth - 1; ++i) {
        float left = snowdrifts[i - 1];
        float right = snowdrifts[i + 1];
        float middle = snowdrifts[i];
        
        snowdrifts[i] = (left + middle + right) / 3.0f;
    }
}

status_t SnowflakeScreenSaver::SaveState(BMessage* into) const
{
    into->AddInt32("snowflakeCount", snowflakeCount);
    into->AddFloat("maxSnowflakeSize", maxSnowflakeSize);
    into->AddFloat("windAmplitude", windAmplitude);
    into->AddFloat("maxSnowflakeSpeed", maxSnowflakeSpeed);
	into->AddBool("showSnowdrifts", showSnowdrifts);
    return B_OK;
}

void SnowflakeScreenSaver::RestoreState(BMessage* from)
{
    if (from != nullptr) {
        if (from->FindInt32("snowflakeCount", &snowflakeCount) != B_OK)
            snowflakeCount = 250;
        if (from->FindFloat("maxSnowflakeSize", &maxSnowflakeSize) != B_OK)
            maxSnowflakeSize = 10.0f;
        if (from->FindFloat("windAmplitude", &windAmplitude) != B_OK)
            windAmplitude = 90.0f;
        if (from->FindFloat("maxSnowflakeSpeed", &maxSnowflakeSpeed) != B_OK)
            maxSnowflakeSpeed = 250.0f;
		if (from->FindBool("showSnowdrifts", &showSnowdrifts) != B_OK)
			showSnowdrifts = true;

        minSnowflakeSize = maxSnowflakeSize / 2;
        minSnowflakeSpeed = maxSnowflakeSpeed / 2;
    }
}