//=============================================================================================
// Szamitogepes grafika hazi feladat keret. Ervenyes 2012-tol.          
// A //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// sorokon beluli reszben celszeru garazdalkodni, mert a tobbit ugyis toroljuk. 
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat. 
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni (printf is fajlmuvelet!)
// - new operatort hivni az onInitialization függvényt kivéve, a lefoglalt adat korrekt felszabadítása nélkül 
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

//gyilok!
#include <iostream>
using namespace std;
//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Innentol modosithatod...

//--------------------------------------------------------
// 3D Vektor
//--------------------------------------------------------

struct Vector {
    double x, y, z;

    Vector() {
        x = y = z = 0;
    }

    Vector(double x0, double y0, double z0 = 0) {
        x = x0;
        y = y0;
        z = z0;
    }

    Vector operator*(double a) {
        return Vector(x * a, y * a, z * a);
    }

    Vector operator+(const Vector & v) {
        return Vector(x + v.x, y + v.y, z + v.z);
    }

    Vector operator-(const Vector & v) {
        return Vector(x - v.x, y - v.y, z - v.z);
    }

    double operator*(const Vector & v) { // dot product
        return (x * v.x + y * v.y + z * v.z);
    }

    Vector operator%(const Vector & v) { // cross product
        return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }

    double Length() {
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

//raster - gyilok!
const int screenWidth = 600;
const int screenHeight = 600;
Color image[screenWidth*screenHeight];

//fibonacci
double fibonacci[100];

//curves, control points

//colors of curves
Color curveColors[10];

//gyilok!
const double VARIABLE_PIXEL_RATE = 100.0;

long time_ = 0;
long clickedTime = 0;
Vector clickedPos;
bool working = false;

/*
 * Program mode
 */
enum MODE {
    EDIT, SELECT
};

MODE programMode = SELECT; //program starts in select mode

const bool fequals(float f1, float f2) {
    if (fabs(f1 - f2) < 0.001) return true;
    return false;
}

const Vector convertPixelsToGl(const Vector pixel) {
    const double x = (pixel.x - screenWidth / 2.0) / ((double) screenWidth / 2.0);
    const double y = (pixel.y - screenHeight / 2.0) / ((double) screenHeight / 2.0);
    return Vector(x, y * -1.0);
}

/*
 * Binet form
 */
const double getFibonacciNr(int n) {
    double sqrt5 = sqrt(5);
    return (pow(1 + sqrt5, n) - pow(1 - sqrt5, n)) / (sqrt5 * pow(2, n));
}

void onInitialization() {
    glViewport(0, 0, screenWidth, screenHeight);

    //fill up array with fibonacci numbers - Binet form
    for (int i = 1; i <= 100; i++) {
        fibonacci[i] = getFibonacciNr(i);
        //        cout << i << ": " << fibonacci[i] << endl;
    }

    //fill up array of curves' colors
    curveColors[0] = Color(1.0, 1.0, 1.0); //black
    curveColors[1] = Color(1.0, 0.0, 0.0); //red
    curveColors[2] = Color(0.0, 1.0, 0.0); //green
    curveColors[3] = Color(0.0, 0.0, 1.0); //blue
    curveColors[4] = Color(1.0, 1.0, 0.0); //yellow
    curveColors[5] = Color(1.0, 0.0, 1.0); //pink
    curveColors[6] = Color(0.0, 1.0, 1.0); //cyan
    curveColors[7] = Color(0.4, 0.4, 0.4); //grey
    curveColors[8] = Color(0.627, 0.125, 0.941); //purple
    curveColors[9] = Color(0.804, 0.36, 0.36); //indian red
}

void onDisplay() {
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glDrawPixels(screenWidth, screenHeight, GL_RGB, GL_FLOAT, image);

    glBegin(GL_LINES);

    //    Color c = lineColors[i / 2];
    //    glColor3f(c.r, c.g, c.b);
    //    glVertex2f(linesCoords[i].x, linesCoords[i].y);
    //    glVertex2f(linesCoords[i + 1].x, linesCoords[i + 1].y);

    glEnd();

    glutSwapBuffers();
}

void onKeyboard(unsigned char key, int x, int y) {
    if (key == 'e') { //edit mode
        programMode = EDIT;
        cout << "INFO: switch to EDIT mode" << endl;
    } else if (key == 'p') { //select mode
        programMode = SELECT;
        cout << "INFO: switch to SELECT mode" << endl;
    } else if (key == 'z') { //zoom out
    }

    glutPostRedisplay();

}

void onMouse(int button, int state, int x, int y) {

    if (button == GLUT_LEFT) {
        if (state == GLUT_DOWN) {
            //double click?
            if (time_ - clickedTime < 500
                    && clickedPos.x == x && clickedPos.y == y) { //0.5s && cursor not moved
                if (programMode == EDIT) {
                    //end CR cpt input
                    cout << "DEBUG: end curve" << endl;
                }
            } else { //single click
                cout << "DEBUG: single click" << endl;
                //add CR cpt, if not contains 100 points already
                if (programMode == EDIT) {
                    cout << "DEBUG: --add CR cpt" << endl;
                } else if (programMode == SELECT) {
                    //search for a point of a curve in 10x10px around click position
                    //if found save coords for moving
                    cout << "DEBUG: --search for point from here: " << x << "; " << y << endl;
                }
            }

            clickedTime = time_;
            clickedPos.x = x;
            clickedPos.y = y;
        } else if (state == GLUT_UP) {
            //if moving: move curve to here
            if (programMode == SELECT) {
                cout << "DEBUG: mouse left up: if moving move curve to here: " << x << "; " << y << endl;
            }
        }
    } else if (button == GLUT_RIGHT_BUTTON) {
        if (programMode == SELECT) {
            if (state == GLUT_DOWN) {
                //search for a point of a curve in 10x10px around click position
                cout << "DEBUG: mouse right down: search for point from here: " << x << "; " << y << endl;
            } else if (state == GLUT_UP) {
                //rotate with vector angle
                cout << "DEBUG: mouse right up: calc rotate from this coords: " << x << "; " << y << endl;
            }
        }
    }

    glutPostRedisplay();
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

        //work

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