/*
 * Copyright 2024 Claude 3.5 Sonnet by Anthropic
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Claude (AI assistant)
 *
 * This screen saver displays three rotating 3D gears with random number of teeth,
 * colors, and positions. The entire scene rotates smoothly in random directions.
 *
 * This screen saver was designed and implemented by Claude, an AI assistant
 * created by Anthropic, demonstrating the capabilities of artificial intelligence
 * in software development. The code was generated based on user requirements
 * and best practices for Haiku development.
 */

#include <ScreenSaver.h>
#include <LayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <StringView.h>
#include <TextView.h>
#include <ScrollView.h>
#include <GLView.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

class GearScreenSaver;

class GearConfigView : public BView {
public:
								GearConfigView(BRect frame, GearScreenSaver* saver);
	virtual	void				AttachedToWindow();
	virtual	void				MessageReceived(BMessage* message);

private:
			GearScreenSaver*	fSaver;
			BStringView*		fNameStringView;
			BTextView*			fInfoTextView;
};

class Gear {
public:
								Gear(int teeth, float moduleSize, float x, float y,
									float z, float r, float g, float b,
									float rotationSpeed);
			void				Draw();
			void				Rotate();
			float				Radius() const { return fRadius; }
			float				ToothHeight() const { return fModuleSize; }
			int					TeethCount() const { return fTeeth; }
			float				Rotation() const { return fRotation; }

			float				x;
			float				y;
			float				z;

			void				DrawCircleWithHole(float outerRadius, float z,
									float innerRadius);

			int					fTeeth;
			float				fModuleSize;
			float				fRadius;
			float				fR;
			float				fG;
			float				fB;
			float				fRotation;
			float				fRotationSpeed;
};

class GearGLView : public BGLView {
public:
								GearGLView(BRect frame);
								~GearGLView();

	virtual	void				AttachedToWindow();
	virtual	void				Draw();

private:
			void				_SynchronizeGears();

			float				fWidth;
			float				fHeight;
			Gear*				fGears[3];
			float				fSceneRotationX;
			float				fSceneRotationY;
			float				fSceneRotationZ;
};

class GearScreenSaver : public BScreenSaver {
public:
								GearScreenSaver(BMessage* archive, image_id image);

	virtual	void				StartConfig(BView* view);
	virtual	status_t			StartSaver(BView* view, bool preview);
	virtual	void				Draw(BView* view, int32 frame);

private:
			GearGLView*			fGLView;
};

// Implementation of GearConfigView

GearConfigView::GearConfigView(BRect frame, GearScreenSaver* saver)
	:
	BView(frame, "GearConfigView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	fSaver(saver)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	fNameStringView = new BStringView("nameString", "Rotating 3D gears");
	fNameStringView->SetFont(be_bold_font);
	
	BRect textRect = Bounds();
	textRect.InsetBy(5, 5);
	fInfoTextView = new BTextView(textRect, "infoTextView", textRect,
		B_FOLLOW_ALL_SIDES);
	fInfoTextView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fInfoTextView->MakeEditable(false);
	fInfoTextView->SetStylable(true);

	fInfoTextView->Insert("©2024 Claude 3.5 Sonnet by Anthropic\n\n");
	fInfoTextView->Insert("This screen saver displays three rotating 3D gears.\n");
	fInfoTextView->Insert("The gears have random number of teeth, colors, and positions.\n");
	fInfoTextView->Insert("The entire scene rotates smoothly in random directions.\n\n");
	fInfoTextView->Insert("This screen saver was designed and implemented by Claude, "
		"an AI assistant created by Anthropic, ");
	fInfoTextView->Insert("demonstrating the capabilities of artificial intelligence "
		"in software development.\n");
	fInfoTextView->Insert("The code was generated based on the user's requirements "
		"and best practices for Haiku development.\n");

	BScrollView* infoScrollView = new BScrollView("infoScrollView", fInfoTextView,
		B_WILL_DRAW | B_FRAME_EVENTS, false, true);

	BGroupLayout* layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);

	layout->SetInsets(B_USE_DEFAULT_SPACING);

	layout->AddView(fNameStringView);    
	layout->AddView(infoScrollView);
}

