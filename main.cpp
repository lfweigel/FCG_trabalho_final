#define GLUT_NO_LIB_PRAGMA
#define GLUT_NO_WARNING_DISABLE

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <time.h>

#include <gl/glut.h>
// openal (sound lib)
#include <al/alut.h>
// bitmap class to load bitmaps for textures
#include "bitmap.h"
// handle for the al.obj model
//#include "ModelAl.h"
// handle generic obj models
#include "3DObject.h"
// lib pra fazer as treta do heightmap
#include <SDL/SDL.h>

// sound stuff
#define NUM_BUFFERS 2
#define NUM_SOURCES 2
#define NUM_ENVIRONMENTS 1
// texture stuff
#define SMOOTH 0
#define SMOOTH_MATERIAL 1
#define SMOOTH_MATERIAL_TEXTURE 2

#define HEIGHT_CONSTANT 10

//tamanho em quads do terreno. significa que só vai usar (SIZE_X)x(Size_Z) pixels do bitmal
#define SIZE_X 300
#define SIZE_Z 300

//current position on the bitmap to calculate correct y-position of the player
int x_pixel, z_pixel;

// other stuff
#define PI 3.14159265

//object stuff
#define OBJECT_SPEED 0.05
#define COLLISION_THRESHOLD 1.0
#define MAX_OBJECTS 10


void mainInit();
void initSound();
void initTexture();
void initModels();
void initLight();
void enableFog();
void createGLUI();
void mainRender();
void mainCreateMenu();
void onMouseButton(int button, int state, int x, int y);
void onMouseMove(int x, int y);
void onKeyDown(unsigned char key, int x, int y);
void onKeyUp(unsigned char key, int x, int y);
void onGLUIEvent(int id);
void onWindowReshape(int x, int y);
void mainIdle();
int main(int argc, char **argv);
void setWindow();
void setViewport(GLint left, GLint right, GLint bottom, GLint top);
void updateState();
void renderFloor();
void updateCam();
void setTextureToOpengl();

/**
Screen dimensions
*/
int windowWidth = 1000;
int windowHeight = 800;

/**
Screen position
*/
int windowXPos = 500;
int windowYPos = 150;

int mainWindowId = 0;

double xOffset = -1.9;
double yOffset = -1.3;
int mouseLastX = 0;
int mouseLastY = 0;

float roty = 0.0f;
float rotx = 90.0f;

bool rightPressed = false;
bool leftPressed = false;
bool upPressed = false;
bool downPressed = false;

bool spacePressed = false;

float speedX = 0.0f;
float speedY = 0.0f;
float speedZ = 0.0f;

float posX = 0.0f;
float posY = 0.0f;
float posZ = 0.0f;

/*
variavel auxiliar pra dar variação na altura do ponto de vista ao andar.
*/
float headPosAux = 0.0f;

float maxSpeed = 0.25f;

float planeSize = 40.0f;

// more sound stuff (position, speed and orientation of the listener)
ALfloat listenerPos[]={0.0,0.0,4.0};
ALfloat listenerVel[]={0.0,0.0,0.0};
ALfloat listenerOri[]={0.0,0.0,1.0,
						0.0,1.0,0.0};

// now the position and speed of the sound source
ALfloat source0Pos[]={ -2.0, 0.0, 0.0};
ALfloat source0Vel[]={ 0.0, 0.0, 0.0};

ALfloat source1Pos[]={ -2.0, 0.0, 0.0};
ALfloat source1Vel[]={ 0.0, 0.0, 0.0};

// buffers for openal stuff
ALuint  buffer[NUM_BUFFERS];
ALuint  source[NUM_SOURCES];
ALuint  environment[NUM_ENVIRONMENTS];
ALsizei size,freq;
ALenum  format;
ALvoid  *data;



// parte de código extraído de "texture.c" por Michael Sweet (OpenGL SuperBible)
// texture buffers and stuff
int i;                       /* Looping var */
BITMAPINFO	*info;           /* Bitmap information */
GLubyte	    *bits;           /* Bitmap RGB pixels */
GLubyte     *ptr;            /* Pointer into bit buffer */
GLubyte	    *rgba;           /* RGBA pixel buffer */
GLubyte	    *rgbaptr;        /* Pointer into RGBA buffer */
GLubyte     temp;            /* Swapping variable */
GLenum      type;            /* Texture type */
GLuint      texture;         /* Texture object */



