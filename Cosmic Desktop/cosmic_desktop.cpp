/*
 * Cosmic Desktop Screensaver for Haiku
 *
 * Watch your desktop take a thrilling ride through a starry cosmos! 
 *
 * Original Author: Claude 3.5 Sonnet (AI Assistant by Anthropic)
 * Modified and Enhanced by: Gemini 1.5 Pro (AI Assistant by Google)
 */

#include <ScreenSaver.h>
#include <LayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <StringView.h>
#include <TextView.h>
#include <ScrollView.h>
#include <StringView.h>
#include <GLView.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Slider.h>
#include <Bitmap.h>
#include <Screen.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <cmath>

class CosmicDesktopConfigView;
class CosmicDesktopGLView;

class CosmicDesktopSaver : public BScreenSaver {
public:
								CosmicDesktopSaver(BMessage* archive, image_id image);

	void						StartConfig(BView* view);
	status_t					StartSaver(BView* view, bool preview);
	status_t					SaveState(BMessage* into) const;
	void						RestoreState(BMessage* from);
	void						Draw(BView* view, int32 frame);

	void						SetRotationSpeed(float speed);
	float						GetRotationSpeed();
	void						SetWobbleAmplitude(float amplitude);
	float						GetWobbleAmplitude();

private:
	CosmicDesktopGLView*		fGLView;
	float						fRotationSpeed;
	float						fWobbleAmplitude;
};

// --- CosmicDesktopConfigView ---

class CosmicDesktopConfigView : public BView {
public:
								CosmicDesktopConfigView(BRect frame,
									CosmicDesktopSaver* saver);

	void						AttachedToWindow();
	void						MessageReceived(BMessage* message);

private:
	static const uint32			kSpeedChanged = 'SpCh';
	static const uint32			kAmplitudeChanged = 'AmCh';
	CosmicDesktopSaver*			fSaver;
	BStringView*				fNameStringView;
	BTextView*					fInfoTextView;
	BSlider*					fSpeedSlider;
	BSlider*					fAmplitudeSlider;
};

// --- CosmicDesktopGLView ---

class CosmicDesktopGLView : public BGLView {
public:
								CosmicDesktopGLView(BRect frame);

	void						AttachedToWindow();
	void						Draw(BRect updateRect);
	void						Advance(float delta);
	void						SetRotationSpeed(float speed);
	void						SetWobbleAmplitude(float amplitude);
	void						SetPreviewMode(bool previewMode);

private:
	void						DrawStars(bool previewMode);
	void						InitializeRotationVector();

	float						SmoothStep(float edge0, float edge1, float x) {
		x = std::max(0.0f, std::min(1.0f, (x - edge0) / (edge1 - edge0)));
		return x * x * (3 - 2 * x);
	}

	static constexpr float		MAX_DISTANCE = 1.5f;

	float						fWidth;
	float						fHeight;
	float						fAspectRatio;
	float						fRotationSpeed;
	float						fRotationAngle;
	float						fDistance;
	GLuint						fTextureId;
	bool						fPreviewMode;
	
	float						fRotationX;
	float						fRotationY;
	float						fRotationZ;
	float						fWobbleAngle;
	float						fWobbleSpeed;
	float						fWobbleAmplitude;
};

// --- CosmicDesktopSaver implementation ---

CosmicDesktopSaver::CosmicDesktopSaver(BMessage* archive, image_id image)
	:
	BScreenSaver(archive, image),
	fGLView(nullptr),
	fRotationSpeed(5.0f),
	fWobbleAmplitude(0.05f)
{
	RestoreState(archive);
}


void
CosmicDesktopSaver::StartConfig(BView* view)
{
	CosmicDesktopConfigView* configView = new CosmicDesktopConfigView(view->Bounds(), this);
	view->AddChild(configView);
}


status_t
CosmicDesktopSaver::StartSaver(BView* view, bool preview)
{
	if (!fGLView) {
		BRect bounds = view->Bounds();
		fGLView = new CosmicDesktopGLView(bounds);
		view->AddChild(fGLView);
		fGLView->SetPreviewMode(preview);
		fGLView->SetRotationSpeed(fRotationSpeed);
		fGLView->SetWobbleAmplitude(fWobbleAmplitude);
	}
	SetTickSize(25000);
	return B_OK;
}


status_t
CosmicDesktopSaver::SaveState(BMessage* into) const
{
	into->AddFloat("rotation_speed", fRotationSpeed);
	into->AddFloat("wobble_amplitude", fWobbleAmplitude);
	return B_OK;
}


