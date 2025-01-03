//Damjan Ilic RA-103/2021
//Tenk trener

#define _CRT_SECURE_NO_WARNINGS


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>


#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"



using namespace std;

//stb_image.h je header-only biblioteka za ucitavanje tekstura.
//Potrebno je definisati STB_IMAGE_IMPLEMENTATION prije njenog ukljucivanja
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <ft2build.h>
#include FT_FREETYPE_H
unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath); //Ucitavanje teksture, izdvojeno u funkciju

void drawBullet(unsigned int shaderProgram, unsigned int bullet);
void drawNeedle(float x1, float x2, float y1, float y2, int shaderProgram, float shaderValue, float angleDegrees);

void shootBullet(unsigned int shaderProgram);
void updateReadyIndicator(float currentTime);

double mapValue(double value, double minSource, double maxSource, double minTarget, double maxTarget) {
    // Proveri da li je vrednost unutar raspona izvora
    if (value < minSource || value > maxSource) {
        std::cerr << "Value out of source range!" << std::endl;
        return 0.0;
    }

    // Mapiraj vrednost na ciljani raspon
    return minTarget + (value - minSource) * (maxTarget - minTarget) / (maxSource - minSource);
}



struct Target {
    float x;
    float targetOffset = 0.0f;
    bool isAlive = true;
};

void shoot(float currentTime, std::vector<Target> &targets);

void drawtxt(unsigned int shader, float x1, float x2, float y1, float y2, unsigned texture) {
    glUseProgram(shader);
    float vertices[] =


        //x1, y1, 0.0f, // Gornji levi ugao
        //x2, y1, 0.0f, // Gornji desni ugao
        //x2, y2, 0.0f, // Donji desni ugao
        //x1, y2, 0.0f  // Donji levi ugao
    //(1.55f, 0.75f, 0.0, 0.9

    {   //X    Y      S    T 
    //    0.4f, 0.0,   0, 0.0,//prvo tjeme
    //    0.9, 0.0,  1.0, 0.0, //drugo tjeme
    //    0.9, 0.9,    1.0, 1, //trece tjeme
    //    0.4f, 0.9,  0.0, 1
        x1, y1,   0, 0.0,//prvo tjeme
        x2, y1,  1.0, 0.0, //drugo tjeme
        x2, y2,    1.0, 1, //trece tjeme
        x1,y2,  0.0, 1
    };

    // notacija koordinata za teksture je STPQ u GLSL-u (ali se cesto koristi UV za 2D teksture i STR za 3D)
    //ST koordinate u nizu tjemena su koordinate za teksturu i krecu se od 0 do 1, gdje je 0, 0 donji lijevi ugao teksture
    //Npr. drugi red u nizu tjemena ce da mapira boje donjeg lijevog ugla teksture na drugo tjeme
    unsigned int stride = (2 + 2) * sizeof(float);

    unsigned int VAOt;
    glGenVertexArrays(1, &VAOt);
    glBindVertexArray(VAOt);
    unsigned int VBOt;
    glGenBuffers(1, &VBOt);
    glBindBuffer(GL_ARRAY_BUFFER, VBOt);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Tekstura zavisnost od slike, ne valja ovako al nisam se snasao
    //unsigned checkerTexture = loadImageToTexture("res/voltmeter.png"); //Ucitavamo teksturu
    //unsigned checkerTexture = loadImageToTexture(filePath);
    unsigned checkerTexture = texture;
    //if (imgNum == 0) {
    //    loadImageToTexture("res/voltmeter.png");
    //}
    //else if (imgNum == 1) {
    //    loadImageToTexture("res/")
    //}
    glBindTexture(GL_TEXTURE_2D, checkerTexture); //Podesavamo teksturu
    glGenerateMipmap(GL_TEXTURE_2D); //Generisemo mipmape 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//S = U = X    GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);// T = V = Y
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   //GL_NEAREST, GL_LINEAR
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(shader);
    unsigned uTexLoc = glGetUniformLocation(shader, "uTex");
    glUniform1i(uTexLoc, 0); // Indeks teksturne jedinice (sa koje teksture ce se citati boje)
    glUseProgram(0);

    glUseProgram(shader);
    glBindVertexArray(VAOt);

    glActiveTexture(GL_TEXTURE0); //tekstura koja se bind-uje nakon ovoga ce se koristiti sa SAMPLER2D uniformom u sejderu koja odgovara njenom indeksu
    glBindTexture(GL_TEXTURE_2D, checkerTexture);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    //glDeleteTextures(1, &checkerTexture);
    glDeleteBuffers(1, &VBOt);
    glDeleteVertexArrays(1, &VAOt);
    //glDeleteProgram(shader);
}


struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};
std::map<char, Character> Characters;
void RenderText(unsigned int shader, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int VAOF, unsigned int VBOF)
{
    // activate corresponding render state
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAOF);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBOF);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}






                                                                                                                    //SETTINGS


bool shouldDrawBullet = true;
int ammo = 10;
bool canShoot = true;
const double cooldown = 7.5;
float lastShotTime = 0.0f;
float voltage = 75.0f;
float hydraulic = 0.0f;
bool toggleMode = true;
float offset = 0.0f; //za panoramu
float targetOffset = 0.0f;
int targetNum = 3;
int hit = 0;
float targetFPS = 60.0f;           // Ciljani FPS
float frameTime = 1.0f / targetFPS;
float missionDoneTime = 0.0f;

//Screen settings
const float SCR_WIDTH = 1920.0f;
const float SCR_HEIGHT = 1080.0f;

// Globalne varijable za poziciju kamere
float cameraPosX = 0.0f, cameraPosY = 1.0f, cameraPosZ = 3.0f;
float cameraSpeed = 0.1f; // Brzina pomeranja kamere
float yaw = -90.0f, pitch = 0.0f; // Za rotaciju kamere


// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


//Kamera
// camera
Camera camera(glm::vec3(cameraPosX, cameraPosY, cameraPosZ));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;



//////////////////targets

float randBetween(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

unsigned checkerTexture = loadImageToTexture("res/texel_checker.png"); //Ucitavamo teksturu


unsigned int rectVAO, rectVBO;

void initRectangle();


void drawRectangle(float x1, float x2, float y1, float y2, int shaderProgram, float shaderValue);
//void drawTexture(float x1, float x2, float y1, float y2, int shaderProgram, GLuint textureID, float shaderValue);





// Funkcije za kretanje kamere pomoću tastature i misa
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);