void
GearConfigView::AttachedToWindow()
{
}

void
GearConfigView::MessageReceived(BMessage* message)
{
}

// Implementation of Gear

Gear::Gear(int teeth, float moduleSize, float x, float y, float z,
	float r, float g, float b, float rotationSpeed)
	:
	fTeeth(teeth),
	fModuleSize(moduleSize),
	fRadius(teeth * moduleSize / 2),
	x(x),
	y(y),
	z(z),
	fR(r),
	fG(g),
	fB(b),
	fRotation(0),
	fRotationSpeed(rotationSpeed)
{
}

void
Gear::Draw()
{
	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(fRotation, 0, 0, 1);

	float toothHeight = fModuleSize;
	float outerRadius = fRadius + toothHeight;
	float innerRadius = fRadius;
	float thickness = 0.3f * fRadius;
	float toothAngle = 2 * M_PI / fTeeth;
	float toothWidthInner = fRadius * sinf(toothAngle / 2);
	float toothWidthOuter = toothWidthInner * 1.6f;

	// Define the bevel thickness for the outer edges of the teeth
	float bevelThickness = 0.1f * thickness;

	// Draw teeth
	glBegin(GL_QUADS);
	for (int i = 0; i < fTeeth; i++) {
		float angle = i * toothAngle;
		
		// Outer face of the tooth
		glNormal3f(cosf(angle + toothAngle / 2), sinf(angle + toothAngle / 2), 0);
		glVertex3f(outerRadius * cosf(angle + toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothWidthOuter / (2 * outerRadius)),
			thickness / 2 - bevelThickness);
		glVertex3f(outerRadius * cosf(angle + toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothWidthOuter / (2 * outerRadius)),
			-thickness / 2 + bevelThickness);
		glVertex3f(outerRadius * cosf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			-thickness / 2 + bevelThickness);
		glVertex3f(outerRadius * cosf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			thickness / 2 - bevelThickness);

		// Top face of the tooth
		glNormal3f(0, 0, 1);
		glVertex3f(outerRadius * cosf(angle + toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothWidthOuter / (2 * outerRadius)),
			thickness / 2 - bevelThickness);
		glVertex3f(outerRadius * cosf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			thickness / 2 - bevelThickness);
		glVertex3f(fRadius * cosf(angle + toothAngle - toothWidthInner / (2 * fRadius)),
			fRadius * sinf(angle + toothAngle - toothWidthInner / (2 * fRadius)),
			thickness / 2);
		glVertex3f(fRadius * cosf(angle + toothWidthInner / (2 * fRadius)),
			fRadius * sinf(angle + toothWidthInner / (2 * fRadius)),
			thickness / 2);
		
		// Bottom face of the tooth
		glNormal3f(0, 0, -1);
		glVertex3f(outerRadius * cosf(angle + toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothWidthOuter / (2 * outerRadius)),
			-thickness / 2 + bevelThickness);
		glVertex3f(fRadius * cosf(angle + toothWidthInner / (2 * fRadius)),
			fRadius * sinf(angle + toothWidthInner / (2 * fRadius)),
			-thickness / 2);
		glVertex3f(fRadius * cosf(angle + toothAngle - toothWidthInner / (2 * fRadius)),
			fRadius * sinf(angle + toothAngle - toothWidthInner / (2 * fRadius)),
			-thickness / 2);
		glVertex3f(outerRadius * cosf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			-thickness / 2 + bevelThickness);

		// Side faces of the tooth with bevel
		glNormal3f(-sinf(angle + toothWidthInner / (2 * fRadius)),
			cosf(angle + toothWidthInner / (2 * fRadius)), 0);
		glVertex3f(outerRadius * cosf(angle + toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothWidthOuter / (2 * outerRadius)),
			thickness / 2 - bevelThickness);
		glVertex3f(fRadius * cosf(angle + toothWidthInner / (2 * fRadius)),
			fRadius * sinf(angle + toothWidthInner / (2 * fRadius)),
			thickness / 2);
		glVertex3f(fRadius * cosf(angle + toothWidthInner / (2 * fRadius)),
			fRadius * sinf(angle + toothWidthInner / (2 * fRadius)),
			-thickness / 2);
		glVertex3f(outerRadius * cosf(angle + toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothWidthOuter / (2 * outerRadius)),
			-thickness / 2 + bevelThickness);

		glNormal3f(sinf(angle + toothAngle - toothWidthInner / (2 * fRadius)),
			-cosf(angle + toothAngle - toothWidthInner / (2 * fRadius)), 0);
		glVertex3f(outerRadius * cosf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			thickness / 2 - bevelThickness);
		glVertex3f(outerRadius * cosf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			outerRadius * sinf(angle + toothAngle - toothWidthOuter / (2 * outerRadius)),
			-thickness / 2 + bevelThickness);
		glVertex3f(fRadius * cosf(angle + toothAngle - toothWidthInner / (2 * fRadius)),
			fRadius * sinf(angle + toothAngle - toothWidthInner / (2 * fRadius)),
			-thickness / 2);
		glVertex3f(fRadius * cosf(angle + toothAngle - toothWidthInner / (2 * fRadius)),
			fRadius * sinf(angle + toothAngle - toothWidthInner / (2 * fRadius)),
			thickness / 2);
	}
	glEnd();

	// Draw gear body (cylinder)
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= fTeeth * 16; i++) {
		float angle = i * (toothAngle / 16);
		glNormal3f(-cosf(angle), -sinf(angle), 0);
		glVertex3f(innerRadius * cosf(angle), innerRadius * sinf(angle), thickness / 2);
		glVertex3f(innerRadius * cosf(angle), innerRadius * sinf(angle), -thickness / 2);
	}
	glEnd();

	// Draw front and back surfaces with a hole
	DrawCircleWithHole(innerRadius, thickness / 2, 0.2f * innerRadius);
	DrawCircleWithHole(innerRadius, -thickness / 2, 0.2f * innerRadius);
	
	// Draw cylinder connecting the holes
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= 360; i += 10) {
		float angle = i * M_PI / 180.0f;
		float x = 0.2f * innerRadius * cosf(angle);
		float y = 0.2f * innerRadius * sinf(angle);
		glNormal3f(cosf(angle), sinf(angle), 0);
		glVertex3f(x, y, thickness / 2);
		glVertex3f(x, y, -thickness / 2);
	}
	glEnd();

	glPopMatrix();
}

