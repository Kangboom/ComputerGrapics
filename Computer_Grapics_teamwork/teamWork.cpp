#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLAUX.H>
#include <Windows.h>
#include "Camera.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "bitmap_fonts.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace glm;
camera myCamera;

int lastX = 0, lastY = 0;
bool mousePressed = false;

float eyeX = 0.2, eyeY = 0.3, eyeZ = 0.5; // 초기 카메라 위치
float centerX = 0.0, centerY = 0.0, centerZ = 0.0; // 초기 시선 방향
float upX = 0.0, upY = 0.0, upZ = 1.0; // 초기 업 벡터

// 태양계 공전, 자전, 텍스쳐
static float Day = 0, Day1 = 0, Day2 = 0, Day3 = 0, Day4 = 0, Day5 = 0, Day6 = 0, Day7 = 0; // 공전 속도 조절
static int Time = 0; // 자전 속도
GLUquadricObj* obj; // 토성고리
GLUquadricObj* sun, * mercury, * venus, * earth, * mars, * jupiter, * saturn, * saturn_ring, * uranus, * neptune, * moon;
AUX_RGBImageRec* pTextureImage[11];    
AUX_RGBImageRec* TextureImage[6];

// 텍스처 배열
GLuint PlanetTexture[11]; // 행성 텍스처
GLuint tex[6];  // 스카이 박스
GLuint texture[1]; // fire 택스쳐
GLuint spaceship_bottom; 
GLuint spaceship_left;
GLuint spaceship_right;
GLuint spaceship_top;


// 태양계
float solarSystemScale = 3.0; // 태양계 크기 조절 변수
float SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;
float globalSpeed = 0.5; // 공전과 자전 속도를 일괄 조절하는 변수
float size = 2; // 행성들 크기 조절 변수

// 시점 모드
bool freeMode = false; 
int planet_view = 0;

// fire 관련 변수
#define MAX_PARTICLES   5000
float slowdown = 2.0f, xspeed, yspeed, zoom = -20.0f;
float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;
bool upflag, downflag, rightflag, leftflag, reset;

// 스크린 너비, 높이
int screen_width;
int screen_height;

// keybord 관련
bool wKeyPressed = false;
bool sKeyPressed = false;
float currentSpeed = 0.0f;
float acceleration = 0.001f; // 가속도 조절값
float movespeed = 0.01f;

// 불관련 변수
typedef struct {
    bool    active;
    float   life;
    float   fade;
    vec3 color;
    vec3 pos;
    vec3 vel;
    vec3 force;
}
particles;
particles particle[MAX_PARTICLES];

int    g_nWindowWidth;
int    g_nWindowHeight;
float g_fDistance = -5.0f;
float g_fSpinX1 = 0.0f;
float g_fSpinY1 = 0.0f;

// 메세지
static char message[255] = "Velocity";

// 이미지 로더
AUX_RGBImageRec* LoadBMP(char* szFilename) { // 이미지파일을 메모리로 가져온다.

    FILE* pFile = NULL;
    if (!szFilename) {
        return NULL;
    }
    pFile = fopen(szFilename, "r");
    if (pFile) {
        fclose(pFile);
        return auxDIBImageLoad(szFilename);
    }
    return NULL;
}

void InitLight() {
    GLfloat light_specular[] = { 1.0, 1.0, 1.0 };   // 직접광
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0 };   // 반사광
    GLfloat light_ambient[] = { 1.0, 1.0, 1.0 };   // 주변광
    GLfloat light_position[] = { 0.0, 0.0, 0.0 };   // 광원의 위치
    glShadeModel(GL_SMOOTH);       // 구로 셰이딩
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.8); // 주변광 좀 더 밝게 설정
}

