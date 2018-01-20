#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "SOIL.h"
#include <GL/glut.h>

#define DODATAK 0.25 //duzina za koju se mrdaju kuglice svakim redisplejem
#define POCETAK 41 //y koordinata od koje krece pojavljivanje loptica
#define MAXR 40 //maximalna pozicija kuglice na y-osi pre crtanja nove na istu zicu
#define TOLERANCIJA 0.05 //tolerancija kasnjenja/ranjenja pojavljivanja kuglice
static float bkk = 10; //brzina kretanja kuglica

typedef struct{
	float* y; //dinamicki alociran niz kuglica na zici
		  //(ima brNota polja jer je to maksimalni br kuglica na jednoj zici)
	int n; //redni broj naredne kuglice za crtanje na zicu
	int m; //redni broj poslednje uklonjene kuglice sa zice
} zica;

typedef struct{
	double* tipkanje; //dinamicki alociran niz koji cuva vreme uklanjanja kuglice
	int n; //redni broj narednog slobodnog polja u nizu
} skor;

typedef struct{
	char* struna; //dinamicki alociran niz kuglica na zicama d/f/g/h/j
	int n; //redni broj narednog slobodnog polja u nizu
	int m; //redni broj poslednje uklonjene kuglice
} redosled;

GLuint slike[12];

static char velicinaProzora; // o/p -> ceo ekran/deo ekrana
static char mod; // e/m/h -> easy/meadium/hard
static char stanjeIgre; //m/i/p -> meni/igra/pauza 
static char rezultat; // g/t/z/p -> gubitak/traje/zavrsnica/pobeda
static char brIgraca; // 1/2 -> jedan/dva
static char igraci; // 1/2/z -> prvi/drugi/zajedno

static zica pomeraj[5]; //podaci o svakoj zici
static redosled redosledZica; //redosled zica pojavljivanja kuglica

//promenljive za racunanje vremenskih intervala
static double pocetakIgre; //vreme pocetka igre
static int sledecaKuglica; //redni broj kuglice cije se vreme sledece ceka
static skor pogodak; //cuva vreme pogodjenih kuglica

//jump scare
static double jumpScareVreme;
static char jumpScare; //0/1/2 -> nije vreme/slika1/slika2

//pauza
static int sid; //subwindow id
static int wid; //window id (id glavnog prozora)
static double pocetakPauze; //vreme pri pocetku pauze
static double pauza; //merac trajanja pauze
static char promena; // 0/1 u zavisnosti da li treba menjati vidljivost potprozora

//podaci iz datoteke
static double *nizPesme; //niz vremena kada treba da se pojavi kruzic
static int brNota; //koliko ukupno ima kruzica za izlazak

//muzika
static char muzika; // u/i -ukljucena/iskljucena

//imena igraca
static char* ime1;
static char* ime2;

static void on_reshape(int sirina, int visina){
	//namestanje viewporta
	glViewport(0, 0, sirina, visina);

    	//namestanje projekcije
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float)sirina/visina, 1, 50);
}

static void on_timer(int value);

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

float abso(double a, double b){
	if(a > b)
		return a - b;
	else if(b > a)
		return b - a;
	else
		return 0;
}