void
Gear::Rotate()
{
	fRotation += fRotationSpeed;
	if (fRotation >= 360.0f)
		fRotation -= 360.0f;
}

void
Gear::DrawCircleWithHole(float outerRadius, float z, float innerRadius)
{
	glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(0, 0, z > 0 ? 1 : -1);
	for (int i = 0; i <= 360; i += 10) {
		float angle = i * M_PI / 180.0f;
		glVertex3f(innerRadius * cosf(angle), innerRadius * sinf(angle), z);
		glVertex3f(outerRadius * cosf(angle), outerRadius * sinf(angle), z);
	}
	glEnd();
}

// Implementation of GearGLView

GearGLView::GearGLView(BRect frame)
	:
	BGLView(frame, "GearGLView", B_FOLLOW_ALL, B_WILL_DRAW, BGL_RGB | BGL_DOUBLE | BGL_DEPTH),
	fWidth(frame.Width()),
	fHeight(frame.Height()),
	fSceneRotationX(0),
	fSceneRotationY(0),
	fSceneRotationZ(0)
{
	srand(time(NULL));

	float moduleSize = 0.075f;
	float baseSpeed = (rand() % 100 + 50) / 100.0f;

	for (int i = 0; i < 3; i++) {
		int teeth = (rand() % 10) * 2 + 7;
		float r = 0.15f + (rand() % 75) / 75.0f;
		float g = 0.15f + (rand() % 75) / 75.0f;
		float b = 0.15f + (rand() % 75) / 75.0f;
		float speed = baseSpeed * (i % 2 == 0 ? 1 : -1) * 20.0f / teeth;
		fGears[i] = new Gear(teeth, moduleSize, 0, 0, 0, r, g, b, speed);
	}

	float gearSpacing = moduleSize;
	fGears[0]->x = -(fGears[0]->Radius() + fGears[1]->Radius() + gearSpacing);
	fGears[1]->x = 0;
	fGears[2]->x = fGears[1]->Radius() + fGears[2]->Radius() + gearSpacing;

	// Synchronize gear rotations
	_SynchronizeGears();
}