// 텍스처를 로드
int LoadPlanetTextures() {
    // 객체 생성
    sun = gluNewQuadric();
    mercury = gluNewQuadric();
    venus = gluNewQuadric();
    earth = gluNewQuadric();
    mars = gluNewQuadric();
    jupiter = gluNewQuadric();
    saturn = gluNewQuadric();
    saturn_ring = gluNewQuadric();
    uranus = gluNewQuadric();
    neptune = gluNewQuadric();
    moon = gluNewQuadric();
    int Status = FALSE;


    // 텍스처 맵핑사용
    gluQuadricTexture(sun, GL_TRUE);
    gluQuadricTexture(mercury, GL_TRUE);
    gluQuadricTexture(venus, GL_TRUE);
    gluQuadricTexture(earth, GL_TRUE);
    gluQuadricTexture(mars, GL_TRUE);
    gluQuadricTexture(jupiter, GL_TRUE);
    gluQuadricTexture(saturn, GL_TRUE);
    gluQuadricTexture(saturn_ring, GL_TRUE);
    gluQuadricTexture(uranus, GL_TRUE);
    gluQuadricTexture(neptune, GL_TRUE);
    gluQuadricTexture(moon, GL_TRUE);

    memset(pTextureImage, 0, sizeof(void*) * 1);        // 포인터 초기화
    if ((pTextureImage[0] = LoadBMP("../Data/Planets/Sun.bmp")) && (pTextureImage[1] = LoadBMP("../Data/Planets/Mercury.bmp")) &&
        (pTextureImage[2] = LoadBMP("../Data/Planets/Venus.bmp")) && (pTextureImage[3] = LoadBMP("../Data/Planets/Earth.bmp")) &&
        (pTextureImage[4] = LoadBMP("../Data/Planets/Mars.bmp")) && (pTextureImage[5] = LoadBMP("../Data/Planets/Jupiter.bmp")) &&
        (pTextureImage[6] = LoadBMP("../Data/Planets/Saturn.bmp")) && (pTextureImage[7] = LoadBMP("../Data/Planets/Saturn_ring.bmp")) &&
        (pTextureImage[8] = LoadBMP("../Data/Planets/Uranus.bmp")) && (pTextureImage[9] = LoadBMP("../Data/Planets/Neptune.bmp")) &&
        (pTextureImage[10] = LoadBMP("../Data/Planets/Moon.bmp")))
    {
        Status = TRUE;
        glGenTextures(11, &PlanetTexture[0]);   // 텍스처 11개 생성
        for (int i = 0; i <= 10; i++) {
            glBindTexture(GL_TEXTURE_2D, PlanetTexture[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, 3, pTextureImage[i]->sizeX,
                pTextureImage[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE,
                pTextureImage[i]->data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // 이미지파일하고 물체의 크기를 맞춰준다.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glEnable(GL_TEXTURE_2D);
        }
    }


    for (int i = 0; i <= 10; i++) {       // 텍스처 공간 반납
        if (pTextureImage[i]) {
            if (pTextureImage[i]->data) {
                free(pTextureImage[i]->data);
            }
            free(pTextureImage[i]);
        }
    }

    return Status;
}
void LoadSkyboxTextures() {
    
    char* skyboxFileNames[] = {
        "../Data/skybox/blue/back.bmp",    // 뒷쪽 면
        "../Data/skybox/blue/front.bmp",  // 앞쪽 면
        "../Data/skybox/blue/right.bmp",  // 오른쪽 면
        "../Data/skybox/blue/left.bmp",   // 왼쪽 면
        "../Data/skybox/blue/top.bmp",    // 위쪽 면
        "../Data/skybox/blue/bot.bmp", // 아래쪽 면
    };

    glGenTextures(6, tex);
    for (int i = 0; i < 6; i++) {
        AUX_RGBImageRec* textureImage = LoadBMP(skyboxFileNames[i]);
        if (textureImage) {
            glBindTexture(GL_TEXTURE_2D, tex[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, 3, textureImage->sizeX,
                textureImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage->data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            free(textureImage->data);
            free(textureImage);
        }
        else {
            printf("스카이박스 텍스처 로딩 실패: %s\n", skyboxFileNames[i]);
        }
    }
}
void LoadFireTextures() {
    AUX_RGBImageRec* TextureImage[1];
    memset(TextureImage, 0, sizeof(void*) * 1);

    if (TextureImage[0] = LoadBMP("../Data/Sun.bmp")) {
        glGenTextures(1, &texture[0]);
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    if (TextureImage[0]) {
        if (TextureImage[0]->data) {
            free(TextureImage[0]->data);
        }

        free(TextureImage[0]);
    }
}
GLuint LoadPNGTexture(const char* filename) {
    GLuint textureID;
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

    if (!image) {
        fprintf(stderr, "이미지를 불러올 수 없습니다: %s\n", filename);
        return 0;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    stbi_image_free(image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

// 우주선 UI 그리기
void DrawHUD() {

    // 2D 모드
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1024, 0, 1024);


    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0, 1.0, 1.0);

    // 우주선 아래 UI
    glBindTexture(GL_TEXTURE_2D, spaceship_bottom);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1024, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1024, 200);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 200);
    glEnd();

    // 우주선 왼쪽 UI
    glBindTexture(GL_TEXTURE_2D, spaceship_left);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 200);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(180, 200);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(180, 1024);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 1024);
    glEnd();

    // 우주선 오른쪽 UI
    glBindTexture(GL_TEXTURE_2D, spaceship_right);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(844, 200);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1024, 200);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1024, 1024);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(844, 1024);
    glEnd();

    // 우주선 위 UI
    glBindTexture(GL_TEXTURE_2D, spaceship_top);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(155, 900);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(865, 900);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(865, 1024);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(155, 1024);
    glEnd();

    // 원래 모드로 복귀
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    endRenderText();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
}

// 스카이박스 그리기
void DrawSkybox() {
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    float r = 1000.0f;

    // 스카이박스의 뒷면을 그립니다.
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-r, -r / 2, -r);
    glTexCoord2f(1, 0); glVertex3f(r, -r / 2, -r);
    glTexCoord2f(1, 1); glVertex3f(r, r, -r);
    glTexCoord2f(0, 1); glVertex3f(-r, r, -r);
    glEnd();

    // 스카이박스의 앞면을 그립니다.
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(r, -r / 2, r);
    glTexCoord2f(1, 0); glVertex3f(-r, -r / 2, r);
    glTexCoord2f(1, 1); glVertex3f(-r, r, r);
    glTexCoord2f(0, 1); glVertex3f(r, r, r);
    glEnd();

    // 스카이박스의 오른쪽 면을 그립니다.
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(r, -r / 2, -r);
    glTexCoord2f(1, 0); glVertex3f(r, -r / 2, r);
    glTexCoord2f(1, 1); glVertex3f(r, r, r);
    glTexCoord2f(0, 1); glVertex3f(r, r, -r);
    glEnd();

    // 스카이박스의 왼쪽 면을 그립니다.
    glBindTexture(GL_TEXTURE_2D, tex[3]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-r, -r / 2, r);
    glTexCoord2f(1, 0); glVertex3f(-r, -r / 2, -r);
    glTexCoord2f(1, 1); glVertex3f(-r, r, -r);
    glTexCoord2f(0, 1); glVertex3f(-r, r, r);
    glEnd();

    // 스카이박스의 위쪽 면을 그립니다.
    glBindTexture(GL_TEXTURE_2D, tex[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-r, r, -r);
    glTexCoord2f(1, 0); glVertex3f(r, r, -r);
    glTexCoord2f(1, 1); glVertex3f(r, r, r);
    glTexCoord2f(0, 1); glVertex3f(-r, r, r);
    glEnd();

    // 스카이박스의 아래쪽 면을 그립니다.
    glBindTexture(GL_TEXTURE_2D, tex[5]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-r, -r / 2, r);
    glTexCoord2f(1, 0); glVertex3f(r, -r / 2, r);
    glTexCoord2f(1, 1); glVertex3f(r, -r / 2, -r);
    glTexCoord2f(0, 1); glVertex3f(-r, -r / 2, -r);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
}

// 불 설정
void SetParticle(int loop)
{
    particle[loop].active = true;
    particle[loop].life = 1.0f;
    particle[loop].fade = float(rand() % 80) / 1000.0f + 0.003f;
    particle[loop].color = vec3(219.0f / 255.0f , 79.0f / 255.0f , 2.0f / 255.0f);
    particle[loop].vel = vec3((rand() % 50) - 25.0f, (rand() % 50) - 25.0f, (rand() % 50) - 25.0f);
    particle[loop].force = vec3(0, 0.6, 0);

    if (loop % 3 == 0) {
        particle[loop].pos = vec3(7.0f, 0.0f, -4.0f);
    }
    if (loop % 3 == 1) {
        particle[loop].pos = vec3(-7.0f, 0.0f, -4.0f);
    }
    if (loop % 3 == 2) {
        particle[loop].pos = vec3(0.0f, 0.0f, -3.0f);
    }
}
int InitGL(GLvoid) {
    upflag = false;
    downflag = false;
    leftflag = false;
    rightflag = false;
    reset = false;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    for (int loop = 0; loop < MAX_PARTICLES; loop++) {
        SetParticle(loop);
    }

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
    return TRUE;
}

// 태양계 그리기
void DrawOrbit(float radius) {
    glColor3f(1.0f, 1.0f, 1.0f); // 흰색으로 설정
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP); 
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.14159265f / 180.0f;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glVertex3f(x, y, 0.0f); // 궤도 상의 점을 추가
    }
    glEnd(); // 루프 그리기 종료
}
void DrawSolarSystem() {
 
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    // 태양
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[0]);
    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    gluSphere(sun, 0.13 * size, 20, 16); 
    glEnable(GL_LIGHTING); // 조명
    glEnable(GL_LIGHT0); 

    // 수성
    glPushMatrix(); // 태양 좌표계
    glRotatef((GLfloat)Day, 0.0, 0.0, 1.0);
    glTranslatef(0.15 * solarSystemScale, 0.0, 0.0); 
    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[1]);
    gluSphere(mercury, 0.026 * size , 10, 8); 
    glPopMatrix();

    // 금성
    glPushMatrix();
    glRotatef((GLfloat)Day1, 0.0, 0.0, 1.0); 
    glTranslatef(0.3 * solarSystemScale, 0.0, 0.0); 
    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[2]);
    gluSphere(venus, 0.05 * size, 10, 8);
    glPopMatrix();

    // 지구
    glPushMatrix(); 
    glRotatef((GLfloat)Day2, 0.0, 0.0, 1.0); 
    glTranslatef(0.55 * solarSystemScale, 0.0, 0.0);
    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[3]);
    gluSphere(earth, 0.06 * size, 10, 8); 
    DrawOrbit(0.11 * solarSystemScale); // 달 궤도 그리기

    // 달
    glPushMatrix(); // 지구 좌표계
    glRotatef((GLfloat)Day1, 0.0, 0.0, 1.0); // 달 공전
    glTranslatef(0.11 * solarSystemScale, 0.0, 0.0); 
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[10]);
    gluSphere(moon, 0.015 * size, 10, 8);
    glPopMatrix();
    glPopMatrix(); // 태양 좌표계로 리턴

    // 화성
    glPushMatrix(); 
    glRotatef((GLfloat)Day3, 0.0, 0.0, 1.0);
    glTranslatef(0.7 * solarSystemScale, 0.0, 0.0);
    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[4]);
    gluSphere(mars, 0.05 * size, 10, 8);
    glPopMatrix();

    // 목성
    glPushMatrix();
    glRotatef((GLfloat)Day4, 0.0, 0.0, 1.0);
    glTranslatef(1.0 * solarSystemScale, 0.0, 0.0);
    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[5]);
    gluSphere(jupiter, 0.12 * size, 10, 8);
    glPopMatrix();

    // 토성
    glPushMatrix();
    glRotatef((GLfloat)Day5, 0.0, 0.0, 1.0);
    glTranslatef(1.4 * solarSystemScale, 0.0, 0.0);
    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[6]);
    gluSphere(saturn, 0.085 * size, 10, 8);

    // 토성 고리
    glPushMatrix();
    obj = gluNewQuadric();
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[7]);
    gluDisk(saturn_ring, 0.1, 0.17, 10, 8); 
    glPopMatrix();
    glPopMatrix();

    // 천왕성
    glPushMatrix();
    glRotatef((GLfloat)Day6, 0.0, 0.0, 1.0);
    glTranslatef(1.7 * solarSystemScale, 0.0, 0.0);
    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[8]);
    gluSphere(uranus, 0.07 * size, 10, 8);
    glPopMatrix();

    // 해왕성
    glPushMatrix();
    glRotatef((GLfloat)Day7, 0.0, 0.0, 1.0);
    glTranslatef(1.9 * solarSystemScale, 0.0, 0.0);
    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[9]);
    gluSphere(neptune, 0.07 * size, 10, 8);
    glPopMatrix();

    // 각 행성의 공전 궤도 그리기
    DrawOrbit(0.15 * solarSystemScale); // 수성 궤도
    DrawOrbit(0.3 * solarSystemScale); // 금성 궤도
    DrawOrbit(0.55 * solarSystemScale); // 지구 궤도
    DrawOrbit(0.7 * solarSystemScale); // 화성 궤도
    DrawOrbit(1.0 * solarSystemScale); // 목성 궤도
    DrawOrbit(1.4 * solarSystemScale); // 토성 궤도
    DrawOrbit(1.7 * solarSystemScale); // 천왕성 궤도
    DrawOrbit(1.9 * solarSystemScale); // 해왕성 궤도
}
void DrawSun() {
    // 태양
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[0]);
    gluSphere(sun, 0.13, 25, 20); // 태양을 그려냄

    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST); // 은면제거

    glRotatef(-g_fSpinY, 1.0f, 0.0f, 0.0f);
    glRotatef(-g_fSpinX, 0.0f, 1.0f, 0.0f);

    for (int loop = 0; loop < MAX_PARTICLES; loop++) {
        if (particle[loop].active) {
            float x = particle[loop].pos[0];
            float y = particle[loop].pos[1];
            float z = particle[loop].pos[2] + zoom;

            glDisable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, texture[0]);
            glColor4f(particle[loop].color[0], particle[loop].color[1], particle[loop].color[2], particle[loop].life);

            glBegin(GL_QUADS);
            glTexCoord2d(0, 0); glVertex3f(x - 0.5f, y - 2.5f, z);
            glTexCoord2d(1, 0); glVertex3f(x + 0.5f, y - 2.5f, z);
            glTexCoord2d(1, 1); glVertex3f(x + 0.5f, y + 0.5f, z);
            glTexCoord2d(0, 1); glVertex3f(x - 0.5f, y + 0.5f, z);
            glEnd();

            particle[loop].pos += particle[loop].vel / (slowdown * 1000);
            particle[loop].vel += particle[loop].force;
            particle[loop].life -= particle[loop].fade;

            if (particle[loop].life < 0.0f) {
                SetParticle(loop);
            }

            if (upflag && (particle[loop].force[1] < 1.5f)) particle[loop].force[1] += 0.01f;
            if (downflag && (particle[loop].force[1] > -1.5f)) particle[loop].force[1] -= 0.01f;
            if (rightflag && (particle[loop].force[0] < 1.5f)) particle[loop].force[0] += 0.01f;
            if (leftflag && (particle[loop].force[0] > -1.5f)) particle[loop].force[0] -= 0.01f;
        }
    }
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
void DrawMercury() {
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[1]);
    gluSphere(mercury, 0.026, 25, 20);
}
void DrawVenus() {
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST); 

    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[2]);
    gluSphere(venus, 0.05, 25, 20); 
    glPopMatrix();
}
void DrawEarth() {
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST); 

    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[3]);
    gluSphere(earth, 0.06, 15, 10); 

    // 달 그리기
    glPushMatrix(); // 지구기준의 좌표계
    glRotatef((GLfloat)Day1, 0.0, 0.0, 1.0);
    glTranslatef(0.1, 0.0, 0.0); 
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[10]);
    gluSphere(moon, 0.015, 15, 10); 
    glPopMatrix();

}
void DrawMars() {
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST); 

    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[4]);
    gluSphere(mars, 0.05, 25, 20); 

}
void DrawJupiter() {
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST); 

    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[5]);
    gluSphere(jupiter, 0.12, 25, 20); 
}
void DrawSaturn() {
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);

    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[6]);
    gluSphere(saturn, 0.085, 25, 20);

    glPushMatrix();
    obj = gluNewQuadric(); 
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[7]);
    gluDisk(saturn_ring, 0.1, 0.13, 30, 30); 
    glPopMatrix();

}
void DrawUranus() {
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST); 

    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[8]);
    gluSphere(uranus, 0.07, 25, 20);
    glPopMatrix();

}
void DrawNeptune() {
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST); 

    glRotatef((GLfloat)Time, 0.0, 0.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, PlanetTexture[9]);
    gluSphere(neptune, 0.07, 25, 20); 
}