void racunanjePoena(){
	float poeni = 0;

	int faktor;
	double prioritet; 

	switch(mod){
		case 'e': faktor = 1000; prioritet = 0.5; break;
		case 'm':
			//u medium modu se ne racuna skor,
			//samo se opisuje nacin igre u zavisnosti
			//je li stiglo do jump scare-a ili ne
			if(rezultat == 'g')
				//promeniti printf
				printf("~nedovoljno duga igra za racunanje skora~\n");
			return;
			break;
		case 'h': faktor = 500; prioritet = 0.75; break;
	}

	for(int i = 0; i < pogodak.n; i++)
		poeni += abso(nizPesme[i]+prioritet, pogodak.tipkanje[i]);
	
	if(poeni != 0)
		poeni = faktor/poeni;
	else
		poeni = 0;

	printf("%f", poeni);

	//cuvanje skora
	FILE* f = fopen("skor.txt", "a");
	if(f == NULL){
		printf("nemoguce upisati skor");
		return;
	}
	
	fprintf(f, "%.3f", poeni);
	if(brIgraca == 1 && igraci == '1'){
		if(ime1 != NULL)
			fprintf(f, " %s", ime1);
		else
			fprintf(f, " anonimus1");
	} else if(brIgraca == 1 && igraci == '2'){
		if(ime2 != NULL)
			fprintf(f, " %s", ime2);
		else
			fprintf(f, " anonimus1");
	}else if(brIgraca == 2){
		if(ime1 != NULL)
			fprintf(f, " %s i", ime1);
		else 
			fprintf(f, " anonimus1 i");
		if(ime2 != NULL)
			fprintf(f, " %s\n", ime2);
		else
			fprintf(f, " anonimus2");
	}
	fprintf(f, "\n");
	fclose(f);
}

void iscrtavanjeSlika(void){

	GLfloat amb[] = { 1, 1, 1, 0.5 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);


	int rbSlike = 0;
	for(int i = -24; i < 24; i = i + 8){
		glPushMatrix();	
		    glEnable(GL_TEXTURE_2D);
		    glBindTexture(GL_TEXTURE_2D, slike[rbSlike++]);
		    glBegin(GL_QUADS);
	  		glTexCoord2f(0,0); glVertex3f(i, 40, 0);
			glTexCoord2f(1,0); glVertex3f(i+8, 40, 0);
			glTexCoord2f(1,1); glVertex3f(i+8, 40, 10);
			glTexCoord2f(0,1); glVertex3f(i, 40, 10);
	  	    glEnd();
		glPopMatrix();
	}

}

void iskacucaSlika(char slika){

	//trazimo redni broj slike na osnovu argumenta funkcije
	int rbSlike;
	float x, y, z1, z2;
	switch(slika){
		case('j'): 
			rbSlike = 6;
			x = 4; y = -3; z1 = 4; z2 = -4;
			break;
		case('y'): 
			rbSlike = 7;
			x = 4; y = -3; z1 = 4; z2 = -4;
			break;
		case('p'): 
			rbSlike = 10; 
			x = 6; y = -1; z1 = 6; z2 = 1;
			break;
		case('g'): 
			rbSlike = 11; 
			x = 6; y = -1; z1 = 6; z2 = 0;
			break;
	}

	glPushMatrix();	
	    glEnable(GL_TEXTURE_2D);
    	    glBindTexture(GL_TEXTURE_2D, slike[rbSlike]);
	    glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex3f(-1*x, y, z2);
		glTexCoord2f(1,0); glVertex3f(   x, y, z2);
		glTexCoord2f(1,1); glVertex3f(   x, y, z1);
		glTexCoord2f(0,1); glVertex3f(-1*x, y, z1);
	    glEnd();
	glPopMatrix();
}

void inicijalizacijaProstora(void);

