#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

struct car{
	char *driver;
	int direction;
	int arrival;
	int duration;
	struct car *next;
};

struct car* head = NULL;
struct car* tail = NULL;
int currentTime;
int currentDirection = -1;
int flow = -1;
int carsOnBridge = 0;
int numberInDirection = 0;
int currDir = -1;

sem_t sem;

void initialize(){

	head = tail = NULL;
}

void enqueue(char *driverName, int direction, int arrivalTime, int durationTime){

	struct car* temp = (struct car*)malloc(sizeof(struct car));
	temp -> driver = malloc(sizeof(char)*(strlen(driverName) + 1));
	strcpy(temp -> driver, driverName);
	temp -> direction = direction;
	temp -> arrival = arrivalTime;
	temp -> duration = durationTime;
	temp -> next = NULL;

	if(head == NULL && tail == NULL){
		head = temp;
		tail = temp;
		return;
	}
	tail -> next = temp;
	tail = temp;
}
void dequeue(){
	struct car* temp = head;

	if(head == NULL)
		return;

	if(head == tail){
		head = NULL;
		tail = NULL;
	}else{
		head = head -> next;
	}
	free(temp -> driver);
	free(temp);
	temp = NULL;
}
struct car * getCar(int i){

	struct car * temp = head;
	int j = 0;

	while(temp != tail && j != i){
		temp = temp -> next;
		j++;
	}
	return temp;
}
void * leaveBridge(void * car){

	struct car * temp = (struct car*)car;
	sem_post(&sem);
	numberInDirection++;
	char * s = ((temp->direction == 'S')? "South" : "North");

	if(temp -> direction != currDir){
   		printf("Direction: %s\n", s);
		currDir = temp -> direction;
	}
	
  	printf("%s\n", temp->driver);
  	carsOnBridge--;
  	sem_wait(&sem);
  	pthread_exit(0);

}
void * drive(void * currentCar){

	struct car * temp = (struct car *) currentCar;
	int p = -1, dir = temp -> direction, j = 0;

	while(p != dir){
		sleep(1);
		sem_post(&sem);

		//if 3 straight cars in one direction switch direction
		//to allow cars to cross in other direction 
		if(numberInDirection == 3){
			numberInDirection = 0;
			flow = ((flow == 'N')? 'S' : 'N');
		}
		if(flow == -1 || (carsOnBridge == 0 && j > 5)){
			flow = dir;
			numberInDirection = 0;
		}
		p = flow;

		if(numberInDirection < 3 && dir == flow && carsOnBridge < 3){
			carsOnBridge++;
			j = 0;
			break;
		}
		j++;
		sem_wait(&sem);
	}
	sem_wait(&sem);
	leaveBridge(temp);
	return NULL;
}
int main(){

	//initlialize semaphore
	sem_init(&sem, 0, 1);
	int counter = 0;
	int i = 0;
	char headers[10][100] = {};
	char user[1024], process[1024];
	int directionIn;
	//char directionS[1024];
	int arrival, duration;
	int directionPass;

    scanf( "%s%s%s%s", headers[0], headers[1], headers[2], headers[3]);

    struct car* temp = NULL;

    while(scanf("%s %lc %d %d", user, &directionIn, &arrival, &duration) != EOF){

		enqueue(user, directionIn, arrival, duration);
		counter++;
	}
	pthread_t tid[counter];

	temp = getCar(0);
	for(currentTime = 0; temp != NULL && currentTime < 10000; currentTime++){

		if(temp -> arrival == currentTime){

			pthread_create(&tid[i], NULL, drive, temp);
			i++;
		}
		temp = getCar(i);

		if(i == counter){
			temp = NULL;
		}
		sleep(1);
	}
	for(i = 0; i < counter; i++){
		pthread_join(tid[i], NULL);
	}
	//destroy semaphore since its not longer needed
	sem_destroy(&sem);
}