// MyDisplay 
void MyDisplay() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 
   
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // 스카이박스 그리기
    glPushMatrix();
        DrawSkybox();
    glPopMatrix();

    // 시점 설정
    gluLookAt(
        myCamera.eye.x, myCamera.eye.y, myCamera.eye.z,
        myCamera.at.x, myCamera.at.y, myCamera.at.z,
        myCamera.up.x, myCamera.up.y, myCamera.up.z
    );

    if (planet_view == 1) DrawSun();
    else if (planet_view == 2) DrawMercury();
    else if (planet_view == 3) DrawVenus();
    else if (planet_view == 4) DrawEarth();
    else if (planet_view == 5) DrawMars();
    else if (planet_view == 6) DrawJupiter();
    else if (planet_view == 7) DrawSaturn();
    else if (planet_view == 8) DrawUranus();
    else if (planet_view == 9) DrawNeptune();
    else DrawSolarSystem();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    DrawHUD();

    if (freeMode) {
        beginRenderText(1024, 960); 
        {
            glColor3f(1.0f, 1.0f, 1.0f);
            renderText(450, 300, BITMAP_FONT_TYPE_TIMES_ROMAN_24, message);
        }
        endRenderText();
    }

    glutPostRedisplay();
    glutSwapBuffers();
}