static void on_display(void){
	//ako je potrebno menjati vidljivost prozora
	if(promena){
		if(stanjeIgre != 'p'){
			glutSetWindow(sid);
			glutHideWindow();
			glutSetWindow(wid);
			glutTimerFunc(bkk, on_timer, 0);
			//posle ovoga je potrebno pritisnuti bilo koje 
			//dugme da bi se aplikacija nastavila			
		} else {
			glutSetWindow(sid);
			glutShowWindow();
		}
		promena = 0;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(stanjeIgre == 'i'){ //igra
		//postavljanje osvetljenja
		GLfloat pozicija[] = { 0, -11, 4.5, 0 };
		glLightfv(GL_LIGHT0, GL_POSITION, pozicija);

		//postavljanje osobina materijala
		GLfloat presijavanje[] = { 20 };
		glMaterialfv(GL_FRONT, GL_SHININESS, presijavanje);
	
		GLfloat em[] = { 0.1, 0.1, 0.1, 1 };
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

		if(rezultat == 'g')
			iskacucaSlika('g'); //crtamo sliku gameover
		else if(rezultat == 'p')
			iskacucaSlika('p'); //crtamo sliku win
		else if(mod != 'm' || jumpScare == 0){
			iscrtavanjeSlika();

			//crtanje zica
			for(int x = -8; x <9; x = x+4){
			    glBegin(GL_LINE_STRIP);
				glVertex3f(x, -7, 0);
				glVertex3f(x, 40, 0);
			    glEnd();
			}

			//zica prekoracenja
			//GLfloat amb[] = { 1, 1, 1, 1 };
			amb[1] = 0;
			amb[2] = 0;
			glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
			glBegin(GL_LINE_STRIP);
			    glVertex3f(-8, -2, 0);
			    glVertex3f(8, -2, 0);
			glEnd();

			//zica prioriteta
			int daljina;
			switch(mod){
				case 'e': daljina = 11; break;
				case 'm': daljina = 7; break;
				case 'h': daljina = 4; break;
			}

			amb[1] = 0.8;
			amb[0] = 0.9;
			glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
			glBegin(GL_LINE_STRIP);
			    glVertex3f(-8, daljina, 0);
			    glVertex3f( 8, daljina, 0);
			glEnd();

			//crtanje kuglica
			for(int i = 0; i < 5; i++){
				for(int j = pomeraj[i].m; j < pomeraj[i].n; j++)
						loptica(pomeraj[i].y[j], i+1);
			}
			

		} else { //crtamo slike jumpscare 1 i 2
			if(jumpScare == 1)
				iskacucaSlika('y');
		    	else
				iskacucaSlika('j');
		}
	} else if(stanjeIgre == 'm'){ //meni
		//postavljanje kamere
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0,0,10,
			  0,0,0,
			  0,1,0);

		GLfloat pozicija[] = { 0, 0, 10, 1 };
		glLightfv(GL_LIGHT0, GL_POSITION, pozicija);

		GLfloat amb[] = { 1, 1, 1, 1 };
		glMaterialfv(GL_FRONT, GL_AMBIENT, amb);

		//prikaz instrukcija za meni:

		////prikazi sliku menija
		glPushMatrix();			
		    glEnable(GL_TEXTURE_2D);
		    glBindTexture(GL_TEXTURE_2D, slike[9]);
 		    glBegin(GL_QUADS);
			glTexCoord2f(0,0); glVertex3f(-7.5,-3, -0.3);
			glTexCoord2f(1,0); glVertex3f( 7.5,-3, -0.3);
			glTexCoord2f(1,1); glVertex3f( 7.5, 3, -0.3);
			glTexCoord2f(0,1); glVertex3f(-7.5, 3, -0.3);
		    glEnd();
		glPopMatrix();

		////prikazi kuglice:

		//////kuglica za pokazivanje moda igre
		float pom;
 		switch(mod){
			case('e'): pom = -1.25; break;
			case('m'): pom =  2.25; break;
			case('h'): pom =  5.5; break;
		}
		glPushMatrix();
		    glTranslatef(pom, 0.9,0.5);
		    glutSolidSphere(0.25,30,30);
		glPopMatrix();

		//////kuglica za pokazivanje aktivnih igraca
 		switch(igraci){
			case('1'): pom = -1.25; break;
			case('z'): pom =  2.25; break;
			case('2'): pom =  5.4; break;
		}

		glPushMatrix();
		    glTranslatef(pom, -0.9,0.5);
		    glutSolidSphere(0.25,30,30);
		glPopMatrix();

	} else if(stanjeIgre == 'p'){ //pauza
		//postavljanje kamere
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0,0,10,
			  0,0,0,
			  0,1,0);

		//GLfloat pozicija[] = { 0, 0, 10, 1 };
		//glLightfv(GL_LIGHT0, GL_POSITION, pozicija);

		//prikazi sliku menija
		glPushMatrix();			
		    glEnable(GL_TEXTURE_2D);
		    glBindTexture(GL_TEXTURE_2D, slike[10]);
 		    glBegin(GL_QUADS);
/*			glTexCoord2f(0,0); glVertex3f(-7.5,-3, -0.3);
			glTexCoord2f(1,0); glVertex3f( 7.5,-3, -0.3);
			glTexCoord2f(1,1); glVertex3f( 7.5, 3, -0.3);
			glTexCoord2f(0,1); glVertex3f(-7.5, 3, -0.3);
*/
			glTexCoord2f(0,0); glVertex3f(-7.5,-3, -0.3);
			glTexCoord2f(0,1); glVertex3f(-7.5, 3, -0.3);
			glTexCoord2f(1,1); glVertex3f( 7.5, 3, -0.3);
			glTexCoord2f(1,0); glVertex3f( 7.5,-3, -0.3);
		    glEnd();
		glPopMatrix();
	}

	glutSwapBuffers();
}

