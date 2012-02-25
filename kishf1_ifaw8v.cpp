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
#include <iostream>
using namespace std;

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

const int screenWidth = 600; // alkalmazás ablak felbontása
const int screenHeight = 600;


Color image[screenWidth*screenHeight]; // egy alkalmazás ablaknyi kép

const double VARIABLE_PIXEL_RATE = 100.0;
const int SECTION_PER_TRACK = 20;
const int LINES_SIZE = 400;
Vector linesCoords[LINES_SIZE]; //10 pálya * 20 szakasz * 2 koord
Color lineColors[LINES_SIZE / 2];

int tracks = 0; // sípályák száma

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

const double calculateHeightValueFromPixel(const Vector pixel) {
    const Vector v = convertPixelsToVariable(pixel);
    return sin(2 * v.x) + cos(3 * v.y) + (v.x * v.y) / 8.0;
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

const Vector getDropVarVector(Vector varClicked) {
    Vector varGradPos = getGradientVarVector(varClicked);

    const double gradLength = (varClicked - varGradPos).Length();
    //variable: 6 = 10 000 m -> /100 -> 100m
    const double multiplier = screenWidth / VARIABLE_PIXEL_RATE / 100;

    const Vector dropVarVector = Vector(varClicked.x + ((varClicked.x - varGradPos.x) / gradLength) * multiplier,
            varClicked.y + ((varClicked.y - varGradPos.y) / gradLength) * multiplier);

    return dropVarVector;
}

void onInitialization() {
    glViewport(0, 0, screenWidth, screenHeight);

    generateSkiParadise();
}

// Rajzolas, ha az alkalmazas ablak ervenytelenne valik, akkor ez a fuggveny hivodik meg

void onDisplay() {
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // torlesi szin beallitasa
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

    // ..

    // Peldakent atmasoljuk a kepet a rasztertarba
    glDrawPixels(screenWidth, screenHeight, GL_RGB, GL_FLOAT, image);

    glBegin(GL_LINES);

    for (int i = 0; i < tracks * (SECTION_PER_TRACK * 2); i = i + 2) {
        Color c = lineColors[i / 2];
        glColor3f(c.r, c.g, c.b);

        if (!(linesCoords[i].x == 0.0 && linesCoords[i].y == 0.0 &&
                linesCoords[i + 1].x == 0.0 && linesCoords[i].y == 0.0)) {

            glVertex2f(linesCoords[i].x, linesCoords[i].y);
            glVertex2f(linesCoords[i + 1].x, linesCoords[i + 1].y);
        }
    }

    glEnd();

    // ...

    glutSwapBuffers(); // Buffercsere: rajzolas vege

}

// Billentyuzet esemenyeket lekezelo fuggveny

void onKeyboard(unsigned char key, int x, int y) {
    if (key == 'd') glutPostRedisplay(); // d beture rajzold ujra a kepet

    //'s' lenyomasra a kovetkezo palyarol sielo inditasa
}

// Eger esemenyeket lekezelo fuggveny

void onMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT && state == GLUT_DOWN) {
        //ha meg nincs 10 palya: palya generalas a pontbol

        if (tracks < 10) {
            Vector beginVector = convertPixelsToVariable(Vector(x, y));
            bool exit = false;
            int section = 0;
            int linesFromTrack = tracks * (SECTION_PER_TRACK * 2);
            cout << endl << "linesFromTrack=" << linesFromTrack << endl;
            while (!exit && section < 20) {

                cout << "tracks=" << tracks << endl;
                cout << "section=" << section << endl;

                int index = linesFromTrack + section * 2;

                lineColors[index / 2] = Color(1.0, 0.0, 1.0);
                linesCoords[index] = convertVariablesToGl(beginVector);

                cout << "index=" << index << endl;
                cout << "linesCoords[" << index << "]=" << linesCoords[index].x;
                cout << "; y=" << linesCoords[index].y << endl;

                const Vector dropVarVector = getDropVarVector(beginVector);
                beginVector = dropVarVector;

                linesCoords[index + 1] = convertVariablesToGl(dropVarVector);

                cout << "linesCoords[" << index + 1 << "]=" << linesCoords[index + 1].x;
                cout << "; y=" << linesCoords[index + 1].y << endl;

                ++section;
            }

            cout << "tracks=" << tracks << endl;
            tracks = tracks + 1;
        }

        glutPostRedisplay(); // Ilyenkor rajzold ujra a kepet
    }
}

// `Idle' esemenykezelo, jelzi, hogy az ido telik, az Idle esemenyek frekvenciajara csak a 0 a garantalt minimalis ertek

void onIdle() {

    long time = glutGet(GLUT_ELAPSED_TIME); // program inditasa ota eltelt ido

    //animáció
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