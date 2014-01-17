/*
Name: 	Antonio (Antuan) Vassell 0805694
	
Date: October 31, 2011
Operating Systems Programming Assignment

This is a stimulation of a laundromat
the function customer() adds new customers to the system which stimulates customers walking in
this will signal to the Station() that there are new customers so assign them to a machine and add them to the queue of customers to be processed. 
This will signal to the Machine() that there are customers to be processed and they will process then release thereself and go back to waiting on new customers. 
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

#include<sys/ipc.h>
#include<semaphore.h>
#include<sched.h>
#include<iostream>
#include<errno.h>

using namespace std;

#define NMACHINES 4
#define NSTATIONS 2
#define NCUSTOMERS 10

sem_t mFree; /* Lock on amount of machines available */
sem_t mAvailable; /* Lock on available array*/
sem_t sCustomers; /* Sem that there are new customers */
sem_t nCustomers; /* sem for customers to be processed */
sem_t sem_cus_count; /* Lock on amount of customers processed */
sem_t machines_s[NMACHINES];
int available[NMACHINES]; /* Number of machines in system */
int cus_count =0;  //Amount of customers added to the system
int cus_machine_num =0;
sem_t nexCus; /* lock on finding a customer in the queue to processed by a machine */

void gotoxy(int x, int y){
	//printf("\033[%d;%df",y,x);
	//fflush(stdout);
}
class Customer{
	/*
	 Customer Class
	*/
	private:
		int name;
		int amount;
		int machine;
		int processed;
	public:
		Customer(int name,int amount){
			this->name = name;
			this->amount = amount;
			this->machine=-1;
			this->processed =-1;
		}
		void setMachine(int machine){
			this->machine = machine;
		}
		int getProcessed(){
			
			return processed;
		}
		void setProcessed(int processed){
			this->processed = processed;
		}
		int getMachine(){
			return machine;
		}
		int getName(){
			return name;
		}
		int getAmount(){
			return amount;
		}
};
Customer *cus_ptr[NCUSTOMERS]; // customers to be added to the system
Customer *new_cus[NMACHINES]; // customers that are added to the system
Customer * getNexCus(){
	/*
	Returns the next available customer to be processed by a machine
	*/
	sem_wait(&nexCus);
	for(int i=0;i<cus_count;i++){
		if(new_cus[i]->getProcessed() == -1){
			new_cus[i]->setProcessed(1);
			sem_post(&nexCus);
			return new_cus[i];
		}
	}
	sem_post(&nexCus);
}
int allocate(){
	/*
	Allocate a customer to the next available machine
	*/
	int i;	
	sem_wait(&mFree);
	sem_wait(&mAvailable);
	for(i=0;i<NMACHINES;i++){
		if(available[i]!=0){
			available[i]=0;
			sem_post(&mAvailable);
			return i;
		}
	}
	sem_post(&mAvailable);
}
void release(int machine){
	/*
	After a machine funishes with a customer it makes itsefl available by modifying the availabe array
	*/
	sem_wait(&mAvailable);
	
	available[machine]=1;

	sem_post(&mAvailable);
	sem_post(&mFree);
}
void *MachineFunc(void * args){
	/*
	This Functions as a machine that waits for a customer to be assigned to it then runs for the amount of time it is required. then releases it self. 
	*/
	int x = *((int *)args);
	Customer *myCus;	
	while(true){
		cout<<"MACHINE "<<x<<" Waiting on my customer"<<endl;
		sem_wait(&machines_s[x]);
		//myCus=getNexCus();
		myCus=new_cus[x];
		cout<<"Machine "<<x<<" in use by Customer: "<<myCus->getName()<<endl;
		sleep(2);
		release(x);
		cout<<"MACHINE "<<x<<" Finish Washing\n";
		sleep(1);
	}
}
void *CustomerFunc(void * args){
	/*
	This is a simple function that just concurrently adds new customers to the system
	*/
	for(int i=0;i<NCUSTOMERS;i++){
		cout<<"Adding a new customer:"<<i<<endl;
		gotoxy(1,1);		
		cus_ptr[i]=new Customer(i,2);
		sem_post(&sCustomers);
	}
}
void *StationFunc(void * args){
	/*
	This function as the individual stations that the customer would insert token and get assigned to a machine
	*/
	while(true){
		cout<<"Waiting for available customer to walk in.."<<endl;
		sem_wait(&sCustomers);
		cout<<"Allocating New Customer...\n";
		sem_wait(&sem_cus_count);
		int name = cus_count;
		
		sem_post(&sem_cus_count);
		int i=0;
		int machine_name=0;
		while(true){
			machine_name = allocate();
			cus_ptr[name]->setMachine(machine_name);
			
			cout<<"New customer "<<cus_ptr[name]->getName()<<" Assigned Machine "<<cus_ptr[name]->getMachine()<<endl;
			i++;
			new_cus[machine_name] = cus_ptr[name];
			sem_post(&machines_s[machine_name]);
			if(cus_ptr[name]->getAmount()<= i){
				break;
			}
			
		}
		
		sem_wait(&sem_cus_count);
		cus_count++;
		sem_post(&sem_cus_count);
		sleep(1);
		
	}
}

int main(){
	pthread_t stations_t[NSTATIONS];
	pthread_t machines_t[NMACHINES];
	pthread_t customer_t;
	for(int i=0;i<NMACHINES;i++){
		available[i]=1;
	}
	for(int i=0;i<NMACHINES;i++){
		sem_init(&machines_s[i],0,0);
	}
	sem_init(&sem_cus_count,0,1);
	sem_init(&sCustomers,0,0);
	sem_init(&nCustomers,0,0);
	sem_init(&mFree,0,NMACHINES);
	sem_init(&nexCus,0,1);
	sem_init(&mAvailable,0,1);

	for(int i=0;i<NMACHINES;i++){
		available[i]=1;
	}
	for(int i=0;i<NSTATIONS;i++){
		pthread_create(&stations_t[i],NULL,StationFunc,NULL);
		sleep(1);
	}
	for(int i=0;i<NMACHINES;i++){
		pthread_create(&machines_t[i],NULL,MachineFunc,(void *)&i);
		sleep(1);
	}
	pthread_create(&customer_t,NULL,CustomerFunc,NULL);
	for(int i=0;i<NSTATIONS;i++){
		pthread_join(stations_t[i],NULL);
	}
	for(int i=0;i<NMACHINES;i++){
		pthread_join(machines_t[i],NULL);
	}
	pthread_join(customer_t,NULL);
	exit(0);
}