// 키보드 콜백 관련
void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_F1:
        // 'F1' 키가 눌렸을 때 실행할 동작 추가
        freeMode = !freeMode;
        planet_view = 0;
        myCamera.InitCamera(vec3(0.3, 0.4, 2), vec3(0, 0, 0), vec3(0, 0, 1));
        break;
    }
}
void keyboard(unsigned char key, int x, int y) {
    
    if (freeMode) {
        switch (key) {
        case 'w':
            wKeyPressed = true; // w 키 눌림 (움직임은 UpdateCameraPostion 에서 변경 하자)
            break;
        case 's':
            sKeyPressed = true; // s 키 눌림           
            break;
        case 'a':           
            myCamera.rotatecamera(myCamera.up, 5); // 왼쪽 회전
            break;
        case 'd':     
            myCamera.rotatecamera(-myCamera.up, 5); // 오른쪽 회전;
            break;
        case 'q':
            myCamera.rotatecamera(myCamera.right, 5); // 위로 회전
            break;
        case 'e':
            myCamera.rotatecamera(-myCamera.right, 5); // 아래로 회전
            break;
        case 'r':
            myCamera.InitCamera(vec3(0.3, 0.4, 2), vec3(0, 0, 0), vec3(0, 0, 1)); // 처음 시점으로 초기화
            break;
        }
    }
    else {
        switch (key) {
        case '1':
            // 태양
            myCamera.eye = vec3(0.3f, 0.0f, 0.0f); 
            myCamera.at = vec3(0.0f, 0.0f, 0.0f);
            planet_view = 1;
            break;
        case '2':
            // 수성
            myCamera.eye = vec3(0.05f * solarSystemScale, 0.0f, 0.0f); 
            myCamera.at = vec3(0.0f, 0.0f, 0.0f); 
            planet_view = 2;
            break;
        case '3':
            // 금성
            myCamera.eye = vec3(0.1f * solarSystemScale, 0.0f, 0.0f); 
            myCamera.at = vec3(0.0f, 0.0f, 0.0f); 
            planet_view = 3;
            break;
        case '4':
            // 지구
            myCamera.eye = vec3(0.15f * solarSystemScale, 0.0f, 0.1f); 
            myCamera.at = vec3(0.0f, 0.0f, 0.0f); 
            planet_view = 4;
            break;
        case '5':
            // 화성
            myCamera.eye = vec3(0.1f * solarSystemScale, 0.0f, 0.0f); 
            myCamera.at = vec3(0.0f, 0.0f, 0.0f); 
            planet_view = 5;
            break;
        case '6':
            // 목성
            myCamera.eye = vec3(0.1f * solarSystemScale + 0.1, 0.0f, 0.0f); 
            myCamera.at = vec3(0.0f, 0.0f, 0.0f);
            planet_view = 6;
            break;
        case '7':
            // 토성
            myCamera.eye = vec3(0.1f * solarSystemScale + 0.1, 0.0f, 0.1f); 
            myCamera.at = vec3(0.0f, 0.0f, 0.0f); 
            planet_view = 7;
            break;
        case '8':
            // 천왕성
            myCamera.eye = vec3(0.1f * solarSystemScale, 0.0f, 0.0f); 
            myCamera.at = vec3(0.0f, 0.0f, 0.0f); 
            planet_view = 8;
            break;
        case '9':
            // 해왕성
            myCamera.eye = vec3(0.1f * solarSystemScale, 0.0f, 0.0f);
            myCamera.at = vec3(0.0f, 0.0f, 0.0f); 
            planet_view = 9;
            break;
        }
    }
    myCamera.updatecamera();
    glutPostRedisplay();
}
void keyboardUp(unsigned char key, int x, int y) {
    
    if (freeMode) {
        switch (key) {
        case 'w':
            wKeyPressed = false; // KeyPressed 플래그로 키보드 눌림 컨트롤 
            break;
        case 's':
            sKeyPressed = false;
            break;
        }
    }
}

