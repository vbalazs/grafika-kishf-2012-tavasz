//=============================================================================================
// Szamitogepes grafika hazi feladat keret. Ervenyes 2012-tol.          
// A //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// sorokon beluli reszben celszeru garazdalkodni, mert a tobbit ugyis toroljuk. 
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat. 
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni (printf is fajlmuvelet!)
// - new operatort hivni az onInitialization f�ggv�nyt kiv�ve, a lefoglalt adat korrekt felszabad�t�sa n�lk�l 
// - felesleges programsorokat a beadott programban hagyni
// - tovabbi kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan gl/glu/glut fuggvenyek hasznalhatok, amelyek
// 1. Az oran a feladatkiadasig elhangzottak ES (logikai AND muvelet)
// 2. Az alabbi listaban szerepelnek:  
// Rendering pass: glBegin, glVertex[2|3]f, glColor3f, glNormal3f, glTexCoord2f, glEnd, glDrawPixels
// Transzformaciok: glViewport, glMatrixMode, glLoadIdentity, glMultMatrixf, gluOrtho2D, 
// glTranslatef, glRotatef, glScalef, gluLookAt, gluPerspective, glPushMatrix, glPopMatrix,
// Illuminacio: glMaterialfv, glMaterialfv, glMaterialf, glLightfv
// Texturazas: glGenTextures, glBindTexture, glTexParameteri, glTexImage2D, glTexEnvi, 
// Pipeline vezerles: glShadeModel, glEnable/Disable a kovetkezokre:
// GL_LIGHTING, GL_NORMALIZE, GL_DEPTH_TEST, GL_CULL_FACE, GL_TEXTURE_2D, GL_BLEND, GL_LIGHT[0..7]
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Varga Balazs
// Neptun : IFAW8V
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy 
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem. 
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a 
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb 
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem, 
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.  
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat 
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================

#include <math.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
// MsWindows-on ez is kell
#include <windows.h>     
#endif // Win32 platform

#include <GL/gl.h>
#include <GL/glu.h>
// A GLUT-ot le kell tolteni: http://www.opengl.org/resources/libraries/glut/
#include <GL/glut.h>     


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Innentol modosithatod...

//--------------------------------------------------------
// 3D Vektor
//--------------------------------------------------------

struct Vector {
    float x, y, z;

    Vector() {
        x = y = z = 0;
    }

    Vector(float x0, float y0, float z0 = 0) {
        x = x0;
        y = y0;
        z = z0;
    }

    Vector operator*(float a) {
        return Vector(x * a, y * a, z * a);
    }

    Vector operator+(const Vector & v) {
        return Vector(x + v.x, y + v.y, z + v.z);
    }

    Vector operator-(const Vector & v) {
        return Vector(x - v.x, y - v.y, z - v.z);
    }

    float operator*(const Vector & v) { // dot product
        return (x * v.x + y * v.y + z * v.z);
    }

    Vector operator%(const Vector & v) { // cross product
        return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }

    float Length() {
        return sqrt(x * x + y * y + z * z);
    }
};

//--------------------------------------------------------
// Spektrum illetve szin
//--------------------------------------------------------

struct Color {
    float r, g, b;

    Color() {
        r = g = b = 0;
    }

    Color(float r0, float g0, float b0) {
        r = r0;
        g = g0;
        b = b0;
    }

    Color operator*(float a) {
        return Color(r * a, g * a, b * a);
    }

    Color operator*(const Color & c) {
        return Color(r * c.r, g * c.g, b * c.b);
    }

    Color operator+(const Color & c) {
        return Color(r + c.r, g + c.g, b + c.b);
    }
};

const int screenWidth = 600;
const int screenHeight = 600;

Color image[screenWidth*screenHeight];

const double VARIABLE_PIXEL_RATE = 100.0;
const int SECTION_PER_TRACK = 20;
const int LINES_SIZE = 400;
Vector linesCoords[LINES_SIZE]; //10 palya * 20 szakasz * 2 koord
Color lineColors[LINES_SIZE / 2];

int tracks = 0; // sipalyak szama
const int MAX_SKIERS = 10;
const double SKIERS_SIZE = 0.03;
Vector skiersCoords[MAX_SKIERS * 3];
int skiers = 0;
int startedSkiers = 0;

long time_ = 0;
bool working = false;

const bool fequals(float f1, float f2) {
    if (fabs(f1 - f2) < 0.001) return true;
    return false;
}

const Vector convertPixelsToVariable(const Vector pixel) {
    return Vector(pixel.x / VARIABLE_PIXEL_RATE, pixel.y / VARIABLE_PIXEL_RATE);
}

const Vector convertPixelsToGl(const Vector pixel) {
    const double x = (pixel.x - screenWidth / 2.0) / ((double) screenWidth / 2.0);
    const double y = (pixel.y - screenHeight / 2.0) / ((double) screenHeight / 2.0);
    return Vector(x, y * -1.0);
}

const Vector convertVariablesToGl(const Vector variable) {
    const Vector pixel(variable.x * VARIABLE_PIXEL_RATE, variable.y * VARIABLE_PIXEL_RATE);

    return convertPixelsToGl(pixel);
}