void
CosmicDesktopSaver::RestoreState(BMessage* from)
{
	if (from) {
		if (from->FindFloat("rotation_speed", &fRotationSpeed) != B_OK) {
			fRotationSpeed = 5.0f;
		}
		if (from->FindFloat("wobble_amplitude", &fWobbleAmplitude) != B_OK) {
			fWobbleAmplitude = 0.05f;
		}
	} else {
		fRotationSpeed = 5.0f;
		fWobbleAmplitude = 0.05f;
	}

	if (fGLView) {
		fGLView->SetRotationSpeed(fRotationSpeed);
		fGLView->SetWobbleAmplitude(fWobbleAmplitude);
	}
}


void
CosmicDesktopSaver::Draw(BView* view, int32 frame)
{
	if (fGLView) {
		fGLView->Advance(0.01f);
		fGLView->Draw(view->Bounds());
	}
}


void
CosmicDesktopSaver::SetRotationSpeed(float speed)
{
	fRotationSpeed = speed;
	if (fGLView) {
		fGLView->SetRotationSpeed(speed);
	}
}


float
CosmicDesktopSaver::GetRotationSpeed()
{
	return fRotationSpeed;
}


void
CosmicDesktopSaver::SetWobbleAmplitude(float amplitude)
{
	fWobbleAmplitude = amplitude;
	if (fGLView) {
		fGLView->SetWobbleAmplitude(amplitude);
	}
}


float
CosmicDesktopSaver::GetWobbleAmplitude()
{
	return fWobbleAmplitude;
}

// --- CosmicDesktopConfigView implementation ---

CosmicDesktopConfigView::CosmicDesktopConfigView(BRect frame, CosmicDesktopSaver* saver)
	:
	BView(frame, "CosmicDesktopConfigView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	fSaver(saver)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	fNameStringView = new BStringView("nameString", "Cosmic Desktop");
	fNameStringView->SetFont(be_bold_font);

	// Create the info text view
	BRect textRect = Bounds();
	textRect.InsetBy(5, 5);
	fInfoTextView = new BTextView(textRect, "infoTextView", textRect, B_FOLLOW_ALL_SIDES);
	fInfoTextView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fInfoTextView->MakeEditable(false);
	fInfoTextView->SetStylable(true);

	// Add text to the info text view
	fInfoTextView->Insert("©2024 Claude 3.5 Sonnet by Anthropic\n");
	fInfoTextView->Insert("©2024 Gemini 1.5 Pro by Google\n\n"); 
	fInfoTextView->Insert("Fasten your seatbelts! This screensaver takes your desktop on a dizzying spin through a colorful galaxy.\n");
	fInfoTextView->Insert("Use the sliders below to adjust the speed of your cosmic journey and the amplitude of the wobble effect.\n");
	fInfoTextView->Insert("Initially dreamt up by Claude, an AI assistant created by Anthropic,\n");
	fInfoTextView->Insert("and polished to a shine by Gemini, an AI assistant by Google, this screensaver is your portal to the stars! \n"); 

	// Create the scroll view for the info text view
	BScrollView* infoScrollView = new BScrollView("infoScrollView", fInfoTextView,
		B_WILL_DRAW | B_FRAME_EVENTS, false, true);

	// Create the rotation speed slider
	fSpeedSlider = new BSlider("speedSlider", "Rotation Speed", new BMessage(kSpeedChanged),
		0, 1000, B_HORIZONTAL);
	fSpeedSlider->SetValue(fSaver->GetRotationSpeed() * 100.0f);

	// Create the wobble amplitude slider
	fAmplitudeSlider = new BSlider("amplitudeSlider", "Wobble Amplitude", new BMessage(kAmplitudeChanged),
		0, 1000, B_HORIZONTAL);
	fAmplitudeSlider->SetValue(fSaver->GetWobbleAmplitude() * 10000.0f);

	// Use BGroupLayout for layout
	BGroupLayout* layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);
	layout->SetInsets(B_USE_DEFAULT_SPACING);

	// Add controls to layout
	layout->AddView(fNameStringView);
	layout->AddView(fSpeedSlider);
	layout->AddView(fAmplitudeSlider);
	layout->AddView(infoScrollView);
}


void
CosmicDesktopConfigView::AttachedToWindow()
{
	fSpeedSlider->SetTarget(this);
	fAmplitudeSlider->SetTarget(this);
}


void
CosmicDesktopConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kSpeedChanged:
			fSaver->SetRotationSpeed(fSpeedSlider->Value() / 100.0f);
			break;
		case kAmplitudeChanged:
			fSaver->SetWobbleAmplitude(fAmplitudeSlider->Value() / 10000.0f);
			break;
		default:
			BView::MessageReceived(message);
	}
}