bool crouched = false;
bool running = false;
bool jumping = false;
float jumpSpeed = 0.06;
float gravity = 0.004;
float heightLimit;
float posYOffset = 0.2;
float backgrundColor[4] = {0.0f,206.0f,209.0f,1.0f};
C3DObject cObj;

int objects_destroyed = 0;

//CModelAl modelAL;

//matriz de heights extraidos do heightmap
std::vector<std::vector<float> > heights;


///////////////////////refatoracao pls

class Object{
public:
    float x_pos, z_pos;
    C3DObject cObj;
    bool alive;

    void initModel(const char* model) {
    this->alive = 1;
	cObj.Init();
	cObj.Load(model);
    }

    void randomlyPlaceOnWorld(int seed) {

        srand(time(NULL) + seed);
        this->x_pos = (0 - planeSize/2) + (((double)rand() / (double)RAND_MAX)*planeSize);
        this->z_pos = (0 - planeSize/2) + (((double)rand() / (double)RAND_MAX)*planeSize);
        printf("Object generated at %f %f \n", x_pos, z_pos);
        }

    void move_and_draw(int seed){

        srand(time(NULL) + seed);
        int movement = rand()%5;
        switch (movement){
            case 0: this->x_pos -= OBJECT_SPEED;
            break;
            case 1: this->x_pos += OBJECT_SPEED;
            break;
            case 2: this->z_pos -= OBJECT_SPEED;
            break;
            case 3: this ->z_pos += OBJECT_SPEED;
            break;
            case 4:
            default: break;
        }

        if (this->x_pos < -planeSize/2.0f)
            this->x_pos = -planeSize/2.0f;
        else if (this->x_pos > planeSize/2.0f)
            this->x_pos = planeSize/2.0f;

        if (this->z_pos < -planeSize/2.0f)
            this->z_pos = -planeSize/2.0f;
        else if (this->z_pos > planeSize/2.0f)
            this->z_pos = planeSize/2.0f;

        glPushMatrix();
        float x_pixel_float = (SIZE_X*(((planeSize/2)+x_pos)/planeSize));
        float z_pixel_float = (SIZE_Z*(((planeSize/2)+z_pos)/planeSize));
        x_pixel = (int) x_pixel_float;
        z_pixel = (int) z_pixel_float;
        glTranslatef(x_pos, (heights[x_pixel][z_pixel]*HEIGHT_CONSTANT)+1.0, z_pos);
        cObj.Draw(SMOOTH_MATERIAL_TEXTURE); // use SMOOTH for obj files, SMOOTH_MATERIAL for obj+mtl files and SMOOTH_MATERIAL_TEXTURE for obj+mtl+tga files
        glPopMatrix();
    }

    void checkColisionWithPlayer(){
        if (std::abs((this->x_pos - posX)) < COLLISION_THRESHOLD)
            if (std::abs((this->z_pos - posZ)) < COLLISION_THRESHOLD) {
                this->alive = 0;
                printf("ball died\n");
                objects_destroyed++;
                alSourcePlay(source[1]);
                Sleep(30);
                alSourceStop(source[1]);
            }

    }

    void destroy() {
        delete this;
    }

};

//Object ball_1, ball_2, ball_3, ball_4, ball_5, ball_6, ball_7, ball_8, ball_9, ball_10;
std::vector<Object> objects(MAX_OBJECTS);

