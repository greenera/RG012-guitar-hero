#include <unistd.h>
#include <GL/glut.h>


static char velicinaProzora; //'o' za ceo ekran, 'p' inace
static char mod; // e/m/h -> easy/meadium/hard 

static void on_reshape(int sirina, int visina){
	//namestanje viewporta
	glViewport(0, 0, sirina, visina);

    	//namestanje projekcije
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float)sirina/visina, 1, 50);
}

void loptica(int pomeraj, int linija){
	//inicijalizuju se koordinate
	int x, y, z = 0;

	//ako je easy mod sve su kuglice razlicite boje, inace iste
	GLfloat ambijentalno[] = {0,0,0,1};
	if(mod == 'e'){
		switch(linija){
			case 1: 
				x = -8;
				ambijentalno[0] = 1; 
				break;
			case 2: 
				x = -4;
				ambijentalno[1] = 1; 
				ambijentalno[0] = 1; 
				break;
			case 3: 
				x =  0;
				ambijentalno[1] = 1; 
				break;
			case 4: 
				x =  4;
				ambijentalno[0] = 1; 
				ambijentalno[2] = 1; 
				break;
			case 5: 
				x =  8;
				ambijentalno[2] = 1; 
				break;
		}
	} else{
		ambijentalno[2] = 1;

		switch(linija){
			case 1: x = -8; break;
			case 2: x = -4; break;
			case 3: x =  0; break;
			case 4: x =  4; break;
			case 5: x =  8; break;
		}
	}

	y = pomeraj;

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambijentalno);
	glPushMatrix();
	glTranslatef(x, y, z);
	glutSolidSphere(1, 50,50);
	glPopMatrix();
}

static void on_display(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Postavljanje osvetljenja
	GLfloat pozicija[] = { 0, -11, 4.5, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, pozicija);

	//postavljanje presijavanja
	GLfloat presijavanje[] = { 20 };
	glMaterialfv(GL_FRONT, GL_SHININESS, presijavanje);

//proba
	GLfloat no_material[] = { 0.1, 0.1, 0.1, 0 };
	GLfloat material_ambient[] = { 1, 1, 1, 1 };
	GLfloat material_diffuse[] = { 0.3, 0.3, 0.3, 1 };
	GLfloat material_specular[] = { 0.8, 0.8, 0.8, 1 };

        glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, no_material);
//kraj probe


	//postavljanje kamere
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, -10,5,
		  0,  2,0,
		  0,  0,1);

	//craju se zice
	glColor3f(0, 1, 0);
	for(int x = -8; x <9; x = x+4){
	    glBegin(GL_LINE_STRIP);
		glVertex3f(x, -2, 0);
		glVertex3f(x, 40, 0);
	    glEnd();
	}

//test
	for(int i = 1; i < 6; i++)
		loptica(15,i);



	glutSwapBuffers();
}
static void on_keyboard(unsigned char key, int x, int y){
	if(key == 27){
		//za pocetak se samo izlazi
		exit(0);

		//u daljem radu se pravi meni za izbor iz cega se izlazi:
			//iz cele igrice
			//iz trenutne igre
		//izborIzlaza();
	} else if(key == 'o' || key == 'O'){
		//proverava se je li vec bilo to slovo
		if(velicinaProzora != 'o'){
			//ako nije radi se resize
			velicinaProzora = 'o';
			glutFullScreen();
		}
	} else if(key == 'p' || key == 'P'){
		//proverava se je li vec bilo to slovo
		if(velicinaProzora != 'p'){
			//ako nije radi se resize
			velicinaProzora = 'p';
			glutReshapeWindow(1000,600);
		}
	}

}

int main(int argc, char** argv){
	// inicijalizacija gluta
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

	//pravi se prozor koji prekriva ceo ekran
	glutCreateWindow("GUITAR HERO");

	//registruju se callback funkcije
	glutDisplayFunc(on_display);
	glutKeyboardFunc(on_keyboard);
	glutReshapeFunc(on_reshape);

	//inicijalizuju se pocetne vrednosti
	velicinaProzora = 'o';
	glutFullScreen();
	glClearColor(0.1, 0.1, 0.1, 0);
	glEnable(GL_DEPTH_TEST);

	//difoltni je najlaksi nivo
	mod = 'e';

//za probu
    GLfloat ambient[] = { 0, 0, 0, 1 };
    GLfloat diffuse[] = { 1, 1, 1, 1 };
    GLfloat specular[] = { 1, 1, 1, 1 };
    GLfloat mambient[] = { 0.4, 0.4, 0.4, 1 };

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); //note: ostavi BACK!

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, mambient);
//kraj probnog dela --- za menjanje

	//pokrecemo glavnu petlju
	glutMainLoop();

	return 0;
}
