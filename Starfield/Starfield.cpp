/*
 * Starfield Screen Saver
 *
 * This program is a Haiku screensaver that displays a starry sky with orbiting teapots.
 *
 * © Claude (version 3.5 Sonnet) by Anthropic
 *
 * Designed and implemented by Claude, an AI assistant created by Anthropic,
 * demonstrating the capabilities of artificial intelligence in software development.
 * The code was generated based on the user's requirements and best practices for Haiku OS development.
 */

#include <ScreenSaver.h>
#include <GLView.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <String.h>
#include <View.h>
#include <Message.h>
#include <LayoutBuilder.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <Slider.h>
#include <TextView.h>
#include <ScrollView.h>
#include <StringView.h>
#include <vector>
#include <algorithm>

static const int MAX_STARS = 1000;
static const int MAX_TEAPOTS = 10;

// Forward declaration
class ConfigView;

// Spectral classes of stars
enum SpectralClass { O, B, A, F, G, K, M };

struct Star {
    float x, y, z;
    float speed;
    float flicker;
    int supernovaTimer;
    SpectralClass spectralClass;
};

struct Teapot {
    float x, y, z;
    float speed;
    float rotationX, rotationY, rotationZ;
    float rotationSpeedX, rotationSpeedY, rotationSpeedZ;
};

class StarfieldGLView : public BGLView {
public:
    StarfieldGLView(BRect frame, int starsCount, int teapotCount, float starSpeed, float teapotSpeed, bool preview)
        : BGLView(frame, "StarfieldGLView", B_FOLLOW_ALL, B_WILL_DRAW, BGL_RGB | BGL_DOUBLE | BGL_DEPTH),
          fWidth(frame.Width()), fHeight(frame.Height()), kNumStars(starsCount),
          kNumTeapots(teapotCount), kStarMaxSpeed(starSpeed), kTeapotMaxSpeed(teapotSpeed), fIsPreview(preview) {
        srand(time(NULL));
        stars.reserve(kNumStars);
        teapots.reserve(kNumTeapots);
        for (int i = 0; i < kNumStars; ++i) {
            Star star;
            ResetStar(star);
            stars.push_back(star);
        }
        for (int i = 0; i < kNumTeapots; ++i) {
            Teapot teapot;
            ResetTeapot(teapot, true);
            teapots.push_back(teapot);
        }
    }

    void AttachedToWindow() override {
        BGLView::AttachedToWindow();

        LockGL();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_DEPTH_TEST);
        
        // Updated lighting setup
        GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };
        GLfloat light_ambient[] = { 0.1f, 0.1f, 0.1f, 0.5f };
        GLfloat light_diffuse[] = { 0.3f, 0.3f, 0.0f, 0.7f };
        GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        // Setting up light attenuation
        GLfloat light_constant_attenuation = 0.5f;
        GLfloat light_linear_attenuation = 0.1f;
        GLfloat light_quadratic_attenuation = 0.01f;

        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, light_constant_attenuation);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, light_linear_attenuation);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, light_quadratic_attenuation);

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        
        UnlockGL();
    }

	void Draw(BRect updateRect) override {
		LockGL();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// First pass: draw stars
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1, 1, -1, 1, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		DrawStars();

		// Second pass: draw teapots
		glClear(GL_DEPTH_BUFFER_BIT);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0, fWidth / fHeight, 0.1, 100.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glDisable(GL_BLEND);

		DrawTeapots();

		SwapBuffers();
		UnlockGL();
	}

    void SetParameters(int starsCount, int teapotCount, float starSpeed, float teapotSpeed) {       
        kNumStars = std::min(starsCount, MAX_STARS);
        kNumTeapots = std::min(teapotCount, MAX_TEAPOTS);
        kStarMaxSpeed = starSpeed;
        kTeapotMaxSpeed = teapotSpeed;

        stars.clear();
        teapots.clear();
        stars.reserve(kNumStars);
        teapots.reserve(kNumTeapots);

        for (int i = 0; i < kNumStars; ++i) {
            Star star;
            ResetStar(star);
            stars.push_back(star);
        }
        for (int i = 0; i < kNumTeapots; ++i) {
            Teapot teapot;
            ResetTeapot(teapot, true);
            teapots.push_back(teapot);
        }
    }