void dodaj(float d, int i){
	//ako nema kuglice na zici
	if(pomeraj[i].n == 0 || pomeraj[i].n == pomeraj[i].m)
		return;

/*samo za probu 
pomeraj[i].m = pomeraj[i].n - 5 > 0 ? pomeraj[i].n - 5 : 0;*/
	for(int j = pomeraj[i].m; j < pomeraj[i].n; j++){
		pomeraj[i].y[j] = pomeraj[i].y[j] - d;
		if(pomeraj[i].y[j] < -2){
			rezultat = 'g';
			racunanjePoena();
			glutPostRedisplay();
		}
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

		if(mod == 'm' && tekuceVreme - jumpScareVreme < 1 &&
				 tekuceVreme - jumpScareVreme > -1){
			jumpScare = (int)(tekuceVreme) %2 + 1;

			if(tekuceVreme - jumpScareVreme > 7)
				rezultat = 'p';
		} else


		//ako je vreme za novu kuglicu
		if(tekuceVreme - nizPesme[sledecaKuglica] < TOLERANCIJA 
		&& tekuceVreme - nizPesme[sledecaKuglica] > -TOLERANCIJA){
			//...random izaberi jednu zicu i dodaj je
			int zica = rand() % 5;
			int i;
			//ako je poslednja loptica na izabranoj
			//zici previsoko biramo sledecu
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

					//namestanje niza za uklanjanje kuglica
					char pom;
					switch(zica){
						case 0: pom = 'd'; break;
						case 1: pom = 'f'; break;
						case 2: pom = 'g'; break;
						case 3: pom = 'h'; break;
						case 4: pom = 'j'; break;
					}
					redosledZica.struna[redosledZica.n] = pom;
					redosledZica.n++;
				}
			}
	
		//ako je prosao brzi voz za narednu kuglicu preskace se
		} else if(tekuceVreme - nizPesme[sledecaKuglica] > TOLERANCIJA){
			sledecaKuglica++;
		}
	} else if(sledecaKuglica >= brNota){
		if(redosledZica.m == brNota){
			rezultat = 'p';
		}
		//i onda kreni sa pomeranjem slike (zavrsnica deo u displeju)
	}
	
	//iscrtavanje prozora
	glutPostRedisplay();

	//provera je li kraj igre:
	//ako je igra predjena
	if(rezultat == 'p'){
		sleep(5);
		stanjeIgre = 'm';
	}

	//ako je igra izgubljena
	if(rezultat == 'g'){
		sleep(5);
		stanjeIgre = 'm';
	}

	//ako je igra pauzirana -> stanje igre je vec promenjeno

	//ako nije kraj igre nastavlja se sa pomeranjem kuglica 
	if (stanjeIgre == 'i')
		glutTimerFunc(bkk, on_timer, 0);
}

static void on_keyboard2(unsigned char key, int x, int y){
	if(key == 27 || key == 'i'){ //nazad u igru		
		stanjeIgre = 'i';
		promena = 1;

		//podesavanje vremena pauze
		struct timeval pom;
		gettimeofday(&pom, NULL); 
		pauza += pom.tv_sec + 0.000001 * pom.tv_usec - pocetakPauze;

		glutPostRedisplay();
	
	} else if(key == 'm'){ //nazad u meni
		
		//dealokacija memorije
		free(nizPesme);
		for(int i = 0; i < 5; i++)
				free(pomeraj[i].y);
		free(pogodak.tipkanje);
		free(redosledZica.struna);
		
		stanjeIgre = 'm';
		promena = 1;
		glutPostRedisplay();
	}
}

