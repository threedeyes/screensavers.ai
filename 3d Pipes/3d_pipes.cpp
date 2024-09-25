/*
 * 3D-Pipes Screen Saver
 *
 * This program is a Haiku OS screensaver that generates colorful 3D pipe structures.
 * It utilizes OpenGL for rendering and allows configuration of the number of pipes and their radii.
 * The screensaver dynamically updates and animates multiple pipes within a 3D grid.
 * 
 * © Claude (version 3.5 Sonnet) by Anthropic
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
#include <Slider.h>
#include <GLView.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

const int GRID_SIZE = 20;

struct Point3D {
    float x, y, z;
};

struct Pipe {
    std::vector<Point3D> segments;
    float r, g, b;
    int direction;
};

class PipesScreenSaver;

class PipesConfigView : public BView {
public:
    PipesConfigView(BRect frame, PipesScreenSaver* saver);
    void AttachedToWindow() override;
    void MessageReceived(BMessage* message) override;

private:
    PipesScreenSaver* fSaver;
    BStringView* fNameStringView;
    BSlider* fPipeCountSlider;
    BSlider* fPipeRadiusSlider;

    enum {
        kMsgPipeCountChanged = 'pccg',
        kMsgPipeRadiusChanged = 'prcg'
    };
};

class PipesGLView : public BGLView {
public:
    PipesGLView(BRect frame);
    void AttachedToWindow() override;
    void Draw(BRect updateRect);
    void SetParameters(int32 pipeCount, float segmentLength, float pipeRadius);

private:
    float fWidth, fHeight;
    std::vector<Pipe> pipes;
    bool grid[GRID_SIZE][GRID_SIZE][GRID_SIZE];
    int32 fPipeCount;
    float fSegmentLength;
    float fPipeRadius;

    void InitPipes();
    void AddNewPipe();
    void UpdatePipes();
    void DrawPipes();
    void DrawPipeSegment(const Point3D& start, const Point3D& end);
    void DrawJoint(const Point3D& prev, const Point3D& current, const Point3D& next);
};

class PipesScreenSaver : public BScreenSaver {
public:
    PipesScreenSaver(BMessage* archive, image_id image);
    void StartConfig(BView* view) override;
    void StopSaver() override { };
    status_t SaveState(BMessage* into) const override;
    void RestoreState(BMessage* from);
    status_t StartSaver(BView* view, bool preview) override;
    void Draw(BView* view, int32 frame) override;

    int32 GetPipeCount() { return fPipeCount; }
    float GetPipeRadius() { return fPipeRadius; }

    void SetPipeCount(int32 count);
    void SetPipeRadius(float radius);

private:
    PipesGLView* fGLView;
    int32 fPipeCount;
    float fSegmentLength;
    float fPipeRadius;
};

// Implementation of PipesConfigView

PipesConfigView::PipesConfigView(BRect frame, PipesScreenSaver* saver)
    : BView(frame, "PipesConfigView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
      fSaver(saver) {
    SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

    BGroupLayout* layout = new BGroupLayout(B_VERTICAL);
    SetLayout(layout);
    layout->SetInsets(B_USE_DEFAULT_SPACING);

    fNameStringView = new BStringView("nameString", "Generates colorful 3D pipe structures");
    fNameStringView->SetFont(be_bold_font);
    layout->AddView(fNameStringView);

    fPipeCountSlider = new BSlider("pipeCount", "Number of Pipes:", 
        new BMessage(kMsgPipeCountChanged), 1, 20, B_HORIZONTAL);
    fPipeCountSlider->SetValue(fSaver->GetPipeCount());
    fPipeCountSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
    fPipeCountSlider->SetHashMarkCount(20);
    fPipeCountSlider->SetLimitLabels("1", "20");
    layout->AddView(fPipeCountSlider);

    fPipeRadiusSlider = new BSlider("pipeRadius", "Pipe Radius:", 
        new BMessage(kMsgPipeRadiusChanged), 1, 20, B_HORIZONTAL);
    fPipeRadiusSlider->SetValue(fSaver->GetPipeRadius() * 100.0f);
    fPipeRadiusSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
    fPipeRadiusSlider->SetHashMarkCount(20);
    fPipeRadiusSlider->SetLimitLabels("Min", "Max");
    layout->AddView(fPipeRadiusSlider);

    BTextView* infoTextView = new BTextView(frame, "infoTextView", frame, B_FOLLOW_ALL_SIDES);
    infoTextView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
    infoTextView->MakeEditable(false);
    infoTextView->SetStylable(true);

    infoTextView->Insert("©2024 Claude 3.5 Sonnet by Anthropic\n\n");
    infoTextView->Insert("This screen saver was designed and implemented by Claude, an AI assistant created by Anthropic, ");
    infoTextView->Insert("demonstrating the capabilities of artificial intelligence in software development.\n");
    infoTextView->Insert("The code was generated based on the user's requirements and best practices for Haiku development.\n");

    BScrollView* infoScrollView = new BScrollView("infoScrollView", infoTextView, B_WILL_DRAW | B_FRAME_EVENTS, false, true);
    layout->AddView(infoScrollView);
}

void PipesConfigView::AttachedToWindow() {
    fPipeCountSlider->SetTarget(this);
    fPipeRadiusSlider->SetTarget(this);
}

void PipesConfigView::MessageReceived(BMessage* message) {
    switch (message->what) {
        case kMsgPipeCountChanged:
            fSaver->SetPipeCount(fPipeCountSlider->Value());
            break;
        case kMsgPipeRadiusChanged:
            fSaver->SetPipeRadius(fPipeRadiusSlider->Value() / 100.0f);
            break;
        default:
            BView::MessageReceived(message);
    }
}

// Implementation of PipesGLView

PipesGLView::PipesGLView(BRect frame)
    : BGLView(frame, "PipesGLView", B_FOLLOW_ALL, B_WILL_DRAW, BGL_RGB | BGL_DOUBLE | BGL_DEPTH),
      fWidth(frame.Width()), fHeight(frame.Height()),
      fPipeCount(10), fSegmentLength(1.0f), fPipeRadius(0.1f) {
    srand(time(NULL));
    InitPipes();
}

void PipesGLView::AttachedToWindow() {
    BGLView::AttachedToWindow();
    LockGL();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    GLfloat light_position[] = { 5.0f, 5.0f, 5.0f, 0.0f };
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

void PipesGLView::Draw(BRect updateRect) {
    LockGL();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    UpdatePipes();
    DrawPipes();

    SwapBuffers();
    UnlockGL();
}

void PipesGLView::SetParameters(int32 pipeCount, float segmentLength, float pipeRadius) {
    fPipeCount = pipeCount;
    fSegmentLength = segmentLength;
    fPipeRadius = pipeRadius;
    InitPipes();
}

void PipesGLView::InitPipes() {
    memset(grid, 0, sizeof(grid));
    pipes.clear();
    for (int i = 0; i < fPipeCount; ++i) {
        AddNewPipe();
    }
}

void PipesGLView::AddNewPipe() {
    Pipe pipe;
    pipe.r = (float)rand() / RAND_MAX;
    pipe.g = (float)rand() / RAND_MAX;
    pipe.b = (float)rand() / RAND_MAX;
    pipe.direction = rand() % 6;

    Point3D start;
    do {
        start.x = rand() % GRID_SIZE - GRID_SIZE / 2;
        start.y = rand() % GRID_SIZE - GRID_SIZE / 2;
        start.z = rand() % GRID_SIZE - GRID_SIZE / 2;
    } while (grid[(int)start.x + GRID_SIZE/2][(int)start.y + GRID_SIZE/2][(int)start.z + GRID_SIZE/2]);

    pipe.segments.push_back(start);
    grid[(int)start.x + GRID_SIZE/2][(int)start.y + GRID_SIZE/2][(int)start.z + GRID_SIZE/2] = true;
    pipes.push_back(pipe);
}

void PipesGLView::UpdatePipes() {
    for (auto& pipe : pipes) {
        Point3D& last = pipe.segments.back();
        Point3D nextPoint = last;  // Initialize next point as the current segment position
        bool moved = false;
        
        // Set chance for random direction change (e.g., 1 in 10 steps)
        bool randomTurn = rand() % 10 == 0;

        // Try to move in the current direction if possible
        if (!randomTurn) {
            // Calculate the next position in the current direction
            switch (pipe.direction) {
                case 0: nextPoint.x += fSegmentLength; break;  // X+
                case 1: nextPoint.x -= fSegmentLength; break;  // X-
                case 2: nextPoint.y += fSegmentLength; break;  // Y+
                case 3: nextPoint.y -= fSegmentLength; break;  // Y-
                case 4: nextPoint.z += fSegmentLength; break;  // Z+
                case 5: nextPoint.z -= fSegmentLength; break;  // Z-
            }

            // Check if the next point is within grid bounds and not occupied
            if (nextPoint.x >= -GRID_SIZE / 2 && nextPoint.x < GRID_SIZE / 2 &&
                nextPoint.y >= -GRID_SIZE / 2 && nextPoint.y < GRID_SIZE / 2 &&
                nextPoint.z >= -GRID_SIZE / 2 && nextPoint.z < GRID_SIZE / 2 &&
                !grid[(int)roundf(nextPoint.x + GRID_SIZE / 2)]
                     [(int)roundf(nextPoint.y + GRID_SIZE / 2)]
                     [(int)roundf(nextPoint.z + GRID_SIZE / 2)]) {

                moved = true;  // Pipe can move in the current direction
                pipe.segments.push_back(nextPoint);
                grid[(int)roundf(nextPoint.x + GRID_SIZE / 2)]
                    [(int)roundf(nextPoint.y + GRID_SIZE / 2)]
                    [(int)roundf(nextPoint.z + GRID_SIZE / 2)] = true;
            }
        }

        // If the pipe didn't move in its current direction, try random directions
        if (!moved) {
            for (int attempts = 0; attempts < 6 && !moved; ++attempts) {
                nextPoint = last;  // Reset next point to the current position
                pipe.direction = rand() % 6;  // Choose a random direction

                // Move in the new random direction
                switch (pipe.direction) {
                    case 0: nextPoint.x += fSegmentLength; break;  // X+
                    case 1: nextPoint.x -= fSegmentLength; break;  // X-
                    case 2: nextPoint.y += fSegmentLength; break;  // Y+
                    case 3: nextPoint.y -= fSegmentLength; break;  // Y-
                    case 4: nextPoint.z += fSegmentLength; break;  // Z+
                    case 5: nextPoint.z -= fSegmentLength; break;  // Z-
                }

                // Check if the new direction is valid
                if (nextPoint.x >= -GRID_SIZE / 2 && nextPoint.x < GRID_SIZE / 2 &&
                    nextPoint.y >= -GRID_SIZE / 2 && nextPoint.y < GRID_SIZE / 2 &&
                    nextPoint.z >= -GRID_SIZE / 2 && nextPoint.z < GRID_SIZE / 2 &&
                    !grid[(int)roundf(nextPoint.x + GRID_SIZE / 2)]
                         [(int)roundf(nextPoint.y + GRID_SIZE / 2)]
                         [(int)roundf(nextPoint.z + GRID_SIZE / 2)]) {

                    moved = true;  // The pipe can move in this new direction
                    pipe.segments.push_back(nextPoint);
                    grid[(int)roundf(nextPoint.x + GRID_SIZE / 2)]
                        [(int)roundf(nextPoint.y + GRID_SIZE / 2)]
                        [(int)roundf(nextPoint.z + GRID_SIZE / 2)] = true;
                }
            }
        }

        // If the pipe could not move in any direction, destroy it and create a new one
        if (!moved) {
            // Free up the grid cells occupied by the current pipe
            for (const auto& segment : pipe.segments) {
                grid[(int)roundf(segment.x + GRID_SIZE / 2)]
                    [(int)roundf(segment.y + GRID_SIZE / 2)]
                    [(int)roundf(segment.z + GRID_SIZE / 2)] = false;
            }

            // Restart the pipe with a new random initial position
            pipe.segments.clear();
            Point3D newStart;
            newStart.x = rand() % GRID_SIZE - GRID_SIZE / 2;
            newStart.y = rand() % GRID_SIZE - GRID_SIZE / 2;
            newStart.z = rand() % GRID_SIZE - GRID_SIZE / 2;

            pipe.segments.push_back(newStart);

            grid[(int)roundf(newStart.x + GRID_SIZE / 2)]
                [(int)roundf(newStart.y + GRID_SIZE / 2)]
                [(int)roundf(newStart.z + GRID_SIZE / 2)] = true;
        }

        // Remove the oldest segments if the pipe has grown too long
        if (pipe.segments.size() > 25) {
            Point3D& first = pipe.segments.front();
            grid[(int)roundf(first.x + GRID_SIZE / 2)]
                [(int)roundf(first.y + GRID_SIZE / 2)]
                [(int)roundf(first.z + GRID_SIZE / 2)] = false;
            pipe.segments.erase(pipe.segments.begin());
        }
    }
}

void PipesGLView::DrawPipes() {
    for (const auto& pipe : pipes) {
        glColor3f(pipe.r, pipe.g, pipe.b);
        for (size_t i = 1; i < pipe.segments.size(); ++i) {
            DrawPipeSegment(pipe.segments[i-1], pipe.segments[i]);
            if (i < pipe.segments.size() - 1) {
                DrawJoint(pipe.segments[i-1], pipe.segments[i], pipe.segments[i+1]);
            }
        }
    }
}

void PipesGLView::DrawPipeSegment(const Point3D& start, const Point3D& end) {
    glPushMatrix();

    // Translate to the starting point of the segment
    glTranslatef(start.x, start.y, start.z);

    // Calculate the direction vector from start to end
    Point3D dir = {end.x - start.x, end.y - start.y, end.z - start.z};
    float length = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);

    // Normalize the direction vector
    dir.x /= length;
    dir.y /= length;
    dir.z /= length;

    // Calculate the rotation angle relative to the axes
    float rotAngle = acosf(dir.z) * 180.0f / M_PI;  // Angle between the pipe direction and the Z-axis
    float rotationAxisX = -dir.y; // Rotate around the Y-axis
    float rotationAxisY = dir.x;  // Rotate around the X-axis

    // Rotate the pipe relative to the calculated axis
    if (fabs(dir.z) < 0.99f) {
        glRotatef(rotAngle, rotationAxisX, rotationAxisY, 0.0f);
    } else if (dir.z < 0) {
        glRotatef(180.0f, 1.0f, 0.0f, 0.0f);  // To flip around the Z-axis if direction is downward
    }

    // Draw the cylinder
    GLUquadricObj *quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluCylinder(quadric, fPipeRadius, fPipeRadius, length, 14, 1);
    gluDeleteQuadric(quadric);

    glPopMatrix();
}


void PipesGLView::DrawJoint(const Point3D& prev, const Point3D& current, const Point3D& next) {
    glPushMatrix();
    glTranslatef(current.x, current.y, current.z);

    GLUquadricObj *quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluSphere(quadric, fPipeRadius * 1.1, 16, 16);
    gluDeleteQuadric(quadric);

    glPopMatrix();
}

// Implementation of PipesScreenSaver

PipesScreenSaver::PipesScreenSaver(BMessage* archive, image_id image)
    : BScreenSaver(archive, image), fGLView(nullptr),
      fPipeCount(10), fSegmentLength(1.0f), fPipeRadius(0.1f) {
        RestoreState(archive);
}

void PipesScreenSaver::StartConfig(BView* view) {
    PipesConfigView *configView = new PipesConfigView(view->Bounds(), this);
    view->AddChild(configView);
}

status_t PipesScreenSaver::SaveState(BMessage* into) const {
    into->AddInt32("pipe_count", fPipeCount);
    into->AddFloat("pipe_radius", fPipeRadius);
    return B_OK;
}

void PipesScreenSaver::RestoreState(BMessage* from)
{
    if (from != nullptr) {
        if (from->FindInt32("pipe_count", &fPipeCount) != B_OK)
            fPipeCount = 10;
        if (from->FindFloat("pipe_radius", &fPipeRadius) != B_OK)
            fPipeRadius = 0.1f;
    }
}

status_t PipesScreenSaver::StartSaver(BView* view, bool preview) {
    if (!fGLView) {
        BRect bounds = view->Bounds();
        fGLView = new PipesGLView(bounds);
        view->AddChild(fGLView);
    }
    fGLView->SetParameters(fPipeCount, fSegmentLength, fPipeRadius);
    view->Window()->SetPulseRate(50000);
    return B_OK;
}

void PipesScreenSaver::Draw(BView* view, int32 frame) {
    if (fGLView) {
        fGLView->Draw(view->Bounds());
    }
}

void PipesScreenSaver::SetPipeCount(int32 count) {
    fPipeCount = count;
    if (fGLView) fGLView->SetParameters(fPipeCount, fSegmentLength, fPipeRadius);
}

void PipesScreenSaver::SetPipeRadius(float radius) {
    fPipeRadius = radius;
    if (fGLView) fGLView->SetParameters(fPipeCount, fSegmentLength, fPipeRadius);
}

// Function to create an instance of the screensaver
extern "C" _EXPORT BScreenSaver* instantiate_screen_saver(BMessage* archive, image_id image) {
    return new PipesScreenSaver(archive, image);
}