GearGLView::~GearGLView()
{
	for (int i = 0; i < 3; i++) {
		delete fGears[i];
	}
}

void
GearGLView::AttachedToWindow()
{
	BGLView::AttachedToWindow();
	LockGL();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };
	GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, fWidth / fHeight, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	UnlockGL();
}

void
GearGLView::Draw()
{
	LockGL();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -5.0f);

	glRotatef(fSceneRotationX, 1.0f, 0.0f, 0.0f);
	glRotatef(fSceneRotationY, 0.0f, 1.0f, 0.0f);
	glRotatef(fSceneRotationZ, 0.0f, 0.0f, 1.0f);

	for (int i = 0; i < 3; i++) {
		glPushMatrix();
		glColor3f(fGears[i]->fR, fGears[i]->fG, fGears[i]->fB);
		fGears[i]->Draw();
		fGears[i]->Rotate();
		glPopMatrix();
	}

	fSceneRotationX += 0.2f;
	fSceneRotationY += 0.25f;
	fSceneRotationZ += 0.1f;

	SwapBuffers();
	UnlockGL();
}

void
GearGLView::_SynchronizeGears()
{
	// Angles per tooth for each gear
	float anglePerTooth1 = 360.0f / fGears[0]->TeethCount();
	float anglePerTooth2 = 360.0f / fGears[1]->TeethCount();
	float anglePerTooth3 = 360.0f / fGears[2]->TeethCount();

	// Synchronize first and second gears
	float rotationRatio12 = (float)fGears[0]->TeethCount() / (float)fGears[1]->TeethCount();
	fGears[1]->fRotation = fGears[0]->Rotation() * rotationRatio12;
	
	// Synchronize second and third gears
	float rotationRatio23 = (float)fGears[1]->TeethCount() / (float)fGears[2]->TeethCount();
	fGears[2]->fRotation = fGears[1]->Rotation() * rotationRatio23;

	// Ensure gears rotate in opposite directions
	fGears[1]->fRotationSpeed = -fGears[0]->fRotationSpeed * rotationRatio12;
	fGears[2]->fRotationSpeed = -fGears[1]->fRotationSpeed * rotationRatio23;
}

// Implementation of GearScreenSaver

GearScreenSaver::GearScreenSaver(BMessage* archive, image_id image)
	:
	BScreenSaver(archive, image),
	fGLView(NULL)
{
}

void
GearScreenSaver::StartConfig(BView* view)
{
	GearConfigView* configView = new GearConfigView(view->Bounds(), this);
	view->AddChild(configView);
}

status_t
GearScreenSaver::StartSaver(BView* view, bool preview)
{
	if (fGLView == NULL) {
		BRect bounds = view->Bounds();
		fGLView = new GearGLView(bounds);
		view->AddChild(fGLView);
	}

	SetTickSize(20000);

	return B_OK;
}

void
GearScreenSaver::Draw(BView* view, int32 frame)
{
	if (fGLView != NULL) {
		fGLView->Draw();
	}
}

// Screensaver hook
extern "C" _EXPORT BScreenSaver*
instantiate_screen_saver(BMessage* message, image_id image)
{
	return new GearScreenSaver(message, image);
}