void loadHeightmap(const char* bitmap_file) {

    SDL_Surface* loaded_bitmap = SDL_LoadBMP(bitmap_file);

    if (!loaded_bitmap) {
        printf("Failed to load %s", bitmap_file);
        return;
    }
    std::vector<float> current_row;
    for (int i=0; i<loaded_bitmap->h; i++){
        current_row.clear();
        for (int j=0; j<loaded_bitmap->w; j++){
            Uint32 pixel = ((Uint32*)loaded_bitmap->pixels)[i*loaded_bitmap->pitch/4 + j];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, loaded_bitmap->format, &r, &g, &b);
            current_row.push_back((float)((r+g+b)/3)/255.0);
        }
        heights.push_back(current_row);
    }
}
/*
void renderHeightmap(float size, float height){

    glShadeModel(GL_SMOOTH);
	glEnable(type);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glPushMatrix();

    glTranslatef(-(float)planeSize/2.0f, 0.0f, -(float)planeSize/2.0f);

    for (int i=0; i<heights.size()-1;i++) {
        for (int j=0; j<heights[i].size()-1; j++) {
            glBegin(GL_TRIANGLE_STRIP);
            glColor3f(heights[i][j],heights[i][j],heights[i][j]);
            //glBegin(GL_QUADS);
            glVertex3f(i*size, heights[i][j]*height, j*size);
            glVertex3f((i+1)*size, heights[i+1][j]*height, j*size);
            glVertex3f(i*size, heights[i][j+1]*height, (j+1)*size);
            glVertex3f((i+1)*size, heights[i+1][j+1]*height, (j+1)*size);
            glEnd();
        }
    }
    glDisable(type);
	glPopMatrix();
}
*/
void setWindow() {

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)windowWidth/(GLfloat)windowHeight,0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(posX,posY + posYOffset + 0.025 * std::abs(sin(headPosAux*PI/180)),posZ,
		posX + sin(roty*PI/180),posY + posYOffset + 0.025 * std::abs(sin(headPosAux*PI/180)) + cos(rotx*PI/180),posZ -cos(roty*PI/180),
		0.0,1.0,0.0);
}

/**
Atualiza a posição e orientação da camera
*/
void updateCam() {
    float x_pixel_float = (SIZE_X*(((planeSize/2)+posX)/planeSize));
    float z_pixel_float = (SIZE_Z*(((planeSize/2)+posZ)/planeSize));
    x_pixel = (int) x_pixel_float;
    z_pixel = (int) z_pixel_float;

    /*float prev_pixel_influence_x, cur_pixel_influence_x, next_pixel_influence_x, prev_pixel_influence_z, cur_pixel_influence_z, next_pixel_influence_z;
    if ((x_pixel_float - x_pixel) >= 0.5) {
        prev_pixel_influence_x = 0;
        next_pixel_influence_x = x_pixel_float - x_pixel;
        cur_pixel_influence_x = 1 - next_pixel_influence_x;
    } else {
        next_pixel_influence_x = 0;
        cur_pixel_influence_x = x_pixel_float - x_pixel;
        prev_pixel_influence_x = 1 - cur_pixel_influence_x;
    }

    if ((z_pixel_float - z_pixel) >= 0.5) {
        prev_pixel_influence_z = 0;
        next_pixel_influence_z = z_pixel_float - z_pixel;
        cur_pixel_influence_z = 1 - next_pixel_influence_z;
    } else {
        next_pixel_influence_z = 0;
        cur_pixel_influence_z = z_pixel_float - z_pixel;
        prev_pixel_influence_z = 1 - cur_pixel_influence_z;
    }
/*
    heightLimit = (heights[x_pixel-1][z_pixel]*prev_pixel_influence_x*HEIGHT_CONSTANT + heights[x_pixel][z_pixel]* cur_pixel_influence_x*HEIGHT_CONSTANT + heights[x_pixel+1][z_pixel]*next_pixel_influence_x*HEIGHT_CONSTANT
                    + heights[x_pixel][z_pixel-1]*prev_pixel_influence_z*HEIGHT_CONSTANT + heights[x_pixel][z_pixel]*cur_pixel_influence_z*HEIGHT_CONSTANT + heights[x_pixel][z_pixel+1]*next_pixel_influence_z*HEIGHT_CONSTANT)/2;
*/
    heightLimit = heights[x_pixel][z_pixel]*HEIGHT_CONSTANT;
   // printf("%d %d - %f %f--- %f %f %f --- %f %f %f -- %f \n", x_pixel, z_pixel, x_pixel_float, z_pixel_float, prev_pixel_influence_x, cur_pixel_influence_x,next_pixel_influence_x, prev_pixel_influence_z, cur_pixel_influence_z, next_pixel_influence_z, heightLimit);

	gluLookAt(posX,posY + posYOffset + 0.025 * std::abs(sin(headPosAux*PI/180)),posZ,
		posX + sin(roty*PI/180),posY + posYOffset + 0.025 * std::abs(sin(headPosAux*PI/180)) + cos(rotx*PI/180),posZ -cos(roty*PI/180),
		0.0,1.0,0.0);

	// atualiza a posição do listener e da origen do som, são as mesmas da camera, já que os passos vem de onde o personagem está
	listenerPos[0] = posX;
	listenerPos[1] = posY;
	listenerPos[2] = posZ;
	source0Pos[0] = posX;
	source0Pos[1] = posY;
	source0Pos[2] = posZ;

    GLfloat light_position1[] = {posX, posY, posZ, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position1);


}

