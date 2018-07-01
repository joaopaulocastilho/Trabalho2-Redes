#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include<unistd.h>

int i;
pthread_mutex_t lock;

void* t1(void *arg) {
  pthread_mutex_lock(&lock);
  for (i = 0; i < 10; i++) printf("%d ", i);
  printf("\n");
  pthread_mutex_unlock(&lock);
}

void* t2(void *arg) {
  pthread_mutex_lock(&lock);
  for (i = 80; i < 90; i++) printf("%d ", i);
  printf("\n");
  pthread_mutex_unlock(&lock);
}

int main(void) {
  pthread_t th1 = 1, th2 = 2;
  pthread_mutex_init(&lock, NULL);
  pthread_create(&th1, NULL, &t1, NULL); //Cria thread 1
  pthread_create(&th2, NULL, &t2, NULL); //Cria thread 2

  pthread_join(th1, NULL);
  pthread_join(th2, NULL);
  pthread_mutex_destroy(&lock);
  return 0;
}