private:
    int kNumStars;
    int kNumTeapots;
    float kStarMaxSpeed;
    float kTeapotMaxSpeed;
    bool fIsPreview;
    std::vector<Star> stars;
    std::vector<Teapot> teapots;
    float currentColor[3];
    float fWidth, fHeight;

	void DrawStars() {
		float aspectRatio = fHeight / fWidth;
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		
		if (fIsPreview) {
			glPointSize(1.2f);
			glBegin(GL_POINTS);
		}
		
		for (auto& star : stars) {
			MoveStar(star);
			SetStarColor(star);

			float flicker = 0.7f + 0.3f * sinf(star.flicker);
			star.flicker += 0.1f;

			float size = 0.01f * (1.0f - (star.z - 0.1f) / 0.9f);
			
			if (star.supernovaTimer <= 0) {
				flicker = 3.0f;
				size *= 3.0f;
				star.supernovaTimer = rand() % 500 + 200;
				currentColor[0] = 1.0f;
				currentColor[1] = 1.0f;
				currentColor[2] = 1.0f;
			} else {
				star.supernovaTimer -= 1;
			}

			float x = star.x / star.z;
			float y = star.y / star.z;
			
			if (fIsPreview) {
				glColor4f(currentColor[0], currentColor[1], currentColor[2], flicker);
				glVertex3f(x, y, 0);
			} else {
				glBegin(GL_TRIANGLE_FAN);
				
				glColor4f(currentColor[0], currentColor[1], currentColor[2], flicker);
				glVertex3f(x, y, 0);
				
				glColor4f(currentColor[0], currentColor[1], currentColor[2], 0);
				glVertex3f(x, y + size, 0);
				glVertex3f(x + size * aspectRatio, y, 0);
				glVertex3f(x, y - size, 0);
				glVertex3f(x - size * aspectRatio, y, 0);
				glVertex3f(x, y + size, 0);
				
				glEnd();
			}
		}
		
		if (fIsPreview) {
			glEnd();
		}
		
		glDisable(GL_BLEND);
	}

    void DrawTeapots() {
        for (auto& teapot : teapots) {
            MoveTeapot(teapot);
            DrawTeapot(teapot);
        }
    }

    void ResetStar(Star& star) {
        star.x = (float(rand()) / RAND_MAX) * 2 - 1;
        star.y = (float(rand()) / RAND_MAX) * 2 - 1;
        star.z = (float(rand()) / RAND_MAX) * 0.9f + 0.1f;
        star.speed = kStarMaxSpeed * (float(rand()) / RAND_MAX + 0.5f);
        star.spectralClass = static_cast<SpectralClass>(rand() % 7);
        star.flicker = float(rand()) / RAND_MAX * 3.14f * 2.0f;
        star.supernovaTimer = rand() % 100 + 50;
    }

    void MoveStar(Star& star) {
        star.z -= star.speed;
        if (star.z <= 0) {
            ResetStar(star);
        }
    }

    void SetStarColor(const Star& star) {
        switch (star.spectralClass) {
            case O: currentColor[0] = 0.5f; currentColor[1] = 0.5f; currentColor[2] = 1.0f; break;
            case B: currentColor[0] = 0.7f; currentColor[1] = 0.7f; currentColor[2] = 1.0f; break;
            case A: currentColor[0] = 0.8f; currentColor[1] = 0.8f; currentColor[2] = 1.0f; break;
            case F: currentColor[0] = 1.0f; currentColor[1] = 1.0f; currentColor[2] = 1.0f; break;
            case G: currentColor[0] = 1.0f; currentColor[1] = 1.0f; currentColor[2] = 0.8f; break;
            case K: currentColor[0] = 1.0f; currentColor[1] = 0.8f; currentColor[2] = 0.5f; break;
            case M: currentColor[0] = 1.0f; currentColor[1] = 0.5f; currentColor[2] = 0.5f; break;
        }
    }

    void ResetTeapot(Teapot& teapot, bool init) {
        teapot.x = (float(rand()) / RAND_MAX) * 2 - 1;
        teapot.y = (float(rand()) / RAND_MAX) * 2 - 1;
        if (init) {
            teapot.z = 2.0f + (float(rand()) / RAND_MAX) * 25.0f;
        } else {
            teapot.z = 20.0f + (float(rand()) / RAND_MAX) * 10.0f;
        }
        teapot.speed = kTeapotMaxSpeed * (float(rand()) / RAND_MAX + 0.5f);
        teapot.rotationX = teapot.rotationY = teapot.rotationZ = 0.0f;
        teapot.rotationSpeedX = (float(rand()) / RAND_MAX - 0.5f) * 5.0f;
        teapot.rotationSpeedY = (float(rand()) / RAND_MAX - 0.5f) * 5.0f;
        teapot.rotationSpeedZ = (float(rand()) / RAND_MAX - 0.5f) * 5.0f;
    }

    void MoveTeapot(Teapot& teapot) {
        teapot.z -= teapot.speed;
        if (teapot.z <= 0) {
            ResetTeapot(teapot, false);
        }
        teapot.rotationX += teapot.rotationSpeedX;
        teapot.rotationY += teapot.rotationSpeedY;
        teapot.rotationZ += teapot.rotationSpeedZ;
    }

    void DrawTeapot(const Teapot& teapot) {
        glPushMatrix();
        glTranslatef(teapot.x, teapot.y, -teapot.z);
        glRotatef(teapot.rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(teapot.rotationY, 0.0f, 1.0f, 0.0f);
        glRotatef(teapot.rotationZ, 0.0f, 0.0f, 1.0f);
        
        float scale = 0.08f;
        glScalef(scale, scale, scale);

        // Updated material for the teapot
        GLfloat mat_ambient[] = { 0.2f / teapot.z, 0.0f, 0.0f, 1.0f };
        GLfloat mat_diffuse[] = { 5.0f / teapot.z, 0.0f, 0.0f, 1.0f };
        GLfloat mat_specular[] = { 1.0f / teapot.z, 0.5f / teapot.z, 0.5f / teapot.z, 1.0f };
        GLfloat mat_shininess[] = { 50.0f };
        GLfloat mat_emission[] = { 0.2f / teapot.z, 0.0f, 0.0f, 1.0f }; // Adding slight glow

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);

        glutSolidTeapot(1.0);

        glPopMatrix();
    }
};

