/*
 * Dark City Screen Saver
 *
 * This screensaver simulates a dark city with traffic.
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
#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <random>
#include <algorithm>

class CityScreenSaver;
class CityConfigView;
class CityGLView;

class CityScreenSaver : public BScreenSaver {
public:
    CityScreenSaver(BMessage* archive, image_id image);
    void StartConfig(BView* view) override;
    status_t SaveState(BMessage* into) const override;
    status_t StartSaver(BView* view, bool preview) override;
    void Draw(BView* view, int32 frame) override;
    void SetCitySpeed(float speed);
    void SetCarSpeed(float speed);
    float GetCitySpeed() { return citySpeed; }
    float GetCarSpeed() { return carSpeed; }

private:
    CityGLView* fGLView;
    float citySpeed;
    float carSpeed;
};

class CityConfigView : public BView {
public:
    CityConfigView(BRect frame, CityScreenSaver* saver)
        : BView(frame, "CityConfigView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
          fSaver(saver) {
        SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

        fNameStringView = new BStringView("nameString", "Dark City");
        fNameStringView->SetFont(be_bold_font);

        fCitySpeedSlider = new BSlider("citySpeedSlider", "Flight Speed", 
            new BMessage(MSG_CITY_SPEED_CHANGED), 0, 100, B_HORIZONTAL);
        fCitySpeedSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
        fCitySpeedSlider->SetLimitLabels("Slow", "Fast");
        fCitySpeedSlider->SetValue(fSaver->GetCitySpeed() * 100.0f);

        fCarSpeedSlider = new BSlider("carSpeedSlider", "Car Speed", 
            new BMessage(MSG_CAR_SPEED_CHANGED), 0, 100, B_HORIZONTAL);
        fCarSpeedSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
        fCarSpeedSlider->SetLimitLabels("Slow", "Fast");
        fCarSpeedSlider->SetValue(fSaver->GetCarSpeed() * 100.0f);

        BRect textRect = Bounds();
        textRect.InsetBy(5, 5);
        fInfoTextView = new BTextView(textRect, "infoTextView", textRect, B_FOLLOW_ALL_SIDES);
        fInfoTextView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
        fInfoTextView->MakeEditable(false);
        fInfoTextView->SetStylable(true);

        fInfoTextView->Insert("©2024 Claude 3.5 Sonnet by Anthropic\n\n");
        fInfoTextView->Insert("This screensaver simulates a dark city with traffic.\n");
        fInfoTextView->Insert("It was designed and implemented with the assistance of Claude, ");
        fInfoTextView->Insert("an AI created by Anthropic, demonstrating the capabilities ");
        fInfoTextView->Insert("of artificial intelligence in software development for Haiku.");

        BScrollView* infoScrollView = new BScrollView("infoScrollView", fInfoTextView, B_WILL_DRAW | B_FRAME_EVENTS, false, true);

        BGroupLayout* layout = new BGroupLayout(B_VERTICAL);
        SetLayout(layout);

        layout->SetInsets(B_USE_DEFAULT_SPACING);

        layout->AddView(fNameStringView);
        layout->AddView(fCitySpeedSlider);
        layout->AddView(fCarSpeedSlider);
        layout->AddView(infoScrollView);
    }

    void AttachedToWindow() override {
        fCitySpeedSlider->SetTarget(this);
        fCarSpeedSlider->SetTarget(this);
    }

    void MessageReceived(BMessage* message) override {
        switch (message->what) {
            case MSG_CITY_SPEED_CHANGED:
                fSaver->SetCitySpeed(fCitySpeedSlider->Value() / 100.0f);
                break;
            case MSG_CAR_SPEED_CHANGED:
                fSaver->SetCarSpeed(fCarSpeedSlider->Value() / 100.0f);
                break;
            default:
                BView::MessageReceived(message);
        }
    }

private:
    CityScreenSaver* fSaver;
    BStringView* fNameStringView;
    BSlider* fCitySpeedSlider;
    BSlider* fCarSpeedSlider;
    BTextView* fInfoTextView;

    static const uint32 MSG_CITY_SPEED_CHANGED = 'CSpd';
    static const uint32 MSG_CAR_SPEED_CHANGED = 'ASpd';
};

class Building {
public:
    float x, z;
    float width, depth;
    float height;
    Building(float x, float z, float width, float depth, float height)
        : x(x), z(z), width(width), depth(depth), height(height) {}
};

class CityBlock {
public:
    float x, z;
    float size;
    std::vector<Building> buildings;
    CityBlock(float x, float z, float size) : x(x), z(z), size(size) {
        GenerateBuildings();
    }
    void GenerateBuildings() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis_size(0.6, 1.4);
        std::uniform_real_distribution<> dis_height(5.0, 30.0);

        int buildingsPerSide = 4;
        float buildingSize = size * 0.75f / buildingsPerSide;
        float offset = size * 0.15f;

        for (int i = 0; i < buildingsPerSide; ++i) {
            for (int j = 0; j < buildingsPerSide; ++j) {
                float bx = x + offset + i * buildingSize;
                float bz = z + offset + j * buildingSize;
                float bw = buildingSize * dis_size(gen) * 0.8f;
                float bd = buildingSize * dis_size(gen) * 0.8f;
                float bh = dis_height(gen);
                buildings.emplace_back(bx, bz, bw, bd, bh);
            }
        }
    }
};

class Car {
public:
    float x, z;
    bool movingTowards;
    bool isHorizontal;
    Car(float x, float z, bool movingTowards, bool isHorizontal)
        : x(x), z(z), movingTowards(movingTowards), isHorizontal(isHorizontal) {}
};

struct Star {
    float x, y, z;
    float brightness;
};

class CityGLView : public BGLView {
public:
    CityGLView(BRect frame)
        : BGLView(frame, "CityGLView", B_FOLLOW_ALL, B_WILL_DRAW, BGL_RGB | BGL_DOUBLE | BGL_DEPTH),
          fWidth(frame.Width()), fHeight(frame.Height()) {		
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        generator.seed(seed);
        SetViewColor(0, 0, 0);
        InitializeCity();
		GenerateStarTexture();
        GenerateCars();
		InitializeCamera();
    }

    void AttachedToWindow() override {
        LockGL();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_NORMALIZE);
        GLfloat light_position[] = { -10.0f, 10.0f, 0.0f, 0.0f };
        GLfloat light_ambient[] = { 0.1f, 0.1f, 0.2f, 1.0f };
        GLfloat light_diffuse[] = { 0.5f, 0.5f, 0.6f, 1.0f };
        GLfloat light_specular[] = { 0.3f, 0.3f, 0.4f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

        GLfloat light1_position[] = { -0.5f, 0.5f, -0.5f, 0.0f };
        GLfloat light1_diffuse[] = { 0.1f, 0.1f, 0.2f, 1.0f };
        glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);

        GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat mat_diffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
        GLfloat mat_specular[] = { 0.6f, 0.6f, 0.6f, 1.0f };
        GLfloat mat_shininess[] = { 60.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, fWidth / fHeight, 0.1, 500.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        UnlockGL();
    }

    void Draw(BRect updateRect) {
        LockGL();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		DrawStarryBackground();

        glLoadIdentity();
        gluLookAt(camPosX, camPosY, camPosZ,
              camLookAtX, camLookAtY, camLookAtZ,
              0.0, 1.0, 0.0);

        DrawGround();
        DrawCity();
        DrawStreets();
        DrawCars();

        SwapBuffers();
        UnlockGL();
    }

    void SetCitySpeed(float speed) {
        citySpeed = speed;
    }

    void SetCarSpeed(float speed) {
        carSpeed = speed;
    }

private:
    float fWidth, fHeight;
    float fCityOffset = 0.0f;
    float camPosX, camPosY, camPosZ;
    float camLookAtX, camLookAtY, camLookAtZ;
    std::vector<CityBlock> fCityBlocks;
    std::vector<Car> fCars;
    std::vector<Star> stars;
    std::default_random_engine generator;
    std::normal_distribution<float> distribution{0.5f, 0.15f};
	GLuint starTexture;
    const float BLOCK_SIZE = 50.0f;
    const int CITY_SIZE = 3;
    float citySpeed = 0.2f;
    float carSpeed = 0.4f;
    const float STREET_WIDTH = 7.0f;
    const float BUILDING_AREA_SIZE = 0.0f;
    const int CARS_PER_LANE = 5;

    void InitializeCity() {
        for (int x = -CITY_SIZE; x <= CITY_SIZE; ++x) {
            for (int z = -CITY_SIZE; z <= CITY_SIZE; ++z) {
                fCityBlocks.emplace_back(x * BLOCK_SIZE, z * BLOCK_SIZE, BLOCK_SIZE);
            }
        }
    }

	void GenerateCars() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(-CITY_SIZE * BLOCK_SIZE, CITY_SIZE * BLOCK_SIZE);

		const int CARS_PER_LANE = 10;

		for (int i = -CITY_SIZE; i <= CITY_SIZE; ++i) {
			for (int j = 0; j < CARS_PER_LANE; ++j) {
				float z = i * BLOCK_SIZE;
				float x = dis(gen);
				fCars.emplace_back(x, z - STREET_WIDTH * 0.25f, true, true);
				fCars.emplace_back(x, z - STREET_WIDTH * 0.75f, false, true);
			}
		}

		for (int i = -CITY_SIZE; i <= CITY_SIZE; ++i) {
			for (int j = 0; j < CARS_PER_LANE; ++j) {
				float x = i * BLOCK_SIZE;
				float z = dis(gen);
				fCars.emplace_back(x - STREET_WIDTH * 0.75f, z, true, false);
				fCars.emplace_back(x - STREET_WIDTH * 0.25f, z, false, false);
			}
		}
	}

	void InitializeCamera() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> disPos(-120.0, 120.0);
		std::uniform_real_distribution<> disLookAt(-50.0, 50.0);

		camPosX = disPos(gen);
		camPosY = 80.0f;
		camPosZ = 70.0f;

		camLookAtX = disLookAt(gen);
		camLookAtY = 10.0f;
		camLookAtZ = -40.0f;
	}

    void UpdateCity() {
        for (auto& block : fCityBlocks) {
            block.z += citySpeed;
            for (auto& building : block.buildings) {
                building.z += citySpeed;
            }
        }
    
        for (auto it = fCityBlocks.begin(); it != fCityBlocks.end();) {
            if (it->z > BLOCK_SIZE * CITY_SIZE) {
                for (auto car_it = fCars.begin(); car_it != fCars.end();) {
                    if (car_it->z > it->z && car_it->z < it->z + BLOCK_SIZE) {
                        car_it = fCars.erase(car_it);
                    } else {
                        ++car_it;
                    }
                }
                it = fCityBlocks.erase(it);
            } else {
                ++it;
            }
        }
    
        while (fCityBlocks.size() < (2 * CITY_SIZE + 1) * (2 * CITY_SIZE + 1)) {
            float newZ = -BLOCK_SIZE * (CITY_SIZE + 1);
            for (int x = -CITY_SIZE; x <= CITY_SIZE; ++x) {
                fCityBlocks.emplace_back(x * BLOCK_SIZE, newZ, BLOCK_SIZE);
                GenerateCarsForBlock(fCityBlocks.back());
            }
        }
    
        fCityOffset += citySpeed;
        if (fCityOffset >= BLOCK_SIZE) {
            fCityOffset -= BLOCK_SIZE;
        }
    
        UpdateCars();
    }

	void GenerateCarsForBlock(const CityBlock& block) {
	    std::random_device rd;
	    std::mt19937 gen(rd());
	    std::uniform_real_distribution<> disHorisontal(-CITY_SIZE * BLOCK_SIZE, CITY_SIZE * BLOCK_SIZE);
	    std::uniform_real_distribution<> disVertical(0, BLOCK_SIZE);

		const int CARS_PER_LANE = 10;

		for (int j = 0; j < CARS_PER_LANE / CITY_SIZE; ++j) {
			float z = block.z;
			float x = disHorisontal(gen);
			fCars.emplace_back(x, z - STREET_WIDTH * 0.25f, true, true);
			fCars.emplace_back(x, z - STREET_WIDTH * 0.75f, false, true);
		}
		for (int j = 0; j < CARS_PER_LANE / CITY_SIZE; ++j) {
			float x = block.x;
			float z = disVertical(gen);
			fCars.emplace_back(x - STREET_WIDTH * 0.75f, z, true, false);
			fCars.emplace_back(x - STREET_WIDTH * 0.25f, z, false, false);
		}
	}

    void UpdateCars() {
        for (auto& car : fCars) {
            car.z += citySpeed;
    
            if (car.isHorizontal) {
                if (car.movingTowards) {
                    car.x += carSpeed;
                    if (car.x > BLOCK_SIZE * (CITY_SIZE + 1)) {
                        car.x = -BLOCK_SIZE * (CITY_SIZE + 1);
                    }
                } else {
                    car.x -= carSpeed;
                    if (car.x < -BLOCK_SIZE * (CITY_SIZE + 1)) {
                        car.x = BLOCK_SIZE * (CITY_SIZE + 1);
                    }
                }
            } else {
                if (car.movingTowards) {
                    car.z += carSpeed;
                    if (car.z > BLOCK_SIZE * (CITY_SIZE + 1)) {
                        car.z = -BLOCK_SIZE * (CITY_SIZE + 1);
                    }
                } else {
                    car.z -= carSpeed;
                    if (car.z < -BLOCK_SIZE * (CITY_SIZE + 1)) {
                        car.z = BLOCK_SIZE * (CITY_SIZE + 1);
                    }
                }
            }
        }
    }

    void DrawCity() {
        UpdateCity();
        for (auto& block : fCityBlocks) {
            DrawCityBlock(block);
        }
    }

    void DrawCityBlock(CityBlock& block) {
        for (auto& building : block.buildings) {
            DrawBuilding(building);
        }
    }

    void DrawBuilding(const Building& building) {
        glPushMatrix();
        glTranslatef(building.x, building.height * 0.5f, building.z);
        glScalef(building.width, building.height, building.depth);

        float distance = sqrt(building.x * building.x + building.z * building.z);
        float fogFactor = 1.0f - std::min(distance / 150.0f, 1.0f);

        glColor3f(0.2f * fogFactor, 0.2f * fogFactor, 0.4f * fogFactor);

        glutSolidCube(1.0f);

        glPopMatrix();
    }

    void DrawGround() {
        glColor3f(0.0f, 0.0f, 0.0f);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        float groundSize = BLOCK_SIZE * (CITY_SIZE * 2 + 1);
        glVertex3f(-groundSize, -0.1f, -groundSize);
        glVertex3f(groundSize, -0.1f, -groundSize);
        glVertex3f(groundSize, -0.1f, groundSize);
        glVertex3f(-groundSize, -0.1f, groundSize);
        glEnd();
    }

    void DrawStreets() {
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
    
        for (const auto& block : fCityBlocks) {
            float x = block.x;
            float z = block.z;
            float horizontalStreetHeight = 0.02f;
            float verticalStreetHeight = 0.05f;
            float distance = sqrt(x * x + z * z);
            float fogFactor = 1.0f - std::min(distance / 200.0f, 1.0f);
    
            glColor3f(0.1f * fogFactor, 0.1f * fogFactor, 0.1f * fogFactor);
    
            glVertex3f(x, horizontalStreetHeight, z);
            glVertex3f(x + BLOCK_SIZE, horizontalStreetHeight, z);
            glVertex3f(x + BLOCK_SIZE, horizontalStreetHeight, z - STREET_WIDTH);
            glVertex3f(x, horizontalStreetHeight, z - STREET_WIDTH);
    
            glVertex3f(x, verticalStreetHeight, z);
            glVertex3f(x - STREET_WIDTH, verticalStreetHeight, z);
            glVertex3f(x - STREET_WIDTH, verticalStreetHeight, z + BLOCK_SIZE);
            glVertex3f(x, verticalStreetHeight, z + BLOCK_SIZE);
        }
        glEnd();
    }

    void DrawCars() {
        glEnable(GL_POINT_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (const auto& car : fCars) {
            float distance = sqrt(car.x * car.x + car.z * car.z);
            float fogFactor = std::max(0.0f, 1.0f - distance / 300.0f);
            float size = std::max(1.0f, 4.0f * fogFactor);
            glPointSize(size);
            glBegin(GL_POINTS);
            if (car.movingTowards) {
                glColor4f(1.0f, 1.0f, 1.0f, fogFactor);
            } else {
                glColor4f(1.0f, 0.0f, 0.0f, fogFactor);
            }
            float offset = 0.2f;
            if (car.isHorizontal) {
                glVertex3f(car.x, 0.2f, car.z - offset);
                glVertex3f(car.x, 0.2f, car.z + offset);
            } else {
                glVertex3f(car.x - offset, 0.2f, car.z);
                glVertex3f(car.x + offset, 0.2f, car.z);
            }
            glEnd();
            if (distance < 100.0f) {
                float glowIntensity = (1.0f - distance / 50.0f) * 0.5f;
                glPointSize(size * 2.0f);
                glBegin(GL_POINTS);
                if (car.movingTowards) {
                    glColor4f(1.0f, 1.0f, 1.0f, glowIntensity);
                } else {
                    glColor4f(1.0f, 0.0f, 0.0f, glowIntensity);
                }
                if (car.isHorizontal) {
                    glVertex3f(car.x, 0.2f, car.z - offset);
                    glVertex3f(car.x, 0.2f, car.z + offset);
                } else {
                    glVertex3f(car.x - offset, 0.2f, car.z);
                    glVertex3f(car.x + offset, 0.2f, car.z);
                }
                glEnd();
            }
        }
    }

    void GenerateStarTexture() {
        const int texSize = 512;
        uint8* texData = new uint8[texSize * texSize * 4];
        std::fill_n(texData, texSize * texSize * 4, 0);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, texSize - 1);
        std::uniform_int_distribution<> brightness(100, 255);

        for (int i = 0; i < 500; ++i) {
            int x = dis(gen);
            int y = dis(gen);
            uint8 b = brightness(gen);
            texData[(y * texSize + x) * 4 + 0] = b;
            texData[(y * texSize + x) * 4 + 1] = b;
            texData[(y * texSize + x) * 4 + 2] = b;
            texData[(y * texSize + x) * 4 + 3] = 255;
        }

        glGenTextures(1, &starTexture);
        glBindTexture(GL_TEXTURE_2D, starTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        delete[] texData;
    }

    void DrawStarryBackground() {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, starTexture);
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(-1, 1, -1, 1, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-1, 0, -1);
        glTexCoord2f(1, 0); glVertex3f(1, 0, -1);
        glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
        glTexCoord2f(0, 1); glVertex3f(-1, 1, -1);
        glEnd();

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glDisable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }
};


CityScreenSaver::CityScreenSaver(BMessage* archive, image_id image)
        : BScreenSaver(archive, image), fGLView(nullptr) {
    if (archive != nullptr) {
	    if (archive->FindFloat("city_speed", &citySpeed) != B_OK) {
	        citySpeed = 0.2f;
	    }
	    if (archive->FindFloat("car_speed", &carSpeed) != B_OK) {
	        carSpeed = 0.4f;
	    }
    } else {
    	citySpeed = 0.2f;
    	carSpeed = 0.4f;
    }
}

void CityScreenSaver::StartConfig(BView* view) {
    CityConfigView *configView = new CityConfigView(view->Bounds(), this);
    view->AddChild(configView);
}

status_t CityScreenSaver::SaveState(BMessage* into) const {
    into->AddFloat("city_speed", citySpeed);
    into->AddFloat("car_speed", carSpeed);
    return B_OK;
}

status_t CityScreenSaver::StartSaver(BView* view, bool preview) {
    if (!fGLView) {
        BRect bounds = view->Bounds();
        fGLView = new CityGLView(bounds);        
        view->AddChild(fGLView);
        fGLView->SetCitySpeed(citySpeed);
        fGLView->SetCarSpeed(carSpeed);
    }
    view->Window()->SetPulseRate(50000);
    return B_OK;
}

void CityScreenSaver::Draw(BView* view, int32 frame) {
    if (fGLView) {
        fGLView->Draw(view->Bounds());
    }
}

void CityScreenSaver::SetCitySpeed(float speed) {
    citySpeed = speed;
    if (fGLView) {
        fGLView->SetCitySpeed(speed);
    }
}

void CityScreenSaver::SetCarSpeed(float speed) {
    carSpeed = speed;
    if (fGLView) {
        fGLView->SetCarSpeed(speed);
    }
}

extern "C" _EXPORT BScreenSaver* instantiate_screen_saver(BMessage* archive, image_id image) {
    return new CityScreenSaver(archive, image);
}
