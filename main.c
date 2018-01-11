#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <GL/glut.h>

#define DODATAK 0.25 //duzina za koju se mrdaju kuglice svakim redisplejem
#define POCETAK 41 //y koordinata od koje krece pojavljivanje loptica
#define MAXR 40 //maximalna pozicija kuglice na y-osi pre crtanja nove na istu zicu
#define TOLERANCIJA 0.1 //tolerancija kasnjenja/ranjenja pojavljivanja kuglice
static float bkk = 10; //brzina kretanja kuglica

typedef struct{
	float* y; //dinamicki alociran niz kuglica na zici
		  //(ima brNota polja jer je to maksimalni br kuglica na jednoj zici)
	int n; //redni broj naredne kuglice za crtanje na zicu
	int m; //redni broj poslednje uklonjene kuglice sa zice
} zica;

static char velicinaProzora; // o/p -> ceo ekran/deo ekrana
static char mod; // e/m/h -> easy/meadium/hard
static char stanjeIgre; //m/i/p -> meni/igra/pauza 
static char rezultat; // g/t/z/p -> gubitak/traje/zavrsnica/pobeda

static zica pomeraj[5]; //podaci o svakoj zici

//promenljive za racunanje vremenskih intervala
static double pocetakIgre; //vreme pocetka igre
static double pauza; //merac trajanja pauze
static int sledecaKuglica; //redni broj kuglice cije se vreme sledece ceka

//podaci iz datoteke
static double *nizPesme; //niz vremena kada treba da se pojavi kruzic
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

	if(stanjeIgre == 'i'){ //igra
		//postavljanje osvetljenja
		GLfloat pozicija[] = { 0, -11, 4.5, 0 };
		glLightfv(GL_LIGHT0, GL_POSITION, pozicija);

		//postavljanje osobina materijala
		GLfloat presijavanje[] = { 20 };
		glMaterialfv(GL_FRONT, GL_SHININESS, presijavanje);
	
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

		for(int i = 0; i < 5; i++){
			for(int j = pomeraj[i].m; j < pomeraj[i].n; j++)
					loptica(pomeraj[i].y[j], i+1);
		}

	} else if(stanjeIgre == 'm'){ //meni
		//postavljanje kamere
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0,0,5,
			  0,0,0,
			  0,1,0);



	} else if(stanjeIgre == 'p'){ //pauza
		//stopira se vreme za pesmu
	}

	glutSwapBuffers();
}

void dodaj(float d, int i){
	//ako nema kuglice na zici
	if(pomeraj[i].n == 0 || pomeraj[i].n == pomeraj[i].m)
		return;

/*samo za probu pomeraj[i].m = pomeraj[i].n - 5 > 0 ? pomeraj[i].n - 5 : 0;*/
	for(int j = pomeraj[i].m; j < pomeraj[i].n; j++){
		pomeraj[i].y[j] = pomeraj[i].y[j] - d;
		if(pomeraj[i].y[j] < -2)
			rezultat = 'g';
	}
}

float poslednja(int i){
	//ako nema nijedne neuklonjene kuglice, vraca se sigurno manji br
	if(pomeraj[i].n == pomeraj[i].m || pomeraj[i].n == 0)
		return MAXR - 1;

	return pomeraj[i].y[pomeraj[i].n-1];
}

static void on_timer(int value)
{
	//provera da li je poziv funkcije nameran
	if (value != 0)
		return;

	//sve kuglice se priblizavaju
	for(int i = 0; i < 5; i++){
		dodaj(DODATAK, i);
	}
	
	//ako ima neiscrtanih kuglica 
	if(sledecaKuglica < brNota && rezultat == 't'){
		double tekuceVreme;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		tekuceVreme = tv.tv_sec + 0.000001 * tv.tv_usec 
			     - pocetakIgre - pauza;

		//ako je vreme za novu kuglicU
		if(abs(tekuceVreme - nizPesme[sledecaKuglica]) < TOLERANCIJA){
			//...random izaberi jednu zicu i dodaj je
			int zica = rand() % 5;
			int i;
			//ako je poslednja loptica na toj zici previsoko biramo sledecu
			for(i = 0; i < 5; i++){
				if(poslednja((zica + i) % 5) < MAXR) {
					zica = (zica+i) % 5;
					break;
				}
			}
			//ako nijedna zica nije odgovarala, preskacemo notu
			//ako je neka zica odgovarala podesavamo pomeraj
			if(i != 5){
				//ako nije vec prosao planiran broj kuglica
				if(pomeraj[zica].n < brNota){
					pomeraj[zica].y[pomeraj[zica].n] = POCETAK;
					pomeraj[zica].n++;

					//uvecavanje brojaca kuglica
					sledecaKuglica++;
				}
			}
	
		//ako je prosao brzi voz za narednu kuglicu
		} else if(tekuceVreme - nizPesme[sledecaKuglica] > 0.1){
			sledecaKuglica++;
		}
	} else if(sledecaKuglica >= brNota){
		//sacekaj da poslednja kuglica bude uklonjena
		//if()
		//rezultat = 'z';
		//i onda kreni sa pomeranjem slike (zavrsnica deo u displeju)
	}
	
	//iscrtavanje prozora
	glutPostRedisplay();

	//provera je li kraj igre:
	//ako je igra predjena
	if(rezultat == 'p'){
		//dodati funkciju za proglasavanje pobede
		stanjeIgre = 'm';
		sleep(5);
	}

	//ako je igra izgubljena
	if(rezultat == 'g'){
		//dodati funkciju za proglasavanje poraza
		stanjeIgre = 'm';
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
		//dealocirati memoriju iz key=13
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
/*dealocirati!*/		pomeraj[i].y = calloc(sizeof (float), brNota);
				//ovo treba dealocirati pri izlasku 
				//  iz igre i ulasku u meni
				//u slucaju izlaska iz cele igrice 
				//  bice automatski dealocirana memorija
				pomeraj[i].n = 0;
				pomeraj[i].m = 0;
			}
			rezultat = 't';
			//ucitavanje nizPesme.txt
			FILE* f = fopen("nizPesama.txt", "r");
			if(f == NULL){
				printf("problem sa datotekom nizPesme\n");
				exit(1);
			}
			if(mod == 'e'){
				fscanf(f, "%d\n", &brNota);
/*dealocirati!*/		nizPesme = calloc(sizeof(double), brNota);
				if(nizPesme == NULL){
					printf("greska sa alokacijom memorije");
					exit(1);
				}
				for(int i = 0; i < brNota; i++)
					fscanf(f, "%lf\n", nizPesme + i);
			}
			fclose(f);

			//inicijalizacija pocetnog vremena
			struct timeval pom;
			gettimeofday(&pom, NULL);
			pocetakIgre = pom.tv_sec + 0.000001 * pom.tv_usec;
			pauza = 0;

			//sid za random biranje zice
			srand(pocetakIgre);

			//momentalni poziv funkcije
			glutTimerFunc(0.1, on_timer, 0);
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