class StarfieldScreenSaver : public BScreenSaver {
public:
    StarfieldScreenSaver(BMessage* archive, image_id image);

    void StartConfig(BView* view) override;
    status_t SaveState(BMessage* into) const override;
    status_t StartSaver(BView* view, bool preview) override;
    void Draw(BView* view, int32 frame) override;
    int32 NumStars();
    int32 NumTeapots();
    float StarSpeed();
    float TeapotSpeed();
    void SetNumStars(int32 num);
    void SetNumTeapots(int32 num);
    void SetStarSpeed(float speed);
    void SetTeapotSpeed(float speed);

private:
    StarfieldGLView* fGLView;
    ConfigView* fConfigView;
    int32 fNumStars;
    int32 fNumTeapots;
    float fStarSpeed;
    float fTeapotSpeed;
    bool fIsPreview;
};

class ConfigView : public BView {
public:
    ConfigView(BRect frame, StarfieldScreenSaver* saver)
        : BView(frame, "SampleConfigView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
          fScreenSaver(saver) {
        SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

        fNameStringView = new BStringView("NameString", "A starry sky with orbiting teapots");
        fNameStringView->SetFont(be_bold_font);

        fStarsSlider = new BSlider("StarsSlider", "Number of Stars:", 
                                   new BMessage(kMsgSetStars), 50, MAX_STARS, B_HORIZONTAL);
        fStarsSlider->SetValue(saver->NumStars());
        fStarsSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
        fStarsSlider->SetHashMarkCount(10);
        fStarsSlider->SetLimitLabels("50", "1000");

        fTeapotsSlider = new BSlider("TeapotsSlider", "Number of Teapots:", 
                                     new BMessage(kMsgSetTeapots), 0, MAX_TEAPOTS, B_HORIZONTAL);
        fTeapotsSlider->SetValue(saver->NumTeapots());
        fTeapotsSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
        fTeapotsSlider->SetHashMarkCount(11);
        fTeapotsSlider->SetLimitLabels("0", "10");

        fStarSpeedSlider = new BSlider("StarSpeedSlider", "Star Speed:", 
                                       new BMessage(kMsgSetStarSpeed), 10, 250, B_HORIZONTAL);
        fStarSpeedSlider->SetValue(saver->StarSpeed() * 10000.0f);

        fTeapotSpeedSlider = new BSlider("TeapotSpeedSlider", "Teapot Speed:", 
                                         new BMessage(kMsgSetTeapotSpeed), 10, 250, B_HORIZONTAL);
        fTeapotSpeedSlider->SetValue(saver->TeapotSpeed() * 1000.0f);

        // Create BTextView for author information
        fInfoTextView = new BTextView("infoTextView", be_plain_font, NULL, B_WILL_DRAW);
        fInfoTextView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
        fInfoTextView->MakeEditable(false);
        fInfoTextView->SetStylable(true);
        fInfoTextView->SetText(
            "©2024 Claude 3.5 Sonnet by Anthropic\n\n"
            "This screen saver was designed and implemented by Claude, an AI assistant created by Anthropic, "
            "demonstrating the capabilities of artificial intelligence in software development.\n"
            "The code was generated based on the user's requirements and best practices for Haiku OS development.\n"
        );

        BScrollView* infoScrollView = new BScrollView("infoScrollView", fInfoTextView, B_WILL_DRAW | B_FRAME_EVENTS, false, true);

        BGroupLayout* layout = new BGroupLayout(B_VERTICAL);
        SetLayout(layout);

        layout->SetInsets(B_USE_DEFAULT_SPACING);           
        layout->AddView(fNameStringView);
        layout->AddView(fStarsSlider);
        layout->AddView(fStarSpeedSlider);
        layout->AddView(fTeapotsSlider);
		layout->AddView(fTeapotSpeedSlider);
        layout->AddView(infoScrollView);
    }

    void AttachedToWindow() override {
        fStarsSlider->SetTarget(this);
        fTeapotsSlider->SetTarget(this);
        fStarSpeedSlider->SetTarget(this);
        fTeapotSpeedSlider->SetTarget(this);
    }

    void MessageReceived(BMessage* message) override {
        switch (message->what) {
            case kMsgSetStars:
                fScreenSaver->SetNumStars(fStarsSlider->Value());
                break;
            case kMsgSetTeapots:
                fScreenSaver->SetNumTeapots(fTeapotsSlider->Value());
                break;
            case kMsgSetStarSpeed:
                fScreenSaver->SetStarSpeed((float)fStarSpeedSlider->Value() / 10000.0f);
                break;
            case kMsgSetTeapotSpeed:
                fScreenSaver->SetTeapotSpeed((float)fTeapotSpeedSlider->Value() / 1000.0f);
                break;
            default:
                BView::MessageReceived(message);
        }
    }

private:
    StarfieldScreenSaver* fScreenSaver;
    BStringView* fNameStringView;
    BSlider* fStarsSlider;
    BSlider* fTeapotsSlider;
    BSlider* fStarSpeedSlider;
    BSlider* fTeapotSpeedSlider;
    BTextView* fInfoTextView;

    static const uint32 kMsgSetStars = 'StSt';
    static const uint32 kMsgSetTeapots = 'StTp';
    static const uint32 kMsgSetStarSpeed = 'StSp';
    static const uint32 kMsgSetTeapotSpeed = 'TpSp';
};

StarfieldScreenSaver::StarfieldScreenSaver(BMessage* archive, image_id image)
        : BScreenSaver(archive, image), fGLView(nullptr), fConfigView(nullptr) {
    if (archive != nullptr) {
        if (archive->FindInt32("num_stars", &fNumStars) != B_OK)
            fNumStars = 300;
        if (archive->FindInt32("num_teapots", &fNumTeapots) != B_OK)
            fNumTeapots = 3;
        if (archive->FindFloat("star_speed", &fStarSpeed) != B_OK)
            fStarSpeed = 0.02f;
        if (archive->FindFloat("teapot_speed", &fTeapotSpeed) != B_OK)
            fTeapotSpeed = 0.05f;
    } else {
        fNumStars = 300;
        fNumTeapots = 3;
        fStarSpeed = 0.02f;
        fTeapotSpeed = 0.05f;
    }
}

void StarfieldScreenSaver::StartConfig(BView* view) {
    fConfigView = new ConfigView(view->Bounds(), this);
    view->AddChild(fConfigView);
}

status_t StarfieldScreenSaver::SaveState(BMessage* into) const {
    into->AddInt32("num_stars", fNumStars);
    into->AddInt32("num_teapots", fNumTeapots);
    into->AddFloat("star_speed", fStarSpeed);
    into->AddFloat("teapot_speed", fTeapotSpeed);
    return B_OK;
}

status_t StarfieldScreenSaver::StartSaver(BView* view, bool preview) {	
    if (!fGLView) {
        BRect bounds = view->Bounds();
        fGLView = new StarfieldGLView(bounds, fNumStars, fNumTeapots, fStarSpeed, fTeapotSpeed, preview);
        view->AddChild(fGLView);
    }
    view->Window()->SetPulseRate(50000);
    return B_OK;
}

void StarfieldScreenSaver::Draw(BView* view, int32 frame) {
    if (fGLView) {
        fGLView->Draw(view->Bounds());
    }
}

int32 StarfieldScreenSaver::NumStars() {
    return fNumStars;
}

int32 StarfieldScreenSaver::NumTeapots() {
    return fNumTeapots;
}

float StarfieldScreenSaver::StarSpeed() {
    return fStarSpeed;
}

float StarfieldScreenSaver::TeapotSpeed() {
    return fTeapotSpeed;
}

void StarfieldScreenSaver::SetNumStars(int32 num) {
    fNumStars = std::min<int32>(num, static_cast<int32>(MAX_STARS));
    if (fGLView) fGLView->SetParameters(fNumStars, fNumTeapots, fStarSpeed, fTeapotSpeed);
}

void StarfieldScreenSaver::SetNumTeapots(int32 num) {
    fNumTeapots = std::min<int32>(num, static_cast<int32>(MAX_TEAPOTS));
    if (fGLView) fGLView->SetParameters(fNumStars, fNumTeapots, fStarSpeed, fTeapotSpeed);
}

void StarfieldScreenSaver::SetStarSpeed(float speed) {
    fStarSpeed = speed;
    if (fGLView) fGLView->SetParameters(fNumStars, fNumTeapots, fStarSpeed, fTeapotSpeed);
}

void StarfieldScreenSaver::SetTeapotSpeed(float speed) {
    fTeapotSpeed = speed;
    if (fGLView) fGLView->SetParameters(fNumStars, fNumTeapots, fStarSpeed, fTeapotSpeed);
}

// Function to create an instance of the screensaver
extern "C" _EXPORT BScreenSaver* instantiate_screen_saver(BMessage* archive, image_id image) {
    return new StarfieldScreenSaver(archive, image);
}