void initLight() {
    glEnable(GL_LIGHTING );
	glEnable( GL_LIGHT0 );

	GLfloat light_ambient[] = { backgrundColor[0], backgrundColor[1], backgrundColor[2], backgrundColor[3] };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position1[] = {0.0, 0.0, 0.0, 1.0 };
	//GLfloat light_position1[] = {0.0, 10.0, 0.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position1);

}

void setViewport(GLint left, GLint right, GLint bottom, GLint top) {
	glViewport(left, bottom, right - left, top - bottom);
}


/**
Initialize
*/
void mainInit() {
	glClearColor(1.0,1.0,1.0,0.0);
	glColor3f(0.0f,0.0f,0.0f);
	setWindow();
	setViewport(0, windowWidth, 0, windowHeight);

	// habilita remocao de faces ocultas

	glFrontFace (GL_CCW);

	glEnable(GL_CULL_FACE);

	// habilita o z-buffer
	glEnable(GL_DEPTH_TEST);

    initSound();

    initTexture();

	initModels();

	initLight();

    loadHeightmap("heightmap.bmp");
}

void initModels() {
/*
	ball_1.initModel("ball.obj");
	ball_1.randomlyPlaceOnWorld(1);

    ball_2.initModel("ball.obj");
	ball_2.randomlyPlaceOnWorld(1000);

    ball_3.initModel("ball.obj");
	ball_3.randomlyPlaceOnWorld(2000);

    ball_4.initModel("ball.obj");
	ball_4.randomlyPlaceOnWorld(3000);

    ball_5.initModel("ball.obj");
	ball_5.randomlyPlaceOnWorld(4000);

    ball_6.initModel("ball.obj");
	ball_6.randomlyPlaceOnWorld(5000);

    ball_7.initModel("ball.obj");
	ball_7.randomlyPlaceOnWorld(6000);

    ball_8.initModel("ball.obj");
	ball_8.randomlyPlaceOnWorld(7000);

    ball_9.initModel("ball.obj");
	ball_9.randomlyPlaceOnWorld(8000);

    ball_10.initModel("ball.obj");
	ball_10.randomlyPlaceOnWorld(9000);
	*/
	for(int i=0; i < MAX_OBJECTS; i++){
        objects[i].initModel("ball.obj");
        objects[i].randomlyPlaceOnWorld(1000*i);
	}
}

/**
Initialize openal and check for errors
*/
void initSound() {

	printf("Initializing OpenAl \n");

	// Init openAL
	alutInit(0, NULL);

	alGetError(); // clear any error messages

    // Generate buffers, or else no sound will happen!
    alGenBuffers(NUM_BUFFERS, buffer);

    if(alGetError() != AL_NO_ERROR)
    {
        printf("- Error creating buffers !!\n");
        exit(1);
    }
    else
    {
        printf("init() - No errors yet.\n");
    }

	alutLoadWAVFile("footsteps_grass.wav",&format,&data,&size,&freq,false);
    alBufferData(buffer[0],format,data,size,freq);
    alutUnloadWAV(format, data, size, freq);

    alutLoadWAVFile("beep.wav",&format,&data,&size,&freq,false);
    alBufferData(buffer[1],format,data,size,freq);

	alGetError(); /* clear error */
    alGenSources(NUM_SOURCES, source);

    if(alGetError() != AL_NO_ERROR)
    {
        printf("- Error creating sources !!\n");
        exit(2);
    }
    else
    {
        printf("init - no errors after alGenSources\n");
    }

	listenerPos[0] = posX;
	listenerPos[1] = posY;
	listenerPos[2] = posZ;

	source0Pos[0] = posX;
	source0Pos[1] = posY;
	source0Pos[2] = posZ;

    source1Pos[0] = posX;
	source1Pos[1] = posY;
	source1Pos[2] = posZ;

	alListenerfv(AL_POSITION,listenerPos);
    alListenerfv(AL_VELOCITY,listenerVel);
    alListenerfv(AL_ORIENTATION,listenerOri);

	alSourcef(source[0], AL_PITCH, 1.0f);
    alSourcef(source[0], AL_GAIN, 1.0f);
    alSourcefv(source[0], AL_POSITION, source0Pos);
    alSourcefv(source[0], AL_VELOCITY, source0Vel);
    alSourcei(source[0], AL_BUFFER,buffer[0]);
    alSourcei(source[0], AL_LOOPING, AL_TRUE);

    alSourcef(source[1], AL_PITCH, 1.0f);
    alSourcef(source[1], AL_GAIN, 1.0f);
    alSourcefv(source[1], AL_POSITION, source0Pos);
    alSourcefv(source[1], AL_VELOCITY, source0Vel);
    alSourcei(source[1], AL_BUFFER,buffer[1]);
    alSourcei(source[1], AL_LOOPING, AL_TRUE);

	printf("Sound ok! \n\n");
}

