#include <unistd.h>
#include <GL/glut.h>


static char velicinaProzora; //'o' za ceo ekran, 'p' inace

static void on_reshape(int sirina, int visina){
	//namestanje viewporta
	glViewport(0, 0, sirina, visina);

    	//namestanje projekcije
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30, (float)sirina/visina, 1, 30);
}

static void on_display(void){
	glClear(GL_COLOR_BUFFER_BIT);

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
			glutReshapeWindow(800,600);
		}
	}

}

int main(int argc, char** argv){
	// inicijalizacija gluta
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	//pravi se prozor koji prekriva ceo ekran
	glutCreateWindow("GUITAR HERO");

	//registruju se callback funkcije
	glutDisplayFunc(on_display);
	glutKeyboardFunc(on_keyboard);
	glutReshapeFunc(on_reshape);

	//inicijalizuju se pocetne vrednosti
	velicinaProzora = 'o';
	glutFullScreen();
	glClearColor(0, 1, 0, 0);
	glEnable(GL_DEPTH_TEST);

	//pokrecemo glavnu petlju
	glutMainLoop();

	return 0;
}