// 마우스 콜백관련
void motion(int x, int y) {
    if (freeMode) {
        int deltaX = -(x - lastX);
        int deltaY = -(y - lastY);

        // 속도 조절
        float sensitivity = 0.1f; 

        // 마우스 움직임에 기반하여 카메라 회전
        myCamera.rotatecamera(myCamera.up, deltaX * sensitivity);
        myCamera.rotatecamera(myCamera.right, deltaY * sensitivity);

        lastX = x;
        lastY = y;

        myCamera.updatecamera();
        glutPostRedisplay();
    }
}

// 타이머 콜백관련
void updateCameraPosition() {
    float speed_control = 0.016f;

    if (wKeyPressed && currentSpeed >= 0) {
        currentSpeed += acceleration * speed_control;
        myCamera.movecamera(myCamera.forward * (movespeed + currentSpeed));
    }
    else if (sKeyPressed && currentSpeed <= 0) {
        currentSpeed -= acceleration * speed_control;
        myCamera.movecamera(-myCamera.forward * (movespeed + currentSpeed));
    }
    else {
        // 속도가 감속 되도록 구현
        if (currentSpeed > 0.0f) {
            currentSpeed -= 1.8 * acceleration * speed_control;
            if (currentSpeed < 0.0f) {
                currentSpeed = 0.0f;
            }
            // 키를 때더라도 계속 앞으로 가게 설정
            myCamera.movecamera(myCamera.forward * (movespeed + currentSpeed));
        }
        else if (currentSpeed < 0.0f) {
            currentSpeed += 1.8 * acceleration * speed_control;
            if (currentSpeed > 0.0f) {
                currentSpeed = 0.0f;
            }
            myCamera.movecamera(-myCamera.forward * (movespeed + currentSpeed));
        }
    }
    sprintf(message, "Velocity : %f", currentSpeed * 10000);
}
void MyTimer(int Value) {

    // 공전 주기 

    Day = (Day + 0.8); //수
    Day1 = (Day1 + 0.7); //금
    Day2 = (Day2 + 0.6); //지
    Day3 = (Day3 + 0.5); //화
    Day4 = (Day4 + 0.4); //목
    Day5 = (Day5 + 0.3); //토
    Day6 = (Day6 + 0.2); //천
    Day7 = (Day7 + 0.1); //해

    if (Day > 360) Day -= 360;
    if (Day1 > 360) Day1 -= 360;
    if (Day2 > 360) Day2 -= 360;
    if (Day3 > 360) Day3 -= 360;
    if (Day4 > 360) Day4 -= 360;
    if (Day5 > 360) Day5 -= 360;
    if (Day6 > 360) Day6 -= 360;
    if (Day7 > 360) Day7 -= 360;

    Time = (Time + 1);
    if (Time > 360) Time -= 360;
    
    updateCameraPosition();
    glutPostRedisplay(); // MyDisplay를 다시 호출
    glutTimerFunc(40, MyTimer, 1); // 타이머 콜백을 다시 호출
}