// --- CosmicDesktopGLView implementation ---

CosmicDesktopGLView::CosmicDesktopGLView(BRect frame)
	:
	BGLView(frame, "CosmicDesktopGLView", B_FOLLOW_ALL, B_WILL_DRAW, BGL_RGB | BGL_DOUBLE | BGL_DEPTH),
	fWidth(frame.Width()),
	fHeight(frame.Height()),
	fRotationSpeed(5.0f),
	fRotationAngle(0.0f),
	fDistance(0.0f),
	fPreviewMode(false),
	fWobbleAngle(0.0f),
	fWobbleSpeed(2.0f),
	fWobbleAmplitude(0.05f)
{
	fAspectRatio = fWidth / fHeight;
	srand(static_cast<unsigned int>(time(nullptr)));
	InitializeRotationVector();
}


void
CosmicDesktopGLView::InitializeRotationVector()
{
	// Random selection of rotation vector
	fRotationX = (rand() % 201 - 100) / 100.0f;  // from -1.0 to 1.0
	fRotationY = (rand() % 201 - 100) / 100.0f;  // from -1.0 to 1.0
	fRotationZ = (rand() % 201 - 100) / 100.0f;  // from -1.0 to 1.0
	
	// Normalize the vector
	float length = sqrt(fRotationX * fRotationX + fRotationY * fRotationY + fRotationZ * fRotationZ);
	fRotationX /= length;
	fRotationY /= length;
	fRotationZ /= length;
}


void
CosmicDesktopGLView::AttachedToWindow()
{
	BGLView::AttachedToWindow();
	LockGL();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_POINT_SMOOTH);

	// Capture screenshot and create texture
	BScreen screen;
	BBitmap* screenshot = new BBitmap(screen.Frame(), B_RGB32);
	screen.ReadBitmap(screenshot, false, NULL);

	glGenTextures(1, &fTextureId);
	glBindTexture(GL_TEXTURE_2D, fTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenshot->Bounds().IntegerWidth() + 1,
		screenshot->Bounds().IntegerHeight() + 1, 0, GL_BGRA, GL_UNSIGNED_BYTE,
		screenshot->Bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete screenshot;

	UnlockGL();
}


void
CosmicDesktopGLView::Draw(BRect updateRect)
{
	LockGL();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, fAspectRatio, 0.1, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 2.11, 0, 0, 0, 0, 1, 0);

	// Draw stars
	DrawStars(fPreviewMode);

	// Enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Light position
	GLfloat lightPos[] = { 1.0f, 1.0f, 2.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	// Increase ambient light for better visibility
	GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

	glTranslatef(0, 0, -sqrt(fDistance));
	
	// Apply main rotation
	glRotatef(fRotationAngle, fRotationX, fRotationY, fRotationZ);
	
	// Add wobble effect depending on distance
	if (fDistance > MAX_DISTANCE / 2) {
		float distanceFactor = SmoothStep(MAX_DISTANCE / 2, MAX_DISTANCE, fDistance);
		float wobbleAmount = sin(fWobbleAngle) * fWobbleAmplitude * distanceFactor * (fWobbleSpeed / 2.0f);
		glRotatef(wobbleAmount * 360.0f, fRotationY, fRotationZ, fRotationX);
	}

	glBindTexture(GL_TEXTURE_2D, fTextureId);

	// Enable blending for transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw parallelepiped
	float halfWidth = 0.9f;
	float halfHeight = halfWidth / fAspectRatio;
	float depth = halfWidth * 2;

	glBegin(GL_QUADS);

	// Front face (no transparency)
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0, 1); glVertex3f(-halfWidth, -halfHeight, halfWidth);
	glTexCoord2f(1, 1); glVertex3f(halfWidth, -halfHeight, halfWidth);
	glTexCoord2f(1, 0); glVertex3f(halfWidth, halfHeight, halfWidth);
	glTexCoord2f(0, 0); glVertex3f(-halfWidth, halfHeight, halfWidth);

	// Back face (semi-transparent)
	glNormal3f(0.0f, 0.0f, -1.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 0.8f); // 80% opacity
	glTexCoord2f(1, 1); glVertex3f(-halfWidth, -halfHeight, -halfWidth);
	glTexCoord2f(1, 0); glVertex3f(-halfWidth, halfHeight, -halfWidth);
	glTexCoord2f(0, 0); glVertex3f(halfWidth, halfHeight, -halfWidth);
	glTexCoord2f(0, 1); glVertex3f(halfWidth, -halfHeight, -halfWidth);

	// Right face (semi-transparent)
	glNormal3f(1.0f, 0.0f, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 0.8f); // 80% opacity
	glTexCoord2f(1, 1); glVertex3f(halfWidth, -halfHeight, -halfWidth);
	glTexCoord2f(1, 0); glVertex3f(halfWidth, halfHeight, -halfWidth);
	glTexCoord2f(0, 0); glVertex3f(halfWidth, halfHeight, halfWidth);
	glTexCoord2f(0, 1); glVertex3f(halfWidth, -halfHeight, halfWidth);

	// Left face (semi-transparent)
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 0.8f); // 80% opacity
	glTexCoord2f(0, 1); glVertex3f(-halfWidth, -halfHeight, -halfWidth);
	glTexCoord2f(1, 1); glVertex3f(-halfWidth, -halfHeight, halfWidth);
	glTexCoord2f(1, 0); glVertex3f(-halfWidth, halfHeight, halfWidth);
	glTexCoord2f(0, 0); glVertex3f(-halfWidth, halfHeight, -halfWidth);

	glEnd();

	// Disable blending and lighting
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);

	SwapBuffers();
	UnlockGL();
}


