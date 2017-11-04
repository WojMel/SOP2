/*
Opis gry w Osadników:

Dolinę zamieszkuje populacja osadników.
Każdy osadnik ma przydzieloną profesję:
   jest myśliwym
   albo kucharzem.
W osadzie gromadzone są dwa typy zasobów:
   zwierzyna i pożywienie.

Każdego dnia każdy myśliwy rusza na łowy i podejmuje jednorazową próbę upolowania zwierzyny.
Aby określić powodzenie polowania wykonujemy dwa rzuty kostką sześcienną:
   jeden rzut dla myśliwego oraz jeden dla zwierzyny i porównujemy te wartości.
   Jeśli liczba oczek myśliwego przewyższa liczbę oczek zwierzyny,
   polowanie jest zakończone sukcesem i zwiększamy o 1 liczbę dostępnej zwierzyny w osadzie.
Niezależnie od wyniku polowania, myśliwy wraca do osady,
konsumuje jedną jednostkę pożywienia i śpi jedną jednostkę czasu (czyli do końca dnia).

Każdego dnia każdy kucharz podejmuje jednorazową akcję pieczenia jednej jednostki zwierzyny
   (o ile zwierzyna jest dostępna w osadzie).
Akcja pieczenia zwierzyny redukuje o 1 liczbę dostępnej zwierzyny
i produkuje tyle jednostek pożywienia, ile oczek wyrzuci kucharz na kostce sześciennej.
Po akcji pieczenia kucharz konsumuje jedną jednostkę pożywienia i śpi jedną jednostkę czasu (czyli do końca dnia).

Jeśli liczba jednostek pożywienia wynosi zero,
osadnik próbujący wykonać akcję konsumpcji opuszcza dolinę na zawsze w poszukiwaniu bardziej urodzajnego miejsca do życia.


/////////////////////////////////////////////////////////////////////
Program ten to implementacja wielowątkowej symulacji gry w Osadników.

Ustalenia techniczne dotyczące implementacji:
1. Schemat działania każdego osadnika wykonywany jest w osobnym wątku.
2. Istnieje parametryzacja liczby myśliwych, kucharzy i początkowej liczby jednostek zasobów zwierzyny i pożywienia
   poprzez podanie argumentów przy uruchomieniu programu.
    Zachowana jest ustalona kolejność parametrów.
    Przykładowe wywołanie:
    $ ./osadnicy 10 11 3 7
    oznacza uruchomienie symulacji dla 10 myśliwych, 11 kucharzy w osadzie, która przechowuje 3 jednostki zwierzyny i 7 jednostek pożywienia.
3. Przeprowadzona symulacja każdego osadnika jest ograniczona do 365 dni.
4. Zasoby są nierozróżnialne i przechowywane są jako informacja o ich ilości.
5. Jednostka czasu, przez którą śpi każdy osadnik jest rzędu milisekund.

!!
Wstępne ustalenia dotyczące oceniania:
1. Program powinien mieć poprawnie zdefiniowane sekcje krytyczne w miejscach, które tego wymagają.
2. Program nie powinien mieć zdefiniowanych sekcji krytycznych w miejscach, które tego nie wymagają.
3. Liczba prymitywów synchronizacyjnych (np. condition variables) powinna być minimalna.
4. Program powinien kończyć się w skończonym czasie (brak deadlocka).
5. Szczególną uwagę należy zwrócić na to, żeby liczba zasobów nigdy nie osiągnęła wartości ujemnej
!!

Ustalenia techniczne dotyczące oddawania rozwiązania:
[x] 1. Program powinien mieć jeden plik źródłowy o dowolnej nazwie, o rozszerzeniu .c lub .cpp
[x] 2. Program powinien być obsługiwany z poziomu polecenia make. Uruchomienie polecenia 'make' powinno służyć do kompilacji.
      Uruchomienie polecenia 'make run' powinno służyć do uruchomienia programu z pewnym domyślnym zestawem argumentów.
[x] 3. Program powinien być napisany w C lub C++ z użyciem bibilioteki pthreads.
[x] 4. Program powinien uruchamiać się na platformie Linux.
[x] 5. Repozytorium powinno zawierać plik .gitignore.
[x] 6. Instrukcja kompilacji powinna zawierać flagi: -Wall, -Werror.
*/