int main(int argc, char** argv) {

    glutInit(&argc, argv);              //GLUT 라이브러리를 초기화시키고 윈도우 운영체제와 연결 하기 위한 함수
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);     //윈도우 컬러모드와 그래픽 카드 더블버퍼링 초기화
    glutCreateWindow("컴퓨터그래픽스");
    InitGL();
    glEnable(GL_DEPTH_TEST);

    // 전체화면
    glutFullScreen();
    screen_width = glutGet(GLUT_SCREEN_WIDTH);
    screen_height = glutGet(GLUT_SCREEN_HEIGHT);

    myCamera.InitCamera(vec3(0.3, 0.4, 2), vec3(0, 0, 0), vec3(0, 0, 1));

    glClearColor(0.0, 0.0, 0.0, 0.0);         //배경색 설정
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT), 0.1, 1000.0);

    // 텍스쳐 불러오기
    LoadPlanetTextures();
    LoadSkyboxTextures();
    LoadFireTextures();
    const char* pngFilename1 = "../Data/spaceship.png";  // 적절한 PNG 파일 경로로 바꿔주세요.
    const char* pngFilename2 = "../Data/left_line.png";  // 적절한 PNG 파일 경로로 바꿔주세요.
    const char* pngFilename3 = "../Data/right_line.png";  // 적절한 PNG 파일 경로로 바꿔주세요.
    const char* pngFilename4 = "../Data/line.png";
    const char* pngFilename5 = "../Data/Particle.png";
    spaceship_bottom = LoadPNGTexture(pngFilename1);
    spaceship_left = LoadPNGTexture(pngFilename2);
    spaceship_right = LoadPNGTexture(pngFilename3);
    spaceship_top = LoadPNGTexture(pngFilename4);

    // 콜백함수

    glutSpecialFunc(specialKeys);
    glutDisplayFunc(MyDisplay);         
    glutPassiveMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    InitLight();
    glutTimerFunc(40, MyTimer, 1);
    glutMainLoop();            //3d를 계속 화면에 뿌려주기위한 루프
    return 0;
}