static void on_keyboard(unsigned char key, int x, int y){
	if(key == 27){
		if(stanjeIgre == 'm')
			exit(0);
		/*else if(stanjeIgre == 'p'){
		ovo stanje ima svoj prozor gde
		je obradjena reakcija na esc }*/		
		else if(stanjeIgre == 'i'){
			//ako je muzika ukljucena pauza nije, izlazi se iz programa
			if(muzika == 'u'){
				stanjeIgre = 'm';
				//gasenjeMuzike()
				glutPostRedisplay();
			} else{
				struct timeval pom;
				gettimeofday(&pom, NULL);
				pocetakPauze = pom.tv_sec + 0.000001 * pom.tv_usec;

				stanjeIgre = 'p';
				promena = 1; 
				glutPostRedisplay();
			}
		}
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

			//ucitavanje nizPesme.txt
			FILE* f = fopen("nizPesama.txt", "r");
			if(f == NULL){
				printf("problem sa datotekom nizPesme\n");
				exit(1);
			}

			if(mod == 'e' || mod == 'm' || mod == 'h'){
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

/*dealocirati!*/	pogodak.tipkanje = calloc(brNota, sizeof(double));
/*dealocirati!*/	redosledZica.struna = calloc(brNota, sizeof(char));
			redosledZica.n = 0;
			redosledZica.m = 0;
			
			//inicijalizacija pocetnog vremena
			struct timeval pom;
			gettimeofday(&pom, NULL);
			pocetakIgre = pom.tv_sec + 0.000001 * pom.tv_usec;
			pauza = 0;
			sledecaKuglica = 0;

			//sid za random biranje zice
			srand(pocetakIgre);

			//pokretanje animacije
			glutTimerFunc(0.1, on_timer, 0);
		}
	} else if(strchr("dfghj", key) != NULL && brIgraca == 1 && stanjeIgre == 'i' && (rezultat == 't' || rezultat == 'z')){
		int pom;
		switch(key){
			case 'd': pom = 0; break;
			case 'f': pom = 1; break;
			case 'g': pom = 2; break;
			case 'h': pom = 3; break;
			case 'j': pom = 4; break;
		}
		if(redosledZica.struna[redosledZica.m] != key
		   || redosledZica.n == redosledZica.m){
			rezultat = 'g';
			racunanjePoena();
		} else{
			struct timeval pomt;
			gettimeofday(&pomt, NULL);

			pogodak.tipkanje[pogodak.n++] = pomt.tv_sec + 0.000001*pomt.tv_usec;
			redosledZica.m++;
			pomeraj[pom].m++;
		}
	} else if (strchr("asdjkl", key) != NULL && brIgraca == 2 && stanjeIgre == 'i' && (rezultat == 't' || rezultat == 'z')){
		int pom1;
		char pom2;
		switch(key){
			case 'a': pom1 = 0; pom2 = 'd'; break;
			case 's': pom1 = 1; pom2 = 'f'; break;
			case 'd': pom1 = 2; pom2 = 'g'; break;
			case 'j': pom1 = 2; pom2 = 'g'; break;
			case 'k': pom1 = 3; pom2 = 'h'; break;
			case 'l': pom1 = 4; pom2 = 'j'; break;
		}
		if(redosledZica.struna[redosledZica.m] != pom2
		   || redosledZica.n == redosledZica.m){
			rezultat = 'g';
			racunanjePoena();
		} else{
			struct timeval pomt;
			gettimeofday(&pomt, NULL);

			pogodak.tipkanje[pogodak.n++] = pomt.tv_sec + 0.000001*pomt.tv_usec;

			redosledZica.m++;
			pomeraj[pom1].m++;
		}
	} else if(stanjeIgre == 'm' && (key == 'e' || key == 'E')){
		mod = 'e';
		glutPostRedisplay();
	} else if(stanjeIgre == 'm' && (key == 'm' || key == 'M')){
		mod = 'm';
		glutPostRedisplay();
	} else if(stanjeIgre == 'm' && (key == 'h' || key == 'H')){
		mod = 'h';
		glutPostRedisplay();
	} else if(stanjeIgre == 'm' && key == '1'){
		igraci = '1';
		glutPostRedisplay();
	}else if(stanjeIgre == 'm' && key == '2'){
		igraci = '2';
		glutPostRedisplay();
	}else if(stanjeIgre == 'm' && (key == 'z' || key == 'Z')){
		igraci = 'z';
		glutPostRedisplay();
	}
}

