#include <stdio.h>
#include <stdlib.h> 
#include <time.h> 
#include <pthread.h>

const long MAX_DNI     = 365;
struct timespec CZAS_SPANIA;
time_t CZAS_SPANIA_sec = 0;
long CZAS_SPANIA_nsec = 5L*1000000L;

long max_dni = 0;
pthread_mutex_t mutex_dni;

typedef struct Zasoby_s {
   long Zwierzyna;
   long Pozywienie;
} Zasoby_t;
pthread_mutex_t mutex_Zasoby;

void *mysliwy(void *vargp);
void *kucharz(void *vargp);

void thread_create(pthread_t* threads, long size, pthread_attr_t *attr, void*(*f)(void*), void *vargp);
void thread_joinNfree(pthread_t* threads, long size);
  


int main(int argc, char *argv[]){
   if( argc != 5 ) return 1;
   srand(time(NULL));
   CZAS_SPANIA.tv_sec = CZAS_SPANIA_sec;
   CZAS_SPANIA.tv_nsec = CZAS_SPANIA_nsec;

   Zasoby_t Zasoby;
   long lmysliwi = strtol(argv[1], NULL, 10);
   long lkucharze = strtol(argv[2], NULL, 10);
   Zasoby.Zwierzyna = strtol(argv[3], NULL, 10);
   Zasoby.Pozywienie = strtol(argv[4], NULL, 10);

   printf("--Status poczatkowy:\nLiczba mysliwych: %ld\nLiczba kucharzy: %ld\nLiczba zwierzyny: %ld\nLiczba pozywienia: %ld\n\n",lmysliwi,lkucharze,Zasoby.Zwierzyna,Zasoby.Pozywienie);

   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

   pthread_mutex_init(&mutex_Zasoby, NULL);
   pthread_mutex_init(&mutex_dni, NULL);


   pthread_t *watki_mysliwi =  (pthread_t *)malloc(lmysliwi*sizeof(pthread_t));
   pthread_t *watki_kucharze = (pthread_t *)malloc(lkucharze*sizeof(pthread_t));

   thread_create(watki_mysliwi, lmysliwi, &attr, &mysliwy, (void*)&Zasoby);
   thread_create(watki_kucharze, lkucharze, &attr, &kucharz, (void*)&Zasoby);
   pthread_attr_destroy(&attr);

   thread_joinNfree(watki_mysliwi, lmysliwi);
   thread_joinNfree(watki_kucharze, lkucharze);
   pthread_mutex_destroy(&mutex_Zasoby);
   pthread_mutex_destroy(&mutex_dni);

   printf("--Status koncowy:\nLiczba mysliwych: 0\nLiczba kucharzy: 0\nLiczba zwierzyny: %ld\nLiczba pozywienia: %ld\nOsada przetrwala %ld dni.\n",Zasoby.Zwierzyna,Zasoby.Pozywienie,max_dni);

   return 0;
}



void thread_joinNfree(pthread_t* threads, long size){
   for(long i = 0; i < size; ++i)
      pthread_join(threads[i], NULL);
   free(threads);
}

void thread_create(pthread_t* threads, long size, pthread_attr_t *attr, void*(*f)(void*), void *vargp){
   for(long i = 0; i < size; ++i)
      pthread_create(&threads[i], attr, f, vargp);
}

void *mysliwy(void *vargp){
   Zasoby_t *Zasoby = (Zasoby_t *)vargp;
   int dzien;
   for(dzien =0; dzien < MAX_DNI; ++dzien){

      pthread_mutex_lock(&mutex_Zasoby);
      if( rand() % 6 > rand() % 6 ){
         ++Zasoby->Zwierzyna;
      }
      
      if( Zasoby->Pozywienie > 0 ){
         --Zasoby->Pozywienie;
         pthread_mutex_unlock(&mutex_Zasoby);
      }
      else{
         pthread_mutex_unlock(&mutex_Zasoby);
         break;
      }

      nanosleep(&CZAS_SPANIA, NULL);
   }   
   pthread_mutex_lock(&mutex_dni);
   if( dzien > max_dni ) max_dni = dzien;
   pthread_mutex_unlock(&mutex_dni);
   return NULL;
}

void *kucharz(void *vargp){
   Zasoby_t *Zasoby = (Zasoby_t *)vargp;
   int dzien;
   for(dzien =0; dzien < MAX_DNI; ++dzien){

      pthread_mutex_lock(&mutex_Zasoby);
      if( Zasoby->Zwierzyna > 0 ){
         --Zasoby->Zwierzyna;
         Zasoby->Pozywienie += (rand() % 6) + 1;
      }

      if( Zasoby->Pozywienie > 0 ){
         --Zasoby->Pozywienie;
         pthread_mutex_unlock(&mutex_Zasoby);
      }
      else{
         pthread_mutex_unlock(&mutex_Zasoby);
         break;
      }

      nanosleep(&CZAS_SPANIA, NULL);
   }
   pthread_mutex_lock(&mutex_dni);
   if( dzien > max_dni ) max_dni = dzien;
   pthread_mutex_unlock(&mutex_dni);
   return NULL;
}