/**
Initialize the texture using the library bitmap
*/
void initTexture(void)
{
    printf ("\nLoading texture..\n");
    // Load a texture object (256x256 true color)
    bits = LoadDIBitmap("grass.bmp", &info);
    if (bits == (GLubyte *)0) {
		printf ("Error loading texture!\n\n");
		return;
	}
    // Figure out the type of texture
    if (info->bmiHeader.biHeight == 1)
      type = GL_TEXTURE_1D;
    else
      type = GL_TEXTURE_2D;

    // Create and bind a texture object
    glGenTextures(1, &texture);
	glBindTexture(type, texture);

    // Create an RGBA image
    rgba = (GLubyte *)malloc(info->bmiHeader.biWidth * info->bmiHeader.biHeight * 4);

    i = info->bmiHeader.biWidth * info->bmiHeader.biHeight;
    for( rgbaptr = rgba, ptr = bits;  i > 0; i--, rgbaptr += 4, ptr += 3)
    {
            rgbaptr[0] = ptr[2];     // windows BMP = BGR
            rgbaptr[1] = ptr[1];
            rgbaptr[2] = ptr[0];
            rgbaptr[3] = (ptr[0] + ptr[1] + ptr[2]) / 3;
    }
    /*
	// Set texture parameters
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(type, 0, 4, info->bmiHeader.biWidth, info->bmiHeader.biHeight,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, rgba );
    */

    printf("Textura %d\n", texture);
	printf("Textures ok.\n\n", texture);

}

/**
Recovers the texture already initialized in initTexture(), setting it to opengl
*/
void setTextureToOpengl(void)
{
    // Create and bind a texture object
    glGenTextures(1, &texture);
	glBindTexture(type, texture);

	// Set texture parameters
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(type, 0, 4, info->bmiHeader.biWidth, info->bmiHeader.biHeight,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, rgba );
}

void renderFloor() {
	// set things up to render the floor with the texture
	glShadeModel(GL_SMOOTH);
	glEnable(type);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glPushMatrix();

    glTranslatef(-(float)planeSize/2.0f, 0.0f, -(float)planeSize/2.0f);

	float textureScaleX = 10.0;
	float textureScaleY = 10.0;
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    int xQuads = SIZE_X;
    int zQuads = SIZE_Z;
    for (int i = 0; i < xQuads; i++) {
        for (int j = 0; j < zQuads; j++) {
              //glBegin(GL_TRIANGLE_STRIP);
              glBegin(GL_QUADS);
              glColor3f(heights[i][j]*HEIGHT_CONSTANT,heights[i][j]*HEIGHT_CONSTANT,heights[i][j]*HEIGHT_CONSTANT);
              /*
                glColor3f(heights[i][j],heights[i][j],heights[i][j]);
                glTexCoord2f(1.0f, 0.0f);   // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f(i*(float)planeSize/xQuads, heights[i][j], j*(float)planeSize/zQuads);

                glTexCoord2f(0.0f, 0.0f);  // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f((i+1)*(float)planeSize/xQuads, heights[i+1][j], j*(float)planeSize/zQuads);

                glTexCoord2f(0.0f, 1.0f);  // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f(i* (float)planeSize/xQuads, heights[i][j+1], (j+1) * (float)planeSize/zQuads);

                glTexCoord2f(1.0f, 1.0f);  // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f((i+1) * (float)planeSize/xQuads, heights[i+1][j+1], (j+1) * (float)planeSize/zQuads);
                */
                glTexCoord2f(1.0f, 1.0f);  // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f(i * (float)planeSize/xQuads, heights[i][j]*HEIGHT_CONSTANT, j * (float)planeSize/zQuads);

                glTexCoord2f(1.0f, 0.0f);   // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f(i * (float)planeSize/xQuads, heights[i][j+1]*HEIGHT_CONSTANT, (j+1) * (float)planeSize/zQuads);

                glTexCoord2f(0.0f, 0.0f);  // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f((i+1) * (float)planeSize/xQuads, heights[i+1][j+1]*HEIGHT_CONSTANT, (j+1) * (float)planeSize/zQuads);

                glTexCoord2f(0.0f, 1.0f);  // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f((i+1) * (float)planeSize/xQuads, heights[i+1][j]*HEIGHT_CONSTANT, j * (float)planeSize/zQuads);



            glEnd();
        }
    }

	glDisable(type);


	glPopMatrix();
}