void
CosmicDesktopGLView::DrawStars(bool previewMode)
{
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Determine number of stars based on mode
	int numStars = previewMode ? 64 : 1024;

	// Generate random stars once on startup
	static std::vector<GLfloat> starPositions;
	static std::vector<GLfloat> starColors;
	if (starPositions.empty()) {
		// Calculate visible area boundaries
		float aspectRatio = fWidth / fHeight;
		float fovY = 45.0f; // vertical field of view in degrees
		float fovX = fovY * aspectRatio;
		float nearZ = 0.1f;
		float farZ = 1500.0f;

		float tanFovY = tan(fovY * M_PI / 180.0f / 2.0f);
		float tanFovX = tan(fovX * M_PI / 180.0f / 2.0f);

		for (int i = 0; i < numStars; ++i) {
			// Generate stars within the visible pyramid
			float z = -(rand() / (float)RAND_MAX * (farZ - nearZ) + nearZ);
			float x = (rand() / (float)RAND_MAX * 2 - 1) * -z * tanFovX;
			float y = (rand() / (float)RAND_MAX * 2 - 1) * -z * tanFovY;

			starPositions.push_back(x);
			starPositions.push_back(y);
			starPositions.push_back(z);

			float brightness = rand() / (float)RAND_MAX;
			float colorVariation = 0.1f; // Maximum color deviation from white

			// Create slight color variations
			float r = brightness + (rand() % 201 - 100) / 1000.0f * colorVariation;
			float g = brightness + (rand() % 201 - 100) / 1000.0f * colorVariation;
			float b = brightness + (rand() % 201 - 100) / 1000.0f * colorVariation;

			starColors.push_back(r);
			starColors.push_back(g);
			starColors.push_back(b);
			starColors.push_back(brightness); // A
		}
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, &starPositions[0]);
	glColorPointer(4, GL_FLOAT, 0, &starColors[0]);

	glPointSize(previewMode ? 0.5f : (1.0f + 2.0f * ((rand() / (float)RAND_MAX) * 0.5f + 0.5f)));
	glDrawArrays(GL_POINTS, 0, starPositions.size() / 3);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
}


void
CosmicDesktopGLView::Advance(float delta)
{
	fRotationAngle += fRotationSpeed * delta * 1.2f;
	if (fRotationAngle > 360.0f) {
		fRotationAngle -= 360.0f;
	}
	
	fDistance += delta;
	if (fDistance > MAX_DISTANCE) {
		fDistance = MAX_DISTANCE;
	}
	
	// Update wobble angle if we've passed half the distance
	if (fDistance > MAX_DISTANCE / 2) {
		float distanceFactor = (fDistance - MAX_DISTANCE / 2) / (MAX_DISTANCE / 2);
		fWobbleAngle += fWobbleSpeed * delta * distanceFactor;
		if (fWobbleAngle > 2 * M_PI) {
			fWobbleAngle -= 2 * M_PI;
		}
	}
}


void
CosmicDesktopGLView::SetRotationSpeed(float speed)
{
	fRotationSpeed = speed * 5.0f;
}


void
CosmicDesktopGLView::SetWobbleAmplitude(float amplitude)
{
	fWobbleAmplitude = amplitude;
}


void
CosmicDesktopGLView::SetPreviewMode(bool previewMode)
{
	fPreviewMode = previewMode;
}

// --- Entry point ---

extern "C" _EXPORT BScreenSaver*
instantiate_screen_saver(BMessage* archive, image_id image)
{
	return new CosmicDesktopSaver(archive, image);
}