int ucitajSlike(void){
	int brojac = 0;

	char* putanjeSlika[] = {"./slikeLinkinPark/bred.png",
				"./slikeLinkinPark/cester.png",
				"./slikeLinkinPark/sinoda.png",
				"./slikeLinkinPark/rob.png",
				"./slikeLinkinPark/han.png",
				"./slikeLinkinPark/phoenix.png",
				"./slikeLinkinPark/jumpscare1.png",
				"./slikeLinkinPark/jumpscare2.png",
				"./slikeLinkinPark/pauza.png",
				"./slikeLinkinPark/meni.png",
				"./slikeLinkinPark/win.png",
				"./slikeLinkinPark/gameover.png",};
	
	for(int i = 0; i < 12; i++){
		slike[i] = SOIL_load_OGL_texture(putanjeSlika[i],
						 SOIL_LOAD_AUTO,
						 SOIL_CREATE_NEW_ID,
						 SOIL_FLAG_INVERT_Y);

		if(slike[i])
			brojac++;
	}

	glFlush();

	return brojac;
}

void inicijalizacijaProstora(void){
	//ukljucivanje potrebnih specifikacija
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	//podesavanje osobina osvetljenja
	GLfloat ambient[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	GLfloat diffuse[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	GLfloat specular[] = { 0.7, 0.7, 0.7, 1 };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	GLfloat mambient[] = { 0.5, 0.5, 0.5, 1 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, mambient);
}

int main(int argc, char** argv){
	// inicijalizacija gluta
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

	//pravi se prozor koji prekriva ceo ekran
	glutInitWindowSize(1700, 900);
	glutInitWindowPosition(300, 700);
	wid = glutCreateWindow("GUITAR HERO");
	glutFullScreen();

	//registruju se callback funkcije
	glutDisplayFunc(on_display);
	glutKeyboardFunc(on_keyboard);
	glutReshapeFunc(on_reshape);

	//inicijalizuju se pocetne vrednosti
	velicinaProzora = 'o';
	promena = 1;
	glClearColor(1,1,1,1);
	mod = 'e'; //difoltni je najlaksi nivo
	stanjeIgre = 'm'; //igra krece iz menija
	jumpScareVreme = 15.132;
	jumpScare = 0;
	muzika = 'i'; //po difoltu je muzika iskljucena
	brIgraca = 1; //po difoltu 1, osim ako se ne unesu 2 imena
	igraci = '1';
	if(argc == 2){
		ime1 = argv[1];
	}
	else if(argc == 3){
		ime1 = argv[1];
		ime2 = argv[2];
		brIgraca = 2;
		igraci = 'z';
	}
	//ucitavanje slika
	if(12 != ucitajSlike()){
		printf("neke slike nisu uspesno ucitane\n");
		exit(1);
	}

	inicijalizacijaProstora();

	//pravimo prozor za pauzu
	glutInitWindowSize(500, 300);
	glutInitWindowPosition(300, 700);
	sid = glutCreateWindow("pauza");

	glutDisplayFunc(on_display);
	glutKeyboardFunc(on_keyboard2);
	glutReshapeFunc(on_reshape);
	inicijalizacijaProstora();

	//pokrecemo glavnu petlju
	glutMainLoop();

	return 0;
}