#include <stdio.h>
#include <stdlib.h> 
#include <time.h> 
#include <unistd.h>
#include <pthread.h>
const long MAX_DNI     = 365;
const long CZAS_SPANIA = 0;

long Zwierzyna  = 0;
long Pozywienie = 0;
pthread_mutex_t mutex_Zwierzyna;
pthread_mutex_t mutex_Pozywienie;

void *mysliwy(void *vargp);
void *kucharz(void *vargp);

void thread_create(pthread_t* threads, long size, pthread_attr_t *attr, void*(*f)(void*));
void thread_joinNfree(pthread_t* threads, long size);
  


int main(int argc, char *argv[]){
   if( argc != 5 ) return 1;
   srand(time(NULL));

   long lmysliwi = strtol(argv[1], NULL, 10);
   long lkucharze = strtol(argv[2], NULL, 10);
   Zwierzyna = strtol(argv[3], NULL, 10);
   Pozywienie = strtol(argv[3], NULL, 10);

   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

   pthread_mutex_init(&mutex_Zwierzyna, NULL);
   pthread_mutex_init(&mutex_Pozywienie, NULL);


   pthread_t *watki_mysliwi =  (pthread_t *)malloc(lmysliwi*sizeof(pthread_t));
   pthread_t *watki_kucharze = (pthread_t *)malloc(lkucharze*sizeof(pthread_t));

   thread_create(watki_mysliwi, lmysliwi, &attr, &mysliwy);
   thread_create(watki_kucharze, lkucharze, &attr, &kucharz);
   pthread_attr_destroy(&attr);

   thread_joinNfree(watki_mysliwi, lmysliwi);
   thread_joinNfree(watki_kucharze, lkucharze);
   pthread_mutex_destroy(&mutex_Zwierzyna);
   pthread_mutex_destroy(&mutex_Pozywienie);

   return 0;
}



void thread_joinNfree(pthread_t* threads, long size){
   for(long i = 0; i < size; ++i)
      pthread_join(threads[i], NULL);
   free(threads);
}

void thread_create(pthread_t* threads, long size, pthread_attr_t *attr, void*(*f)(void*)){
   for(long i = 0; i < size; ++i)
      pthread_create(&threads[i], attr, f, NULL);
}

void *mysliwy(void *vargp){
   for(int dzien =0; dzien < MAX_DNI; ++dzien){

      if( rand() % 6 > rand() % 6 ){
         pthread_mutex_lock(&mutex_Zwierzyna);
         ++Zwierzyna;
         pthread_mutex_unlock(&mutex_Zwierzyna);
      }
      
      pthread_mutex_lock(&mutex_Pozywienie);
      if( Pozywienie > 0 ){
         --Pozywienie;
         pthread_mutex_unlock(&mutex_Pozywienie);
      }
      else{
         pthread_mutex_unlock(&mutex_Pozywienie);
         return NULL;
      }

      sleep(CZAS_SPANIA);
   }
   return NULL;
}

void *kucharz(void *vargp){
   for(int dzien =0; dzien < MAX_DNI; ++dzien){

      pthread_mutex_lock(&mutex_Zwierzyna);
      if( Zwierzyna > 0 ){
         --Zwierzyna;
         pthread_mutex_unlock(&mutex_Zwierzyna);
         pthread_mutex_lock(&mutex_Pozywienie);
         Pozywienie += (rand() % 6) + 1;
         pthread_mutex_unlock(&mutex_Pozywienie);
      }
      else pthread_mutex_unlock(&mutex_Zwierzyna);

      
      pthread_mutex_lock(&mutex_Pozywienie);
      if( Pozywienie > 0 ){
         --Pozywienie;
         pthread_mutex_unlock(&mutex_Pozywienie);
      }
      else{
         pthread_mutex_unlock(&mutex_Pozywienie);
         return NULL;
      }

      sleep(CZAS_SPANIA);
   }
   return NULL;
}