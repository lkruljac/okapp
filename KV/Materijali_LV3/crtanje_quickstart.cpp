#include <stdlib.h>
#include <stdio.h>
#include "/usr/include/GL/glut.h"
#include "/usr/include/sndfile.hh"

#include <math.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <cstdlib>

using namespace std; // dopušta ispis iz konzole bez pisanja: std::
/*! GLUT funkcija za poziv za početnog ekrana */
void display(void);
/*! GLUT funkcija za ponovno iscrtavanje */
void reshape(int, int);
/*! GLUT idle funkcija */
void idle();
void f_tipke(int, int, int);

int zvuk=0;


/** Učitava ppm datoteku (sliku) s diska.
 @input filename - putanja do slike
 @input width - širina slike
 @input height - visina slike
 @return Vraca vrijednosti piksela u RGB obliku kao unsigned chars (R0 G0 B0 R1 G1 B1 R2 G2 B2 .... itd) **/
unsigned char* loadPPM(const char* filename, int& width, int& height){
	const int BUFSIZE = 128;
	FILE* fp;
	unsigned int read;
	unsigned char* rawData;
	char buf[3][BUFSIZE];
	char* retval_fgets;
	size_t retval_sscanf;

	if ( (fp=fopen(filename, "rb")) == NULL)
	{
		std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
		width = 0;
		height = 0;
		return NULL;
	}

	retval_fgets = fgets(buf[0], BUFSIZE, fp);

	// Pročitaj širinu i visinu:
	do
	{
		retval_fgets=fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');
	retval_sscanf=sscanf(buf[0], "%s %s", buf[1], buf[2]);
	width  = atoi(buf[1]);
	height = atoi(buf[2]);

	// Pročitaj maxval:
	do
	{
	  retval_fgets=fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');

	// Pročitaj podatke iz slike:
	rawData = new unsigned char[width * height * 3];
	read = fread(rawData, width * height * 3, 1, fp);
	fclose(fp);
	if (read != 1)
	{
		std::cerr << "error parsing ppm file, incomplete data" << std::endl;
		delete[] rawData;
		width = 0;
		height = 0;
		return NULL;
	}
	return rawData;
}

// inicijalizacija OpenGL-a
void initGL()
{
  glEnable(GL_TEXTURE_2D);   // omogući mapiranje tekstura
  glShadeModel(GL_SMOOTH);   // omogući 'smooth shading'
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // crna pozadina
  glClearDepth(1.0f);        // buffer za dubinu
  glDepthFunc(GL_LEQUAL);    // konfiguracija testiranja dubine
  // omogući dodatne postavke vezane za korekciju zbog perspektive, anti-aliasinga i slicno:
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
}

// učitaj sliku u objekt teksture
void loadTexture()
{
  GLuint texture[1];     // prostor za jednu teksturu
  int twidth, theight;   // širina/visin u pikselima
  unsigned char* tdata;  // podaci piksela
  // Učitavanje slike:
  tdata = loadPPM("auto3.ppm", twidth, theight);
  if (tdata==NULL) return;
  // Kreiranje ID-a za teksturu
  glGenTextures(1, &texture[0]);
  // Postavljanje ove teksture kao one s kojom radimo
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  // Generiraj teksturu
  glTexImage2D(GL_TEXTURE_2D, 0, 3, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, tdata);
  // Filtriranje za povećanje i smanjenje
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int main(int argc, char** argv){

  glutInit(&argc, argv);
  /* postavi veličinu prozora na  512 x 512 */
  glutInitWindowSize(512, 512);
  /*postavi mod ekrana u: Red, Green, Blue, Alpha
    alociraj depth buffer
    omogući double buffering
  */
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  /* kreiraj prozor */
  glutCreateWindow("Texturing Example");
  /* postavi GLUT funkciju za display callback
    ova funkcija će se pozvati treba biti iscrtan prozor*/
  glutDisplayFunc(display);
  /* postavi GLUT funkciju za reshape
    ovo će se pozvati uvijek kada se promjeni veličina prozora
    uključujući prvi put
  */
  glutReshapeFunc(reshape);

  glutIdleFunc(idle);

  glutSpecialFunc(f_tipke);

  loadTexture();
  initGL();

  /* Ulazak u glavnu petlju koja se neprestano izvodi i obrađuje evente koji se događaju
     */
  glutMainLoop();
  return 0;
}

void f_tipke(int key, int x, int y){

	switch(key){
		case GLUT_KEY_F1:
			zvuk=1;
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  			glBegin(GL_QUADS);
    			glColor3f(1, 1, 1);

   			 // koordinate točaka pozadinskog pravokutnika (upside down)
    			glTexCoord2f(0, 1); glVertex3f(-2, -1, 0);
    			glTexCoord2f(1, 1); glVertex3f(2, -1, 0);
    			glTexCoord2f(1, 0); glVertex3f(2, 1, 0);
    			glTexCoord2f(0, 0); glVertex3f(-2, 1, 0);

  			/* crtanje cetverokuta */
  			glEnd();
   			glColor3f(1.0,0.0,0.0);
     			glBegin(GL_QUADS);
         		//prva linija prednja lijeva strana
        		glVertex3f(-1.22f, -0.20f, 0.0f); // top left
        		glVertex3f(-1.15f,-0.16f, 0.0f); // top right 
        		glVertex3f(-1.03f, -0.36f, 0.0f); // bottom right
        		glVertex3f(-1.1f, -0.40f, 0.0f); // bottom left
        		glEnd();
        		glutSwapBuffers();


			break;
		case GLUT_KEY_F2:
			zvuk=2;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  			glBegin(GL_QUADS);
    			glColor3f(1, 1, 1);

    			 // koordinate točaka pozadinskog pravokutnika (upside down)
    			glTexCoord2f(0, 1); glVertex3f(-2, -1, 0);
    			glTexCoord2f(1, 1); glVertex3f(2, -1, 0);
    			glTexCoord2f(1, 0); glVertex3f(2, 1, 0);
    			glTexCoord2f(0, 0); glVertex3f(-2, 1, 0);

    			/* crtanje cetverokuta */
  			glEnd();
   			glColor3f(1.0,0.0,0.0);
     			glBegin(GL_QUADS);
         		//prva linija prednja lijeva strana
        		glVertex3f(-1.22f, -0.20f, 0.0f); // top left
        		glVertex3f(-1.15f,-0.16f, 0.0f); // top right 
        		glVertex3f(-1.03f, -0.36f, 0.0f); // bottom right
        		glVertex3f(-1.1f, -0.40f, 0.0f); // bottom left
        
			glColor3f(1.0,0.35,0.35);
        		//druga linija prednja lijeva strana
        		glVertex3f(-1.35f,-0.20f,0.00f);//top left
        		glVertex3f(-1.28f,-0.16f,0.0f);//top right
        		glVertex3f(-1.09f,-0.46f,0.00f);//bottom right
        		glVertex3f(-1.15f,-0.50f,0.0f);//bottom left
       			glEnd();
        		glutSwapBuffers();
			break;
		case GLUT_KEY_F3:
			zvuk=3;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  			glBegin(GL_QUADS);
   	 		glColor3f(1, 1, 1);

   	 		// koordinate točaka pozadinskog pravokutnika (upside down)
    			glTexCoord2f(0, 1); glVertex3f(-2, -1, 0);
    			glTexCoord2f(1, 1); glVertex3f(2, -1, 0);
    			glTexCoord2f(1, 0); glVertex3f(2, 1, 0);
    			glTexCoord2f(0, 0); glVertex3f(-2, 1, 0);

    			/* crtanje cetverokuta */
  			glEnd();
   			glColor3f(1.0,0.0,0.0);
     			glBegin(GL_QUADS);
         		//prva linija prednja lijeva strana
        		glVertex3f(-1.22f, -0.20f, 0.0f); // top left
        		glVertex3f(-1.15f,-0.16f, 0.0f); // top right 
        		glVertex3f(-1.03f, -0.36f, 0.0f); // bottom right
        		glVertex3f(-1.1f, -0.40f, 0.0f); // bottom left

        		glColor3f(1.0,0.35,0.35);
        		//druga linija prednja lijeva strana
        		glVertex3f(-1.35f,-0.20f,0.00f);//top left
        		glVertex3f(-1.28f,-0.16f,0.0f);//top right
        		glVertex3f(-1.09f,-0.46f,0.00f);//bottom right
        		glVertex3f(-1.15f,-0.50f,0.0f);//bottom left

        		glColor3f(1.0,0.50,0.50);
        		//treca linija prednja lijeva strana
        		glVertex3f(-1.48f,-0.21f,0.0f);//top left
        		glVertex3f(-1.40f,-0.16f,0.0f);//top right
        		glVertex3f(-1.14f,-0.55f,0.0f);//bottom right
        		glVertex3f(-1.22f,-0.60f,0.0f);//bottom left

        		glEnd();
       			glutSwapBuffers();
			break;
		case GLUT_KEY_F4:
			zvuk=0;
			display();
			break;
		}


}

/*! glut display callback funkcija. Svaki put kad treba iscrtati prozor, glut poziva ovu funkciju
*/
void display(){
  cerr << "display callback" << endl;

  /* ocisti buffer boje (sve postaje crno) */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* nacrtaj četverokute */
  glBegin(GL_QUADS);   

    // postavi boju u bijelu
    glColor3f(1, 1, 1);

    // koordinate svih točaka (upside down)
    glTexCoord2f(0, 1); glVertex3f(-2, -1, 0);
    glTexCoord2f(1, 1); glVertex3f(2, -1, 0);
    glTexCoord2f(1, 0); glVertex3f(2, 1, 0);
    glTexCoord2f(0, 0); glVertex3f(-2, 1, 0);

  /* završeno crtanje četveroktua */
  glEnd();

  /* zamijeni buffere (jer u jednom crta, a drugi prikazuje na ekranu) */
  glutSwapBuffers();
}

/* glut reshape callback funkcija. Poziva se kad se mijenja dimenzija prozora
*/
void reshape(int width, int height)
{
  cerr << "reshape callback" << endl;

  /* reci OpenGL-u da želimo pravokutnik velicine prozora
  */
  glViewport(0,0,width,height);

  /* prebacivanje na matricu projekcije */
  glMatrixMode(GL_PROJECTION);

  /* očisti matricu projekcije */
  glLoadIdentity();

  /* postavi pogled kamere (orthographic projection with 4x4 unit square canvas) */
  glOrtho(-2,2,-2,2,2,-2);

  /* prebaci natrag na matricu modela */
  glMatrixMode(GL_MODELVIEW);

}

/* ovo se poziva kad racuanlo nema nista za raditi (kad nista novo ne crtamo nego samo cekamo neki event) */
void idle() 
{
    // display();     // call display callback, useful for animation
} 