void renderScene() {
	glClearColor(backgrundColor[0],backgrundColor[1],backgrundColor[2],backgrundColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // limpar o depth buffer

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	updateCam();
/*
    glPushMatrix();
    //glTranslatef(0.0,10.0,0.0);
    glTranslatef(posX+1, heights[x_pixel][z_pixel]*HEIGHT_CONSTANT, posZ+1);
	cObj.Draw(SMOOTH_MATERIAL_TEXTURE); // use SMOOTH for obj files, SMOOTH_MATERIAL for obj+mtl files and SMOOTH_MATERIAL_TEXTURE for obj+mtl+tga files
	glPopMatrix();
*/
/*
    if(ball_1.alive) {
    ball_1.move_and_draw(1);
    ball_1.checkColisionWithPlayer();
    }

    if(ball_2.alive) {
    ball_2.move_and_draw(1000);
    ball_2.checkColisionWithPlayer();
    }

    if(ball_3.alive) {
    ball_3.move_and_draw(2000);
    ball_3.checkColisionWithPlayer();
    }

    if(ball_4.alive) {
    ball_4.move_and_draw(3000);
    ball_4.checkColisionWithPlayer();
    }

    if(ball_5.alive) {
    ball_5.move_and_draw(4000);
    ball_5.checkColisionWithPlayer();
    }

    if(ball_6.alive) {
    ball_6.move_and_draw(5000);
    ball_6.checkColisionWithPlayer();
    }

    if(ball_7.alive) {
    ball_7.move_and_draw(6000);
    ball_7.checkColisionWithPlayer();
    }

    if(ball_8.alive){
    ball_8.move_and_draw(7000);
    ball_8.checkColisionWithPlayer();
    }

    if(ball_9.alive){
    ball_9.move_and_draw(8000);
    ball_9.checkColisionWithPlayer();
    }

    if(ball_10.alive){
    ball_10.move_and_draw(9000);
    ball_10.checkColisionWithPlayer();
    }
*/
    for (int i=0; i<MAX_OBJECTS; i++) {
        if(objects[i].alive) {
            objects[i].move_and_draw(1000*i);
            objects[i].checkColisionWithPlayer();
        }
    }

    // sets the bmp file already loaded to the OpenGL parameters
    setTextureToOpengl();

	renderFloor();
	//renderHeightmap(0.1,0.4);

	//modelAL.Translate(0.0f,1.0f,0.0f);
	//modelAL.Draw();
}

void updateState() {

	if (upPressed || downPressed) {

		if (running) {
			speedX = 0.05 * sin(roty*PI/180) * 2;
			speedZ = -0.05 * cos(roty*PI/180) * 2;
		} else {
			speedX = 0.05 * sin(roty*PI/180);
			speedZ = -0.05 * cos(roty*PI/180);
		}
/*
		// efeito de "sobe e desce" ao andar
		headPosAux += 8.5f;
		if (headPosAux > 180.0f) {
			headPosAux = 0.0f;

		}*/

        if (upPressed) {
            posX += speedX;
            posZ += speedZ;
        } else {
            posX -= speedX;
            posZ -= speedZ;
        }

	} else {
		// parou de andar, para com o efeito de "sobe e desce"
		headPosAux = fmod(headPosAux, 90) - 1 * headPosAux / 90;
		headPosAux -= 4.0f;
		if (headPosAux < 0.0f) {
			headPosAux = 0.0f;
		}
	}

	posY += speedY;
	if (posY < heightLimit) {
		posY = heightLimit;
		speedY = 0.0f;
		jumping = false;
	} else {
		speedY -= gravity;
	}

	if (crouched) {
		posYOffset -= 0.01;
		if (posYOffset < 0.1) {
			posYOffset = 0.1;
		}
	} else {
		posYOffset += 0.01;
		if (posYOffset > 0.2) {
			posYOffset = 0.2;
		}
	}

	if(posX < - planeSize/2.0f)
        posX = -planeSize/2.0f;
    else if (posX > planeSize/2.0f)
        posX = planeSize/2.0f;

    if(posZ < - planeSize/2.0f)
        posZ = -planeSize/2.0f;
    else if (posZ > planeSize/2.0f)
        posZ = planeSize/2.0f;
}

/**
Render scene
*/
void mainRender() {
	updateState();
	renderScene();
	glFlush();
	glutPostRedisplay();
	Sleep(30);
}

/**
Handles events from the mouse right button menu
*/
void mainHandleMouseRightButtonMenuEvent(int option) {
	switch (option) {
		case 1 :
			exit(0);
			break;
		default:
			break;
	}
}

/**
Create mouse button menu
*/
void mainCreateMenu() {
	glutCreateMenu(mainHandleMouseRightButtonMenuEvent);
	glutAddMenuEntry("Quit", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/**
Mouse button event handler
*/
void onMouseButton(int button, int state, int x, int y) {
	//printf("onMouseButton button: %d \n", button);
	glutPostRedisplay();
}

/**
Mouse move while button pressed event handler
*/
void onMouseMove(int x, int y) {

	/*mouseLastX = x;
	mouseLastY = y;*/

	glutPostRedisplay();
}

/**
Mouse move with no button pressed event handler
*/
void onMousePassiveMove(int x, int y) {

	roty += (x - mouseLastX);

	rotx -= (y - mouseLastY);

	if (rotx < -128.0) {
		rotx = -128.0;
	}

	if (rotx > -45.0) {
		rotx = -45.0;
	}

	mouseLastX = x;
	mouseLastY = y;

	//glutPostRedisplay();
}

/**
Key press event handler
*/
void onKeyDown(unsigned char key, int x, int y) {
	//printf("%d \n", key);
	switch (key) {
		case 32: //space
			if (!spacePressed && !jumping) {
				jumping = true;
				speedY = jumpSpeed;
			}
			spacePressed = true;
			break;
		case 119: //w
			if (!upPressed) {
				alSourcePlay(source[0]);
			}
			upPressed = true;
			break;
		case 115: //s
		    if (!downPressed){
                alSourcePlay(source[0]);
		    }
			downPressed = true;
			break;
		case 97: //a
			leftPressed = true;
			break;
		case 100: //d
			rightPressed = true;
			break;
		case 99: //c
			crouched = true;
			break;
		case 114: //r
			running = true;
			break;
		default:
			break;
	}

	//glutPostRedisplay();
}

/**
Key release event handler
*/
void onKeyUp(unsigned char key, int x, int y) {
	switch (key) {
		case 32: //space
			spacePressed = false;
			break;
		case 119: //w
			if (upPressed) {
				alSourceStop(source[0]);
			}
			upPressed = false;
			break;
		case 115: //s
		    if (downPressed) {
                alSourceStop(source[0]);
		    }
			downPressed = false;
			break;
		case 97: //a
			leftPressed = false;
			break;
		case 100: //d
			rightPressed = false;
			break;
		case 99: //c
			crouched = false;
			break;
		case 114: //r
			running = false;
			break;
		case 27:
			exit(0);
			break;
		default:
			break;
	}

	//glutPostRedisplay();
}

void onWindowReshape(int x, int y) {
	windowWidth = x;
	windowHeight = y;
	setWindow();
	setViewport(0, windowWidth, 0, windowHeight);
}

/**
Glut idle funtion
*/
void mainIdle() {
	/**
	Set the active window before send an glutPostRedisplay call
	so it wont be accidently sent to the glui window
	*/

	glutSetWindow(mainWindowId);
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(windowWidth,windowHeight);
	glutInitWindowPosition(windowXPos,windowYPos);

	/**
	Store main window id so that glui can send it redisplay events
	*/
	mainWindowId = glutCreateWindow("Trabalho Final FCG");

	glutDisplayFunc(mainRender);

	glutReshapeFunc(onWindowReshape);

	/**
	Register mouse events handlers
	*/
	glutMouseFunc(onMouseButton);
	glutMotionFunc(onMouseMove);
	glutPassiveMotionFunc(onMousePassiveMove);

	/**
	Register keyboard events handlers
	*/
	glutKeyboardFunc(onKeyDown);
	glutKeyboardUpFunc(onKeyUp);

	mainInit();

	glutMainLoop();

    return 0;
}