int main(void)
{




    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ INICIJALIZACIJA ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (!glfwInit()) {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    float aspectRatio = (float)mode->width / (float)mode->height;
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "[Tenk Trener]", glfwGetPrimaryMonitor(), NULL);

    if (window == NULL) {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);
    //fp kamera
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //sakriva sterlicu i strelica se nece zaustaviti na ivici ekrana = moze 360 okret

    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }
    //
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ Inicijalizacija Culling-a ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);        // Omogućava culling
    glCullFace(GL_BACK);           // Skida zadnje strane
    glFrontFace(GL_CCW);           // Postavlja CCW kao spoljne strane (counter-clockwise)



    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Tamna pozadina
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ TEKSTURE ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned bulletG = loadImageToTexture("res/tank_bullet.png");
    unsigned voltmeterG = loadImageToTexture("res/voltmeter.png");
    unsigned flamesG = loadImageToTexture("res/flames.png");
    unsigned panoramaG = loadImageToTexture("res/panorama.png");
    unsigned targetG = loadImageToTexture("res/target.png");
    unsigned zoomG = loadImageToTexture("res/zoom.png");
    unsigned deadG = loadImageToTexture("res/dead.png");
    unsigned tankCannonG = loadImageToTexture("res/tank_cannon.png");
    unsigned backgroundG = loadImageToTexture("res/background.png");
    unsigned semaphoreONG = loadImageToTexture("res/semaphoreON.png");
    unsigned semaphoreOFFG = loadImageToTexture("res/semaphoreOFF.png");
    unsigned aimG = loadImageToTexture("res/aim.png");
    unsigned dottedG = loadImageToTexture("res/dotted.png");
    unsigned xboxG = loadImageToTexture("res/box.png");
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ SEJDERI ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    unsigned int lightBulbShader = createShader("lightBulb.vert", "lightBulb.frag");
    unsigned int panoramaShader = createShader("panorama.vert", "panorama.frag");


    //3d
    unsigned int triDTest = createShader("kocka3dtest.vert", "kocka3dtest.frag");


    //float vertices3d[] = {
    //    // Koordinate           // Boje
    //    // Zadnja strana
    //    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,  -1.0f, -1.0f, -1.0f, // Crvena
    //     0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,  1.0f, -1.0f, -1.0f,// Zelena
    //     0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, -1.0f,// Plava
    //    -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f,  -1.0f, 1.0f, -1.0f, // Žuta

    //    // Prednja strana
    //    -0.5f, -0.5f,  0.5f,   0.5f, 0.0f, 0.5f,  -1.0f,- 1.0f, 1.0f,// Ljubičasta
    //     0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 1.0f,  1.0f,- 1.0f, 1.0f,// Cijan
    //     0.5f,  0.5f,  0.5f,   1.0f, 0.5f, 0.0f,  1.0f, 1.0f, 1.0f,// Narandžasta
    //    -0.5f,  0.5f,  0.5f,   0.5f, 1.0f, 0.5f,  -1.0f, 1.0f, 1.0f,// Svetlo zelena
    //};
    //float vertices3d[] = {
    //    // Koordinate        // Tekstura (u, v)     // Normale
    //    // Zadnja strana
    //    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,        -1.0f, -1.0f, -1.0f, // Donji levi ugao
    //     0.5f, -0.5f, -0.5f,   1.0f, 0.0f,         1.0f, -1.0f, -1.0f, // Donji desni ugao
    //     0.5f,  0.5f, -0.5f,   1.0f, 1.0f,         1.0f,  1.0f, -1.0f, // Gornji desni ugao
    //    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,        -1.0f,  1.0f, -1.0f, // Gornji levi ugao

    //    // Prednja strana
    //    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,        -1.0f, -1.0f,  1.0f,
    //     0.5f, -0.5f,  0.5f,   1.0f, 0.0f,         1.0f, -1.0f,  1.0f,
    //     0.5f,  0.5f,  0.5f,   1.0f, 1.0f,         1.0f,  1.0f,  1.0f,
    //    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,        -1.0f,  1.0f,  1.0f,
    //};
    
    float vertices3d[] = {
        // Koordinate (x, y, z)      // Tekstura (u, v)   // Normale (nx, ny, nz)

        // Zadnja strana (normala: -Z) - kvadrat podeljen na dva trougla
         // Donji levi ugao
        0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   -1.0f,  1.0f, -1.0f,
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f,   -1.0f, -1.0f, -1.0f, // Donji desni ugao
          // Gornji desni ugao
         -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -1.0f, -1.0f, -1.0f,
        //-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -1.0f, -1.0f, -1.0f, // Donji levi ugao
        // 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   -1.0f,  1.0f, -1.0f, // Gornji desni ugao
        //-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,   -1.0f,  1.0f, -1.0f, // Gornji levi ugao

        // Prednja strana (normala: +Z)
        //-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,    1.0f, -1.0f,  1.0f, // Donji levi ugao
        // 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,    1.0f, -1.0f,  1.0f, // Donji desni ugao
        // 0.5f,  0.5f,  0.5f,   1.0f, 1.0f,    1.0f,  1.0f,  1.0f, // Gornji desni ugao

        //-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,    1.0f, -1.0f,  1.0f, // Donji levi ugao
        // 0.5f,  0.5f,  0.5f,   1.0f, 1.0f,    1.0f,  1.0f,  1.0f, // Gornji desni ugao
        //-0.5f,  0.5f,  0.5f,   0.0f, 1.0f,    1.0f,  1.0f,  1.0f, // Gornji levi ugao

        //// Leva strana (normala: -X)
        //-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -1.0f, -1.0f, -1.0f, // Donji levi ugao
        //-0.5f, -0.5f,  0.5f,   1.0f, 0.0f,   -1.0f, -1.0f,  1.0f, // Donji desni ugao
        //-0.5f,  0.5f,  0.5f,   1.0f, 1.0f,   -1.0f,  1.0f,  1.0f, // Gornji desni ugao

        //-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -1.0f, -1.0f, -1.0f, // Donji levi ugao
        //-0.5f,  0.5f,  0.5f,   1.0f, 1.0f,   -1.0f,  1.0f,  1.0f, // Gornji desni ugao
        //-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,   -1.0f,  1.0f, -1.0f, // Gornji levi ugao

        // Desna strana (normala: +X)
         //0.5f, -0.5f, -0.5f,   0.0f, 0.0f,    1.0f, -1.0f, -1.0f, // Donji levi ugao
         //0.5f, -0.5f,  0.5f,   1.0f, 0.0f,    1.0f, -1.0f,  1.0f, // Donji desni ugao
         //0.5f,  0.5f,  0.5f,   1.0f, 1.0f,    1.0f,  1.0f,  1.0f, // Gornji desni ugao

         //0.5f, -0.5f, -0.5f,   0.0f, 0.0f,    1.0f, -1.0f, -1.0f, // Donji levi ugao
         //0.5f,  0.5f,  0.5f,   1.0f, 1.0f,    1.0f,  1.0f,  1.0f, // Gornji desni ugao
         //0.5f,  0.5f, -0.5f,   0.0f, 1.0f,    1.0f,  1.0f, -1.0f, // Gornji levi ugao

         //// Donja strana (normala: -Y)
         //-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -1.0f, -1.0f, -1.0f, // Donji levi ugao
         // 0.5f, -0.5f, -0.5f,   1.0f, 0.0f,    1.0f, -1.0f, -1.0f, // Donji desni ugao
         // 0.5f, -0.5f,  0.5f,   1.0f, 1.0f,    1.0f, -1.0f,  1.0f, // Gornji desni ugao

         //-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -1.0f, -1.0f, -1.0f, // Donji levi ugao
         // 0.5f, -0.5f,  0.5f,   1.0f, 1.0f,    1.0f, -1.0f,  1.0f, // Gornji desni ugao
         //-0.5f, -0.5f,  0.5f,   0.0f, 1.0f,   -1.0f, -1.0f,  1.0f, // Gornji levi ugao

         //// Gornja strana (normala: +Y)
         //-0.5f,  0.5f, -0.5f,   0.0f, 0.0f,   -1.0f,  1.0f, -1.0f, // Donji levi ugao
         // 0.5f,  0.5f, -0.5f,   1.0f, 0.0f,    1.0f,  1.0f, -1.0f, // Donji desni ugao
         // 0.5f,  0.5f,  0.5f,   1.0f, 1.0f,    1.0f,  1.0f,  1.0f, // Gornji desni ugao

         //-0.5f,  0.5f, -0.5f,   0.0f, 0.0f,   -1.0f,  1.0f, -1.0f, // Donji levi ugao
         // 0.5f,  0.5f,  0.5f,   1.0f, 1.0f,    1.0f,  1.0f,  1.0f, // Gornji desni ugao
         //-0.5f,  0.5f,  0.5f,   0.0f, 1.0f,   -1.0f,  1.0f,  1.0f  // Gornji levi ugao
    };

    unsigned int indices3d[] = {
        // Zadnja strana
        2, 1, 0,   // Prvi trougao
        0, 3, 2,   // Drugi trougao

        // Prednja strana
        4, 5, 6,   // Prvi trougao
        6, 7, 4,   // Drugi trougao

        // Leva strana
        7, 3, 0,   // Prvi trougao
        0, 4, 7,   // Drugi trougao

        // Desna strana
        6, 5, 1,   // Prvi trougao
        1, 2, 6,   // Drugi trougao

        // Donja strana
        0, 1, 5,   // Prvi trougao
        5, 4, 0,   // Drugi trougao

        // Gornja strana
        6, 2, 3,   // Prvi trougao
        3, 7, 6    // Drugi trougao
    };

    //normale 


    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  50.0f,  0.0f),
        glm::vec3(6.0f,  0.0f, 0.0f),
        glm::vec3(-7.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(12.4f, -0.4f, -3.5f),
        glm::vec3(-6.7f,  3.0f, -7.5f),
        glm::vec3(9.3f, -2.0f, -2.5f),
        glm::vec3(11.5f,  2.0f, -2.5f),
        glm::vec3(5.5f,  15.0f, -2.5f),
        glm::vec3(5.5f,  5.5f -1.0f, 5.5f)
    };

    //tlo
    float groundVertices[] = {
        // Pozicije           // Teksturne koordinate
        -50.0f, -5.0f, -50.0f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,// Levo dole
         50.0f, -5.0f, -50.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,// Desno dole
         50.0f, -5.0f,  50.0f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,// Desno gore
        -50.0f, -5.0f,  50.0f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,// Levo gore
    };

    unsigned int groundIndices[] = {
        2, 1, 0, // Prvi trougao
        3, 2, 0  // Drugi trougao
    };
    unsigned int groundVAO, groundVBO, groundEBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glGenBuffers(1, &groundEBO);

    glBindVertexArray(groundVAO);

    // Povezivanje vertex podataka
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

    // Postavljanje atributa
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Pozicije
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Teksture
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); // Teksture
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);


    /////////////////////3D TEST
    //unsigned int VAO3d, VBO3d, EBO3d;
    //glGenVertexArrays(1, &VAO3d);
    //glGenBuffers(1, &VBO3d);
    //glGenBuffers(1, &EBO3d);

    //glBindVertexArray(VAO3d);
    //glBindBuffer(GL_ARRAY_BUFFER, VBO3d);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices3d), vertices3d, GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3d);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices3d), indices3d, GL_STATIC_DRAW);

    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    //glEnableVertexAttribArray(2);

    //glBindVertexArray(0);
    unsigned int VAO3d, VBO3d;
    glGenVertexArrays(1, &VAO3d);
    glGenBuffers(1, &VBO3d);

    // Bind VAO
    glBindVertexArray(VAO3d);

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO3d);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices3d), vertices3d, GL_STATIC_DRAW);

    // Podesite atribute vrhova
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Pozicije
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Teksture
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); // Normale
    glEnableVertexAttribArray(2);

    // Unbind VAO
    glBindVertexArray(0);

    //model koji se krece sa kamerom
    //unsigned int vaotest, vbotest, ebotest;
    //glGenVertexArrays(1, &vaotest);
    //glGenBuffers(1, &vbotest);
    //glGenBuffers(1, &ebotest);
    //glBindVertexArray(vaotest);
    //glBindBuffer(GL_ARRAY_BUFFER, vbotest);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices3d), vertices3d, GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebotest);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices3d), indices3d, GL_STATIC_DRAW);

    //// Pretpostavljam da su tvoji podaci u vertices3d slični, pa samo treba da postaviš atribute:
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    //glEnableVertexAttribArray(2);

    //glBindVertexArray(0);





    //Svetlost
    glm::vec3 lightPos0(0.0f, 25.0f, 0.0f);
    glUseProgram(triDTest);

    glUniform3fv(glGetUniformLocation(triDTest, "lightPos0"), 1, glm::value_ptr(lightPos0));

    GLuint textureID = loadImageToTexture("res/box.png");
    if (textureID == 0) {
        std::cout << "Greska pri ucitavanju teksture!" << std::endl;
    }
    else {
        std::cout << "Tekstura ucitana uspesno!" << std::endl;
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER PETLJA ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glfwPollEvents();


        glUseProgram(triDTest);
        //cubePositions[9] = glm::vec3(cameraPosX, cameraPosY - 1.0f, cameraPosZ);
        //times
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // ----- tastatura input
        processInput(window);

        //// Koristimo shader za kocku
        
        //int modelLoc = glGetUniformLocation(triDTest, "model");
        //int viewLoc = glGetUniformLocation(triDTest, "view");

        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));  // Model matrica (identična za kocku)
        //glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));  // Postavljamo view matricu
        //glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));  // Postavljamo projekciju

        //glBindVertexArray(VAO3d);
        //glDrawElements(GL_TRIANGLES, sizeof(indices3d) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);




        int projLoc = glGetUniformLocation(triDTest, "projection");
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        //triDTest.setMat4("projection", projection);



        // camera/view transformation
        int viewLoc = glGetUniformLocation(triDTest, "view");
        glm::mat4 view = camera.GetViewMatrix();
        //ourShader.setMat4("view", view);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // render boxes
        //textura
        glBindVertexArray(VAO3d);
        glActiveTexture(GL_TEXTURE0);

        //glBindTexture(GL_TEXTURE_2D, xboxG);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // Ili GL_NEAREST
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(glGetUniformLocation(triDTest, "uTex"), 0);
        //cout << glGetUniformLocation(triDTest, "uTex") << endl;
        //vaoo



        int modelLoc = glGetUniformLocation(triDTest, "model");
        //glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        //model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f));
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
            //glDrawElements(GL_TRIANGLES, sizeof(indices3d) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
            //glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        //glDrawElements(GL_TRIANGLES, sizeof(indices3d) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(groundVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);


      /*  glBindVertexArray(vaotest);
        glDrawElements(GL_TRIANGLES, sizeof(indices3d) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);*/

        //drawBullet(unifiedShader, bulletG);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);

    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{

    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);


    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        cout << "Uspesno ucitana tekstura " << filePath << endl;
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}
void drawBullet(unsigned int shaderProgram, unsigned int bullet) {
    if (!shouldDrawBullet) {
        return;
    }
    unsigned int bulletCillinderVAO, bulletCillinderVBO;
    for (float i = 0; i < ammo; i++) {
        // drawRectangle(-1.55 + i / 5, -1.45 + i / 5, -0.8, -0.6, shaderProgram, 3.0f);
        drawtxt(shaderProgram, -1 + i / 9, -0.9 + i / 9, -0.8, -0.6, bullet);
    }
}

