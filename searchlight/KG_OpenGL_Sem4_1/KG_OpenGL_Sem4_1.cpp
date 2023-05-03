// tri1.cpp : Defines the entry point for the console application.
//


#include <stdlib.h>
#include "glut.h"
#include "glaux.h"
#include <math.h>


float xRot, yRot;

GLint k = 2;
GLfloat	 lightPos[] = { 0.0f, 0.0f, 75.0f, 1.0f };
GLfloat  specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat  specref[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat  ambientLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat  spotDir[] = { 0.0f, 0.0f, -1.0f };

GLfloat light3_diffuse[] = { 0.4, 0.7, 0.2 };
GLfloat light3_position[] = { 0.0, 0.0, 1.0, 1.0 };
GLfloat light3_spot_direction[] = { 0.0, 0.0, -1.0 };

void SetupRC() {

    glEnable(GL_DEPTH_TEST);	// Удаление скрытых поверхностей
    glEnable(GL_LIGHTING);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    glEnable(GL_LIGHT3);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, light3_diffuse);
    glLightfv(GL_LIGHT3, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT3, GL_POSITION, light3_position);

    glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 30);

    glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, light3_spot_direction);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);   
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    // Задана полная зеркальная отражательная способность с ярким бликом     	
    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);

      // Допускаются значения GL_SHININESS  в интервале [0; 128].   
    glMateriali(GL_FRONT, GL_SHININESS, 128);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_NORMALIZE);
}
void RenderScene(void) {
    glShadeModel(GL_SMOOTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix(); // сохраняем значение модельно-видовой матрицы
    // вращение модели
    glRotatef(yRot, 0.0f, 1.0f, 0.0f);
    glRotatef(xRot, 1.0f, 0.0f, 0.0f);
    glLightfv(GL_LIGHT3, GL_POSITION, lightPos); // настройка позиции ИС
    if (k == 2)
        glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, spotDir);

    //  Присваиваем следующим объектам красный цвет
    glColor3ub(255, 0, 0);

    //  перемещаем источник света (ИС) в новую позицию
    glTranslatef(lightPos[0], lightPos[1], lightPos[2]);
    glutSolidCone(4.0f, 6.0f, 15, 15); // рисуем в ней конус

    // Рисуем лампочку в виде желтой сферы
    glPushAttrib(GL_LIGHTING_BIT);
    // Выключаем освещение и рисуем сферу-лампочку
    glDisable(GL_LIGHTING);
    glColor3ub(255, 255, 0);
    glutSolidSphere(3.0f, 15, 15);

    // Восстановить переменные состояния освещения
    glPopAttrib();

    // Восстанавливаются координатные преобразования  
    glPopMatrix();

    GLfloat x, y;
    if (k == 2) 
    {
        glColor3f(.1, .8, 0);
        glutSolidSphere(30.0f, 50, 50);
    }
    else 
        if (k == 1)
        {
            GLfloat   color[] = { 0.75f,    0.0f,    1.0f,    1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

            glBegin(GL_QUADS);
            glNormal3f(0.0, 0.0, -1.0);
            for (x = -60.0; x < 60.0; x += 0.5)
            {
                for (y = -60.0; y < 60.0; y += 0.5)
                {
                    glVertex3f(x, y, 0.0);
                    glVertex3f(x, y + 0.5, 0.0);
                    glVertex3f(x + 0.5, y + 0.5, 0.0);
                    glVertex3f(x + 0.5, y, 0.0);
                }
            }
            glEnd();
            glShadeModel(GL_SMOOTH);
        }

    // Отображаются результаты
    glutSwapBuffers();
}


void SpecialKeys(int key, int x, int y) { // обработка нажатия на клавиши
    if (key == GLUT_KEY_UP)
        xRot -= 5.0f;
    if (key == GLUT_KEY_DOWN)
        xRot += 5.0f;
    if (key == GLUT_KEY_LEFT)
        yRot -= 5.0f;
    if (key == GLUT_KEY_RIGHT)
        yRot += 5.0f;
    if (key > 356.0f)
        xRot = 0.0f;
    if (key < -1.0f)
        xRot = 355.0f;
    if (key > 356.0f)
        yRot = 0.0f;
    if (key < -1.0f)
        yRot = 355.0f;

    // Обновить окно
    glutPostRedisplay();
}
void resize(int w, int h) {
    GLfloat fAspect;

    // Set Viewport to window dimensions
    glViewport(0, 0, w, h);

    // Обновить матрицу проецирования
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    fAspect = (GLfloat)w / (GLfloat)h;
    gluPerspective(35.0f, fAspect, 0.1, 10000.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); // обновить модельно-видовую матрицу
    glTranslatef(0.0f, 0.0f, -250.0f);
}
void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case VK_ESCAPE:
    {
        exit(0);
    }

    case '1':
    {
        k = 1;
        break;
    }
    case '2':
    {
        k = 2;
        break;
    }
    }
    glutPostRedisplay();
}

int main(int argc, char* argv[]) {
    // настройка среды
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Прожектор");

    glutReshapeFunc(resize);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);
    SetupRC();
    glutMainLoop();

    return 0;
}





