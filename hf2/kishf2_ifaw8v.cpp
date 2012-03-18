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

    Vector& operator=(const Vector & other) {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
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

//------------------------------------------------------------------------------

const int screenWidth = 600;

const double virtualWidth = 1000; //1m = 1000mm
double currentWidth = 78;

const int NR_OF_CURVES = 10;
const int NR_OF_CTRPs = 100; //number of control points / curve

//fibonacci
double fibonacci[NR_OF_CTRPs];

class CatmullRomCurve {
private:

    Vector MAGIC(double t, int i) {
        Vector a, b, c, d;

        //        cout << "t=" << t << endl;
        //        cout << "i=" << i << endl;

        double t_i = fibonacci[i];
        double t_i_plus_1 = fibonacci[i + 1];
        double t_i_plus_2 = fibonacci[i + 2];
        double t_i_minus_1 = fibonacci[i - 1];

        //        cout << "t_i=" << t_i << endl;
        //        cout << "t_i_plus_1=" << t_i_plus_1 << endl;
        //        cout << "t_i_plus_2=" << t_i_plus_2 << endl;
        //        cout << "t_i_minus_1=" << t_i_minus_1 << endl;

        Vector f_i = ctrlPoints[i];
        Vector f_i_plus_1 = ctrlPoints[i + 1];
        Vector f_i_plus_2 = ctrlPoints[i + 2];
        Vector f_i_minus_1 = ctrlPoints[i - 1];

        //        cout << "f_i = " << f_i.x << " :: " << f_i.y << endl;
        //        cout << "f_ip1 = " << f_i_plus_1.x << " :: " << f_i_plus_1.y << endl;
        //        cout << "f_ip2 = " << f_i_plus_2.x << " :: " << f_i_plus_2.y << endl;
        //        cout << "f_im1 = " << f_i_minus_1.x << " :: " << f_i_minus_1.y << endl;

        Vector v_i = ((f_i - f_i_minus_1) * (1 / (t_i - t_i_minus_1)) +
                (f_i_plus_1 - f_i) * (1 / (t_i_plus_1 - t_i))) * 0.5;

        //        Vector temp1 = f_i - f_i_minus_1;
        //        double temp2 = 1 / (t_i - t_i_minus_1);
        //        Vector temp3 = temp1 * (1 / (t_i - t_i_minus_1));
        //
        //        cout << "temp1 = " << temp1.x << " :: " << temp1.y << endl;
        //        cout << "temp2 = " << temp2 << endl;
        //        cout << "temp3 = " << temp3.x << " :: " << temp3.y << endl;
        //        cout << "v_i = " << v_i.x << " :: " << v_i.y << endl;

        //szerintem nem jó: v_i(t+1) !!!
        Vector v_i_plus_1 = ((f_i_plus_1 - f_i) * (1 / (t_i_plus_1 - t_i)) +
                (f_i_plus_2 - f_i_plus_1) * (1 / (t_i_plus_2 - t_i_plus_1))) * 0.5;

        d = f_i;
        c = v_i;

        b = ((f_i_plus_1 - f_i) * 3) * (1 / pow(t_i_plus_1 - t_i, 2)) -
                (v_i_plus_1 + v_i * 2) * (1 / (t_i_plus_1 - t_i));

        a = (v_i_plus_1 + v_i) * (1 / pow(t_i_plus_1 - t_i, 2)) -
                ((f_i_plus_1 - f_i) * 2) * (1 / pow(t_i_plus_1 - t_i, 3));

        //b és v_i_plus_1 fuggnek egymastol
        //        Vector v_i_plus_1 = 3 * a * pow(t_i_plus_1 - t_i) +
        //                2 * b * pow(t_i_plus_1 - t_i, 2) * c;

        // t{i} <= t < t{i+1}
        //t{i} = fibonacci[i]
        Vector f_t = a * pow(t - t_i, 3) + b * pow(t - t_i, 2) +
                c * (t - t_i) + d;

        //        cout << "f_t = " << f_t.x << " :: " << f_t.y << endl;

        return f_t;
    }
public:
    Vector ctrlPoints[NR_OF_CTRPs];
    int numOfPoints;

    CatmullRomCurve() { //: dt(0.025), numOfPoints(0)
        numOfPoints = 0;
    }

    void draw(const Color c) {
        glColor3f(c.r, c.g, c.b);
        glBegin(GL_LINE_STRIP);
        for (int i = 1; i < numOfPoints - 2; ++i) {
            double rate = (fibonacci[i + 1] - fibonacci[i]) / 100.0;
            for (double t = fibonacci[i]; t < fibonacci[i + 1]; t += rate) {
                Vector v = MAGIC(t, i);
                glVertex2f(v.x, v.y);
            }

        }
        glEnd();

        glBegin(GL_TRIANGLES);
        glColor3f(0.0, 0.0, 1.0);
        for (int i = 0; i < numOfPoints; i++) {
            glVertex2f(ctrlPoints[i].x, ctrlPoints[i].y);
            glVertex2f(ctrlPoints[i].x - 1, ctrlPoints[i].y - 1);
            glVertex2f(ctrlPoints[i].x + 1, ctrlPoints[i].y - 1);
        }
        glEnd();
    }

    void addVector(const Vector c) {
        if (numOfPoints < NR_OF_CTRPs) {
            ctrlPoints[numOfPoints++] = c;
        }
    }
};

//curves, control points
int currentCurveIndex = 0;
CatmullRomCurve crCurves[NR_OF_CURVES];

//colors of curves
Color curveColors[NR_OF_CURVES];

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

MODE programMode = SELECT;

const bool fequals(float f1, float f2) {
    if (fabs(f1 - f2) < 0.001) return true;
    return false;
}

/*
 * Binet form
 */
const double getFibonacciNr(int n) {
    double sqrt5 = sqrt(5);
    return (pow(1 + sqrt5, n) - pow(1 - sqrt5, n)) / (sqrt5 * pow(2, n));
}

void onInitialization() {
    glViewport(0, 0, screenWidth, screenWidth);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const int plus = (virtualWidth / 2) + (currentWidth / 2);
    const int minus = (virtualWidth / 2) - (currentWidth / 2);

    gluOrtho2D(minus, //left
            plus, //right
            minus, //bottom
            plus); //top

    //fill up array with fibonacci numbers - Binet form
    for (int i = 2; i <= NR_OF_CTRPs; i++) {
        fibonacci[i - 1] = getFibonacciNr(i);
    }

    //fill up array of curves' colors
    curveColors[0] = Color(0.0, 0.0, 0.0); //black
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

    for (int i = 0; i < currentCurveIndex + 1; i++) {
        crCurves[i].draw(curveColors[i]);
    }

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

        if (currentWidth + (currentWidth / 10) > virtualWidth) { //10%
            currentWidth = virtualWidth;
        } else {
            currentWidth = currentWidth + (currentWidth / 10);
        }

        const int plus = (virtualWidth / 2) + (currentWidth / 2);
        const int minus = (virtualWidth / 2) - (currentWidth / 2);

        glLoadIdentity();
        gluOrtho2D(minus, plus, minus, plus);

        cout << "INFO: zoomed out. size: " << currentWidth << endl;
    }

    glutPostRedisplay();
}

