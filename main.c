#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>

#define duzina 0.1 //duzina za koju se mrdaju kuglice svakim redisplejem
#define MAXR 40 //maximalna pozicija kuglice na y-osi pre crtanja nove na istu zicu

typedef struct{
	float* y; //dinamicki alociran niz
	int n; //duzina niza
} kuglice;

//tekuceVreme <= nizPesme[sledecaKuglica

static char velicinaProzora; //'o' za ceo ekran, 'p' inace
static char mod; // e/m/h -> easy/meadium/hard 
static char rezultat; // g/t/p gubitak/traje/pobeda
static kuglice *pomeraj[5]; //za koliko se pomera svaka kuglica
static float bkk = 500; //brzina kretanja kuglica
static char stanjeIgre; //m za meni, i za igru, p za pauzu 
static time_t pocetakIgre; //vreme pocetka igre
static time_t tekuceVreme; //trenutni merac vremena proteklog od pocetka igre
static int sledecaKuglica; //redni broj kuglice cije se vreme sledece ceka
static time_t *nizPesme; //niz vremena kada treba da se pojavi kruzic
static int brNota; //koliko ukupno ima kruzica za izlazak

static void on_reshape(int sirina, int visina){
	//namestanje viewporta
	glViewport(0, 0, sirina, visina);

    	//namestanje projekcije
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float)sirina/visina, 1, 50);
}

void loptica(int y, int linija){
	//inicijalizuju se koordinate
	int x, z = 0;

	//ako je easy mod sve su kuglice razlicite boje, inace iste
	GLfloat ambijentalno[] = {0,0,0,1};
	if(mod == 'e'){
		switch(linija){
			case 1: 
				x = -8;
				ambijentalno[1] = 1; 
				ambijentalno[0] = 1;
				break;
			case 2: 
				x = -4;
				ambijentalno[0] = 0.5; 
				ambijentalno[2] = 1; 
				break;
			case 3: 
				x =  0;
				ambijentalno[1] = 1; 
				break;
			case 4: 
				x =  4;
				ambijentalno[0] = 1;
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

	//postavljanje osobina materijala
	GLfloat em[] = { 0.1, 0.1, 0.1, 0 };
        glMaterialfv(GL_FRONT, GL_EMISSION, em);

	GLfloat amb[] = { 1, 1, 1, 1 };
        glMaterialfv(GL_FRONT, GL_AMBIENT, amb);

	GLfloat dif[] = { 0.3, 0.3, 0.3, 1 };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

	GLfloat spec[] = { 0.8, 0.8, 0.8, 1 };
        glMaterialfv(GL_FRONT, GL_SPECULAR, spec);

	//postavljanje kamere
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, -10,5,
		  0,  2,0,
		  0,  0,1);

	//crtanje zica
	for(int x = -8; x <9; x = x+4){
	    glBegin(GL_LINE_STRIP);
		glVertex3f(x, -2, 0);
		glVertex3f(x, 40, 0);
	    glEnd();
	}

//test
	for(int i = 1; i < 6; i++)
		loptica(41,i);



	glutSwapBuffers();
}

void dodaj(int d, kuglice* kugliceJedneZice){
	if(kugliceJedneZice->n == 0)
		return;

	for(int i = 0; i < kugliceJedneZice->n; i++){
		kugliceJedneZice->y[i] -= d;
		if(kugliceJedneZice->y[i] < -2)
			rezultat = 'g';
	}
}

float poslednja(kuglice* nizKuglica){
	if(nizKuglica->n == 0)
		return MAXR - 1;
	float max = nizKuglica->y[0];
	for(int i = 1; i < nizKuglica->n; i++){
		if(nizKuglica->y[i] > max)
			max =  nizKuglica->y[i];
	}

	return max;
}

static void on_timer(int value)
{
	//provera da li je poziv funkcije nameran
	if (value != 0)
		return;

	//sve kuglice se priblizavaju
	for(int i = 0; i < 5; i++){
		dodaj(duzina, pomeraj[i]);
	}

	//ako je vreme za novu kuglicu...
	if(tekuceVreme <= nizPesme[sledecaKuglica]){
		//...random izaberi jednu zicu i dodaj je
		int zica = rand() % 5;
		int i;
		for(i = 0; i < 5; i++){
			if(poslednja(pomeraj[(zica + i) % 5]) > MAXR)
				zica= (zica+1) % 5;
			else 
				break;
		}
		//ako nijedna zica nije odgovarala, preskacemo notu
		//ako je neka zica odgovarala podesavamo pomeraj
		if(i != 5){
			pomeraj[zica]->y = realloc(pomeraj[zica]->y, (pomeraj[zica]->n+1)*sizeof(float));
			pomeraj[zica]->n++;
		}

		//ako ima jos kuglica za crtanje, uvecavamo brojac kuglica
		if(sledecaKuglica != brNota)
			sledecaKuglica++;
	}
	
	//iscrtavanje prozora
	glutPostRedisplay();

	//provera je li kraj igre:
	//ako je igra predjena
	if(rezultat == 'p'){
		stanjeIgre = 'm';
		//dodati funkciju za proglasavanje pobede
		sleep(5);
	}

	//ako je igra izgubljena
	if(rezultat == 'g'){
		stanjeIgre = 'm';
		//dodati funkciju za proglasavanje poraza
		sleep(5);
	}

	//ako je igra pauzirana -> stanje igre je vec promenjeno

	//ako nije kraj igre nastavlja se sa pomeranjem kuglica 
	if (stanjeIgre == 'i')
		glutTimerFunc(bkk, on_timer, 0);
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
	} else if(key == 13){
		//igra krece jedino u slucaju da je enter pritisnuto iz menija
		if(stanjeIgre == 'm'){
			//ulazi se u mod za igru
			stanjeIgre = 'i';
			//inicijalizacija pomeraja - nema nijedne loptice
			for(int i = 0; i < 5; i++){
				pomeraj[i] = malloc(sizeof (kuglice));
				//ovo treba dealocirati pri izlasku 
				//  iz igre i ulasku u meni
				//u slucaju izlaska iz cele igrice 
				//  bice automatski dealocirana memorija
				pomeraj[i]->n = 0;
				(*pomeraj[i]).y = NULL;
			}
			rezultat = 't';
			pocetakIgre = time(NULL);
			srand(pocetakIgre);
			glutTimerFunc(bkk, on_timer, 0);
		}
	}

}

int main(int argc, char** argv){
	// inicijalizacija gluta
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

	//pravi se prozor koji prekriva ceo ekran
	glutCreateWindow("GUITAR HERO");
	glutFullScreen();

	//registruju se callback funkcije
	glutDisplayFunc(on_display);
	glutKeyboardFunc(on_keyboard);
	glutReshapeFunc(on_reshape);

	//inicijalizuju se pocetne vrednosti
	velicinaProzora = 'o';
	glClearColor(0.1, 0.1, 0.1, 0);
	mod = 'e'; //difoltni je najlaksi nivo
	stanjeIgre = 'm'; //igra krece iz menija
	sledecaKuglica = 0; //krecemo od prve kuglice za prikazivanje

	//ukljucivanje potrebnih specifikacija
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	//podesavanje osobina osvetljenja
	GLfloat ambient[] = { 0.1, 0.1, 0.1, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	GLfloat diffuse[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	GLfloat specular[] = { 0.7, 0.7, 0.7, 1 };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	GLfloat mambient[] = { 0.5, 0.5, 0.5, 1 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, mambient);

	//pokrecemo glavnu petlju
	glutMainLoop();

	return 0;
}
