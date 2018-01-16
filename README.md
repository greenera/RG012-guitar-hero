# RS012-guitar-hero
Guitar hero
## Uputstvo: 
<ul>
  <li>igra se pokrece sa: "./gh imeIgraca1 imeIgraca2" i ukoliko se imena ne unesu podrazumevana imena su anonimus1 i anonimus2</li>
  <li> 1/2/z - igra pod prvim imenom/drugim imenom/zajedno</li>
  <li> e/m/h - easy/medium/hard nivo</li>
  <li> o/p - fullscreen/manji prozor</li>
  <li> esc/enter - izlazak iz programa/pocetak igre</li>
  <li> tasteri za uklanjanje kuglica za dva igraca: 
    <li>a/s/d - prva/druga/treca zica</li>
    <li>j/k/l - treca/cetvrta/peta zica</li> </li>
  <li> tasteri za uklanjanje kuglica za jednog igraca: 
    <li>d/f/g/h/j - prva/druga/treca/cetvrta/peta zica</li> </li>
  <li>esc - taster za stopoiranje (dalje obajasnjenje prikazano je u prozoru)</li>
</ul>

## Opis:
<ul>
  <li>po pokretanju, nakon par sekundi inicijalizacije - kad ceo ekran pocrni, je prikazan meni za izbor igre </li>
  <li>ukoliko se igra u dva igraca oni igraju zajedno, zadatak prvog je da uklanja kuglice sa prve dve zice, zadatak drugog sa poslednje dve, dok sa srednje zice uklanjaju kuglice po dogovoru, svako sa svog tastera</li>
  <li>da bi se dobio maksimalan broj poena treba uklanjati kuglice sto blize zutoj liniji</li>
  <li>igra se zavrsava porazom ukoliko se kuglice ne otklanjaju redosledom kojim izlaze ili neka kuglica dospe do crvene linije</li>
  <li>igra se zavrsava pobedom ukoliko su sve kuglice otklonjene</li>
  <li>postignut skor je zapisan u fajlu skor.txt i u terminalu, na kraju svih igara </li>
  <li>kuglice izlaze u ritmu pesme One more light od Linkin Park koja se moze naci na ovom linku: https://www.youtube.com/watch?v=Tm8LGxTLtQk</li>
  <li> </li>
</ul>

## Kompilacija sa: 
	gcc -I/usr/X11R6/include -I/usr/pkg/include -L/usr/X11R6/lib -L/usr/pkg/lib  -o gh main.c -lglut -lGL -lGLU -lSOIL -Wall