void onMouse(int button, int state, int x, int y) {

    if (button == GLUT_LEFT) {
        if (state == GLUT_DOWN) {

            if (time_ - clickedTime < 500
                    && clickedPos.x == x && clickedPos.y == y) { //0.5s && cursor not moved
                if (programMode == EDIT) {
                    //end CR cpt input
                    cout << "DEBUG: end curve" << endl;

                    if (currentCurveIndex < NR_OF_CURVES) {
                        ++currentCurveIndex; //next curve
                    }
                }
            } else { //single click
                cout << "DEBUG: single click" << endl;
                if (programMode == EDIT) {

                    double worldXFrom0 = (currentWidth / screenWidth) * x;
                    double worldYFrom0 = ((currentWidth / screenWidth) * y - currentWidth) * -1.0f;

                    double worldXcenter = (virtualWidth / 2) - (currentWidth / 2) + worldXFrom0;
                    double worldYcenter = (virtualWidth / 2) - (currentWidth / 2) + worldYFrom0;

                    //add CR cpt, if not contains 100 points already
                    if (currentCurveIndex < NR_OF_CURVES &&
                            crCurves[currentCurveIndex].numOfPoints < NR_OF_CTRPs - 1) {
                        cout << "DEBUG: --add CR cpt" << endl;

                        Vector newVector = Vector(worldXcenter, worldYcenter);
                        crCurves[currentCurveIndex].addVector(newVector);

                        cout << "ctrlPoints[" << currentCurveIndex << "][";
                        cout << crCurves[currentCurveIndex].numOfPoints - 1 << "]=";
                        cout << newVector.x << " :: " << newVector.y << endl;
                    }

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