void shootBullet(unsigned int shaderProgram) {


    unsigned int bulletCillinderVAO, bulletCillinderVBO;
    for (float i = ammo / 1.0; i < 10; i++) {
        float bulletCillinderVertices[]{
        -1.55f + i / 5 , -0.6f, 0.0f, // Gornji levi ugao
         -1.45f + i / 5 , -0.6f, 0.0f, // Gornji desni ugao
         -1.45f + i / 5 , -0.8f, 0.0f, // Donji desni ugao
        -1.55f + i / 5 , -0.8f, 0.0f  // Donji levi ugao
        };




        //unsigned int rectVAO, rectVBO;


        glGenVertexArrays(1, &bulletCillinderVAO);
        glGenBuffers(1, &bulletCillinderVBO);

        glBindVertexArray(bulletCillinderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, bulletCillinderVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(bulletCillinderVertices), bulletCillinderVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(bulletCillinderVAO);
        glUniform1f(glGetUniformLocation(shaderProgram, "colorSwitch"), 0.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}

void updateReadyIndicator(float currentTime) {
    if (ammo > 0 && (currentTime - lastShotTime) >= cooldown) {
        canShoot = true;
    }
    else {
        canShoot = false;
    }
}

void shoot(float currentTime, std::vector<Target> &targets) {
    if (canShoot && ammo > 0) {
        ammo--;  // Smanji municiju
        lastShotTime = currentTime;
        std::cout << "Paljba!" << std::endl;

        //for (auto it = targets.begin(); it != targets.end(); ) {
        //    if (std::abs(it->x - it->targetOffset) < 0.15) {
        //        std::cout << "Pogodak" << std::endl;
        //        std::cout << "Pogodjen: " << std::to_string(it->x) << std::endl;

        //        
        //        it = targets.erase(it); // Erase i automatski pomera iterator
        //    }
        //    else {
        //        ++it;  // Ako nije pogodjena, predji na sledeci cilj
        //    }
        //}
        for (Target &t : targets) {
            //t.x + w/2 (0.1) = sredina mete
            if ((std::abs((t.x+0.1) - t.targetOffset) < 0.065) && t.isAlive) {
                t.isAlive = false;
                hit++;
            }
        }
    }
}
void initRectangle() {
    // Kreiraj koordinate pravougaonika (oko centra)
    float rectangleVertices[]{
        -0.5f,  0.5f, 0.0f, // Gornji levi ugao
         0.5f,  0.5f, 0.0f, // Gornji desni ugao
         0.5f, -0.5f, 0.0f, // Donji desni ugao
        -0.5f, -0.5f, 0.0f  // Donji levi ugao
    };

    // Generisemo VAO i VBO samo jednom
    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
     //glBindVertexArray(0);
}

void drawRectangle(float x1, float x2, float y1, float y2, int shaderProgram, float shaderValue) {
    initRectangle();
    glUseProgram(shaderProgram); // Koristi sejder program
    //initRectangle();
    // Prilagodjavamo koordinate pravougaonika
    float rectangleVertices[]{
        x1, y1, 0.0f, // Gornji levi ugao
         x2, y1, 0.0f, // Gornji desni ugao
         x2, y2, 0.0f, // Donji desni ugao
        x1, y2, 0.0f  // Donji levi ugao
    };

    // Binduj VAO i VBO
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rectangleVertices), rectangleVertices); // Azuriraj koordinate

    // Posaljite uniformnu promenljivu za boju
    glUniform1f(glGetUniformLocation(shaderProgram, "colorSwitch"), shaderValue);
    // Nacrtaj pravougaonik
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Oslobodi resurse
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // glDeleteBuffers(1, &rectVAO);
    // glDeleteVertexArrays(1, &rectVAO);
    // glDeleteProgram(shaderProgram);
}


void drawNeedle(float x1, float x2, float y1, float y2, int shaderProgram, float shaderValue, float angleDegrees) {
    glUseProgram(shaderProgram);

    // Pretvorite ugao iz stepeni u radijane
    float angleRadians = angleDegrees * (3.14f / 180.0f);

    float shakeAngle = (rand() % 5 - 2) * 0.0025f;

    if (voltage < 5) {
        shakeAngle = 0;
    }
    else if (voltage < 25) {
        shakeAngle = (rand() % 5 - 2) * 0.0025f;
    }
    else if (voltage < 60) {
        shakeAngle = (rand() % 5 - 2) * 0.0085f;
    }
    else if (voltage <= 90) {
        shakeAngle = (rand() % 5 - 2) * 0.015f;
    }

    angleRadians = angleRadians + shakeAngle;


    // Izracunajte sinus i kosinus za ugao
    float cosTheta = cos(angleRadians);
    float sinTheta = sin(angleRadians);

    // Pivot tacka (desna ivica)
    float pivotX = x2;
    float pivotY = (y1 + y2) / 2.0f;

    // Definisite vrhove u lokalnom prostoru (u odnosu na pivot)
    float localVertices[4][2] = {
        {x1 - pivotX, y1 - pivotY}, // Gornji levi
        {x2 - pivotX, y1 - pivotY}, // Gornji desni
        {x2 - pivotX, y2 - pivotY}, // Donji desni 
        {x1 - pivotX, y2 - pivotY}  // Donji levi
    };

    // Transformisani vrhovi
    float transformedVertices[4][3]; // Dodajemo Z koordinatu
    for (int i = 0; i < 4; i++) {
        transformedVertices[i][0] = localVertices[i][0] * cosTheta - localVertices[i][1] * sinTheta + pivotX;
        transformedVertices[i][1] = localVertices[i][0] * sinTheta + localVertices[i][1] * cosTheta + pivotY;
        transformedVertices[i][2] = 0.0f; // Z ostaje 0
    }

    // Binduj VAO i VBO
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(transformedVertices), transformedVertices); // Azuriraj koordinate

    // Posaljite uniformnu promenljivu za boju
    glUniform1f(glGetUniformLocation(shaderProgram, "colorSwitch"), shaderValue);

    // Nacrtaj pravougaonik
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Oslobodi resurse
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}



//3d 
//inputi 
void processInput(GLFWwindow* window)
{
    //ugasi na escape
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //cam related WASD movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}