const double get100mInVar() {
    return screenWidth / VARIABLE_PIXEL_RATE / 100;
}

const double calculateHeightValue(const Vector varVector) {
    return sin(2 * varVector.x) + cos(3 * varVector.y) + (varVector.x * varVector.y) / 8.0;
}

const double calculateHeightValueFromPixel(const Vector pixel) {
    const Vector v = convertPixelsToVariable(pixel);
    return calculateHeightValue(v);
}

void generateSkiParadise() {
    for (int y = 0; y < screenHeight; y++) {
        for (int x = 0; x < screenWidth; x++) {
            const double height = calculateHeightValueFromPixel(Vector(x, y));
            //height: 0..4,5
            //0: black (0,0,0) -> 4.5: white (1,1,1)
            const float greyCode = (float) (height * 0.2222f);
            image[y * screenWidth + x] = Color(greyCode, greyCode, greyCode);
        }
    }
}

const Vector getGradientVarVector(const Vector varFrom) {

    const double x = 2 * cos(2 * varFrom.x) + varFrom.y / 8.0; // d/dx f(v)
    const double y = varFrom.x / 8.0 - 3 * sin(3 * varFrom.y); // d/dy f(v)

    return Vector(x, y);
}

const double getDropAngleInRad(Vector v1, Vector v2) {
    const double heightDiff = calculateHeightValue(v1) - calculateHeightValue(v2);
    const double angleInRad = atan(heightDiff / get100mInVar());

    return angleInRad;
}

const double getDropAngleInDeg(Vector v1, Vector v2) {
    return getDropAngleInRad(v1, v2) * (180 / M_PI);
}

const Vector getDropVarVector(Vector varClicked) {
    Vector varGradPos = getGradientVarVector(varClicked);

    const double gradLength = (varClicked - varGradPos).Length();
    //variable: 6 = 10 000 m -> /100 -> 100m
    const double multiplier100m = get100mInVar();

    const Vector dropVarVector = Vector(varClicked.x + ((varClicked.x - varGradPos.x) / gradLength) * multiplier100m,
            varClicked.y + ((varClicked.y - varGradPos.y) / gradLength) * multiplier100m);

    return dropVarVector;
}

double getGlRotateAngleInRad(const Vector glHead, Vector glDrop) {
    Vector helper(glHead.x, glDrop.y);

    double rad = acos((helper - glHead).Length() / (glDrop - glHead).Length());

    if (glDrop.y > glHead.y) { //1. negyed fix
        rad = M_PI - rad;
    }

    if (glDrop.x < glHead.x) {
        if (glHead.y > 0 && glDrop.y > 0) { //2. negyed fix
            rad = -rad;
        }

        if (glHead.y < 0 && glDrop.y < 0) { //4. negyed fix
            rad = -rad;
        }
    }

    return rad;
}

void rotatePoint(const Vector glBase, Vector& rotate, double angleInRad) {

    double x = (rotate.x - glBase.x) * cos(angleInRad) - (rotate.y - glBase.y) * sin(angleInRad) + glBase.x;
    double y = (rotate.x - glBase.x) * sin(angleInRad) + (rotate.y - glBase.y) * cos(angleInRad) + glBase.y;

    rotate.x = x;
    rotate.y = y;
}

void onInitialization() {
    glViewport(0, 0, screenWidth, screenHeight);

    generateSkiParadise();
}

void onDisplay() {
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // torlesi szin beallitasa
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

    glDrawPixels(screenWidth, screenHeight, GL_RGB, GL_FLOAT, image);

    glBegin(GL_LINES);

    for (int i = 0; i < tracks * (SECTION_PER_TRACK * 2); i = i + 2) {
        Color c = lineColors[i / 2];
        glColor3f(c.r, c.g, c.b);

        if (!(linesCoords[i].x == 0.0 && linesCoords[i].y == 0.0 &&
                linesCoords[i + 1].x == 0.0 && linesCoords[i + 1].y == 0.0)) {

            glVertex2f(linesCoords[i].x, linesCoords[i].y);
            glVertex2f(linesCoords[i + 1].x, linesCoords[i + 1].y);
        }
    }

    glEnd();

    //sielok kirajzolasa
    glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_TRIANGLES);

    for (int i = 0; i < skiers * 3; i = i + 3) {

        if (!(skiersCoords[i].x == 0.0 && skiersCoords[i].y == 0.0 &&
                skiersCoords[i + 1].x == 0.0 && skiersCoords[i + 1].y == 0.0)) {

            glVertex2f(skiersCoords[i].x, skiersCoords[i].y);
            glVertex2f(skiersCoords[i + 1].x, skiersCoords[i + 1].y);
            glVertex2f(skiersCoords[i + 2].x, skiersCoords[i + 2].y);
        }
    }

    glEnd();

    glutSwapBuffers();
}

void onKeyboard(unsigned char key, int x, int y) {
    if (key == 'd') glutPostRedisplay();

    if (key == 's' && startedSkiers < MAX_SKIERS) {
        startedSkiers++;
        glutPostRedisplay();
    }
}

void onMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT && state == GLUT_DOWN) {
        //ha meg nincs 10 palya: palya generalas a pontbol

        if (tracks < 10) {
            Vector pixel(x, y);
            Vector beginVector = convertPixelsToVariable(pixel);

            //sielo letrehozasa
            int skierIndex = skiers * 3;
            Vector skierHead = convertPixelsToGl(pixel);
            skiersCoords[skierIndex] = Vector(skierHead.x, skierHead.y);
            skiersCoords[skierIndex + 1] = Vector(skierHead.x + SKIERS_SIZE, skierHead.y + SKIERS_SIZE);
            skiersCoords[skierIndex + 2] = Vector(skierHead.x - SKIERS_SIZE, skierHead.y + SKIERS_SIZE);

            //sielo palyaval szembeforditasa
            double angleInRad = getGlRotateAngleInRad(skierHead, convertVariablesToGl(getDropVarVector(beginVector)));
            rotatePoint(skierHead, skiersCoords[skierIndex + 1], angleInRad);
            rotatePoint(skierHead, skiersCoords[skierIndex + 2], angleInRad);

            //kovetkezo sielo
            skiers++;

            int section = 0;
            int linesFromTrack = tracks * (SECTION_PER_TRACK * 2);
            while (section < 20) {

                const Vector dropVarVector = getDropVarVector(beginVector);

                //leejto vagy emelkedo szakasz jon? emelkedon nem tud felsiklani
                double h1 = calculateHeightValue(beginVector);
                double h2 = calculateHeightValue(dropVarVector);

                //a 0. szakaszt rajzolja meg, hogy ne tunjon �gy, hogy nem hoz letre
                //palyat
                if (section > 0 && (h2 > h1 || fequals(h2, h1))) {
                    break;
                }

                int index = linesFromTrack + section * 2;

                linesCoords[index] = convertVariablesToGl(beginVector);

                linesCoords[index + 1] = convertVariablesToGl(dropVarVector);

                //beginVarVector -> dropVarVector
                //alapb�l kek: 20 fok alatti
                Color lineColor = Color(0.0, 0.0, 1.0);

                double dropAngle = getDropAngleInDeg(beginVector, dropVarVector);

                if (dropAngle >= 40) { //legalabb 40 fok -> fekete
                    lineColor.b = 0.0;
                } else if (dropAngle > 20 && dropAngle < 40) { //20-40 fok kozott -> piros
                    lineColor.r = 1.0;
                    lineColor.b = 0.0;
                }

                lineColors[index / 2] = lineColor;

                beginVector = dropVarVector; //kovetkezo iteraciohoz

                ++section;
            }

            tracks = tracks + 1;
        }

        glutPostRedisplay();
    }
}

void simulateWorld(long tstart, long tend) {
    float dt = 50;
    for (float ts = tstart; ts < tend; ts += dt) {
        float te;
        if (tend >= ts + dt) {
            te = ts + dt;
        } else {
            te = tend;
        }

        for (int i = 0; i < startedSkiers; i++) {
            // step skiers
            int skierIndex = i * 3;
            //            skiersCoords[skierIndex] = Vector(skierHead.x, skierHead.y);
            //            skiersCoords[skierIndex + 1] = Vector(skierHead.x + SKIERS_SIZE, skierHead.y + SKIERS_SIZE);
            //            skiersCoords[skierIndex + 2] = Vector(skierHead.x - SKIERS_SIZE, skierHead.y + SKIERS_SIZE);
        }

    }
}

void onIdle() {

    if (!working) {
        working = true;
        long old_time = time_;
        time_ = glutGet(GLUT_ELAPSED_TIME);

        simulateWorld(old_time, time_);

        working = false;
    }

    glutPostRedisplay();
}

// ...Idaig modosithatod
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// A C++ program belepesi pontja, a main fuggvenyt mar nem szabad bantani

int main(int argc, char **argv) {
    glutInit(&argc, argv); // GLUT inicializalasa
    glutInitWindowSize(600, 600); // Alkalmazas ablak kezdeti merete 600x600 pixel
    glutInitWindowPosition(100, 100); // Az elozo alkalmazas ablakhoz kepest hol tunik fel
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // 8 bites R,G,B,A + dupla buffer + melyseg buffer

    glutCreateWindow("Grafika hazi feladat"); // Alkalmazas ablak megszuletik es megjelenik a kepernyon

    glMatrixMode(GL_MODELVIEW); // A MODELVIEW transzformaciot egysegmatrixra inicializaljuk
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION); // A PROJECTION transzformaciot egysegmatrixra inicializaljuk
    glLoadIdentity();

    onInitialization(); // Az altalad irt inicializalast lefuttatjuk

    glutDisplayFunc(onDisplay); // Esemenykezelok regisztralasa
    glutMouseFunc(onMouse);
    glutIdleFunc(onIdle);
    glutKeyboardFunc(onKeyboard);

    glutMainLoop(); // Esemenykezelo hurok

    return 0;
}