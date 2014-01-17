/*
Name:	Antonio (Antuan) Vassell 0805694
		Shauna Samuels		 
__________________
Student Professor
__________________
This program is to stimulate a class session consisting of students competing with each other to answer questions to the
professor and the professor her self taking a nap when no students not asking a question or waking up when there is a question
*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<sys/ipc.h>
#include<semaphore.h>
#include<sched.h>

#define NSTUDENTS_T 7 /*Number of students */
sem_t mutex,attention,professor;/*Three semaphors*/

int student_name;

void AnswerStart(){
	printf("Professor: Zzz zz zz #sleeping...\n");
	sem_wait(&attention);
}
void AnswerDone(){
	printf("Professor: Finish Answering student %d...\n",student_name);
	sem_post(&professor);
}
void QuestionStart(int x){
	printf("Student %d: Waiting For my turn...\n",x);
	sem_wait(&mutex);
	student_name = x;
}
void QuestionDone(int x){
	sem_post(&attention);
	sem_wait(&professor);
	printf("Student %d: Got my answer...\n",x);
	sem_post(&mutex);
}
void * ProfessorFunc(void *arg){
	while(true){
		AnswerStart();
		sleep(1);
		printf("Professor: Answering student %d...\n",student_name);
		sleep(1);
		AnswerDone();
	}
}
void * StudentFunc(void * ptr){
	int x=*((int*)ptr);
	while(true){
		QuestionStart(x);
		sleep(1);
		printf("Student %d: Asking...\n",x);
		sleep(1);
		QuestionDone(x);
		sleep(1);
	}
}
int main(){
	pthread_t students_t[NSTUDENTS_T];
	pthread_t professor_t;

	sem_init(&mutex,0,1);
	sem_init(&attention,0,0);
	sem_init(&professor,0,0);

	for(int i=0;i<NSTUDENTS_T;i++){
		pthread_create(&students_t[i],NULL,StudentFunc,(void *)&i);
		sleep(1);
	}
	pthread_create(&professor_t,NULL,ProfessorFunc,NULL);
	for(int i=0;i<NSTUDENTS_T;i++){
		pthread_join(students_t[i],NULL);
	}
	pthread_join(professor_t,NULL);
	exit(0);
}
