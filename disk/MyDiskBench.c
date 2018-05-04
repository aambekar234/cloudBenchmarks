
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

double data_size = 1e10;
const char* filename = "/tmp/test.bin";
const double th_value = 600.0;
const double th_l_value = 0.5;

typedef struct test_strat 
{
	FILE *fp;
	int loop_count;
    	int start;
	int end;
	int block_size;
	int type;
	char* buff;

} test_strat;



void initialize_out_file(char * filename)
{
	FILE *fp = fopen(filename,"w");
	fputs("Workload\t Concurrency\t BlockSize\t MyDiskBenchThroughput\t TheoreticalThroughput\t Efficiency\n",fp);
	fclose(fp);
	
}

void *disk_write(void *strat)
{
	struct test_strat *s_data = (test_strat *) strat;
	FILE *fp = s_data->fp;
	int start = s_data->start;
	int end = s_data->end;
	int block_size = s_data->block_size;
	int loop_count = s_data->loop_count;
	int type = s_data->type;
	char *buff = s_data->buff;

	fseek(fp,start,SEEK_SET);

	for(int i =0;i<loop_count;i++)
	{
		if(type == 0)
		{
			int r = rand() % loop_count;
			fseek(fp,start+(r*block_size), SEEK_SET);
		}	
		fwrite(buff,1,block_size,fp);	
		
	}


}


void *disk_read(void *strat)
{
	struct test_strat *s_data = (test_strat *) strat;
	FILE *fp = s_data->fp;
	int start = s_data->start;
	int end = s_data->end;
	int block_size = s_data->block_size;
	int loop_count = s_data->loop_count;
	int type = s_data->type;
	char * buff = (char *)malloc(block_size);

	setvbuf(fp, NULL, _IONBF, 0);
	int fd = fileno(fp);
	fsync(fd);

	fseek(fp,start,SEEK_SET);
	char * read_buff = malloc(block_size);

	for(int i =0;i<loop_count;i++)
	{
		if(type == 0)
		{
			int r = rand() % loop_count;
			fseek(fp,start+(r*block_size), SEEK_SET);
		}	
		memset(buff,0,block_size);
		memset(read_buff,0,block_size);
		fread(buff,block_size,1,fp);
		strncpy(read_buff,buff, block_size);	
		fflush(fp);
		//read(fd,buff,block_size);
	}
	free(read_buff);
	free(buff);

}


//run test according to input file
void run_test(int strat, char *strat_label,int threadc, double block_size, char* block_size_label, char *out_file)
{
	pthread_t threads[threadc]; 
	long tid; 
	int rc; 
	
	struct timeval  start, end;

	double block = data_size/ threadc;	
	int loop_count = block / block_size;
	FILE *fp;
	if(strat ==1 || strat == 2)
	{
		fp=fopen(filename, "wb");
		printf("Write \n");
	}
	else
	{
		fp=fopen(filename, "rb");
	}
	
	
	srand(time(NULL));
	char *buff = (char *) malloc(block_size);
	int r = rand() % 10;
	char a = 65 + r;
	memset(buff,a,block_size);	
	

	gettimeofday(&start, NULL);
	double tstart = start.tv_sec+(start.tv_usec/1000000.0);	
	
	
	test_strat *data = (test_strat*) malloc(threadc * sizeof (test_strat));

	int type = strat %2;	
	for (tid = 0; tid < threadc; tid++) 
	{
		int start = tid * block;
		int end = start + block -1;
		switch(strat)
		{
			case 1:
			case 2:
				 
				data[tid].start = start;
				data[tid].end = end;
				data[tid].block_size = block_size; 
				data[tid].loop_count = loop_count;
				data[tid].fp = fp;
				data[tid].type = type;
				data[tid].buff = buff;

				rc = pthread_create(&threads[tid], NULL, disk_write, (void *) &data[tid]);

				break;
			case 3:
			case 4:
				data[tid].start = start;
				data[tid].end = end;
				data[tid].block_size = block_size; 
				data[tid].loop_count = loop_count;
				data[tid].fp = fp;
				data[tid].type = type;
				data[tid].buff = buff;

				rc = pthread_create(&threads[tid], NULL, disk_read, (void *) &data[tid]);
				break;
		}		

		if (rc) 
		{
			printf("Could not create thread %ld\n", tid);
		}

	}

	for (tid = 0; tid < threadc; tid++) 
	{
		rc = pthread_join(threads[tid], NULL);
		if (rc) 
		{
			printf("Could not join thread %ld\n", tid);
		}
	}

	gettimeofday(&end, NULL);	
	double tend = end.tv_sec+(end.tv_usec/1000000.0);
	double time = tend - tstart;

	double throughput = ((double)loop_count * (double)block_size * (double)threadc)/(time * (double)1e6);
	double efficiency = (throughput/th_value) * (double)100;

	//calculate latency
	if(block_size == 1)
	{
		double latency = time /(double)1000;
		throughput = latency;
		efficiency = (throughput/ th_l_value) * (double)100;
	}

	printf("Workload\t Concurrency\t BlockSize\t Throughput\t TheoreticalThroughput\t Efficiency\n\n");
	
	printf("%s\t\t %d\t\t %s\t\t %f\t\t %f\t\t %f\n", strat_label,threadc,block_size_label, throughput, th_value, efficiency);



	printf("Writing output in a file...\n");
	FILE *fp2 = fopen(out_file,"r");
	if (fp2 == NULL)
	{
		
		initialize_out_file(out_file);
	} 
	
		FILE *fp3 = fopen(out_file,"a");
		fprintf(fp3,"%s\t\t %d\t\t %s\t\t %f\t\t %f\t\t %f\n", strat_label,threadc,block_size_label, throughput, th_value, efficiency);
		fclose(fp3);
	printf("Test Complete...\n");

	
	free(buff);
	fclose(fp);
}

void generate_data_file()
{
	printf("Please wait! Generating data file to read..\n");
	srand(time(NULL));
	FILE *fp=fopen(filename, "wb");
	int loop_count = data_size / 1000;	
	
	
	for(int i = 0;i<loop_count;i++)
	{
		int r =rand() % 10;
		char a = 65 + r;
		char *buff = (char *) malloc(1000);
		memset(buff,a,1000);
		fwrite(buff, 1, 1000, fp);
		free(buff);
					
	}
	fclose(fp);
}

int main(int argc, char** argv)
{

	

	if(argc <2)
	{
		printf("Usage [input filename path]\n");
		return 0;
	}	
	char* filename = argv[1];

		
	FILE *fp = fopen(filename,"r");
	if (fp == NULL)
	{
		printf("File not found \n");
		return 0;
	}
	else
	{

		char strat_label[100];
		char block_size_l[100];
		char thread_count[100];
		
		
		memset(block_size_l,0,100);
		memset(thread_count,0,100);
		memset(strat_label,0,100);		
		

		fscanf(fp,"%[^\n]\n", strat_label);
		fscanf(fp,"%[^\n]\n",block_size_l);
		fscanf(fp,"%[^\n]\n",thread_count);

		int threadc = atoi(thread_count);
		int block_size = atoi(block_size_l);
		
		

		if(threadc == NULL || strat_label == NULL || block_size == NULL)
		{
			printf("Input file is incorrect.\n");
			
		}
		else
		{
				
			int strat = 0;
			if (strcmp(strat_label, "WS") == 0) 
			{
			  	strat = 1;
			}
			else if (strcmp(strat_label, "WR") == 0) 
			{
			  	strat = 2;
			}
			else if (strcmp(strat_label, "RS") == 0) 
			{
			  	strat = 3;
			}
			else if (strcmp(strat_label, "RR") == 0) 
			{
			  	strat = 4;
			}
			
			char block_size_label[100];
			memset(block_size_label,0,100);

			if(block_size == 1)
			{
				data_size = 1e9;
				strcat(block_size_label,"1B");
			}
			else if (block_size == 1e3)
			{
				strcat(block_size_label,"1KB");
			}
			else if(block_size == 1e6)
			{
				strcat(block_size_label,"1MB");
			}
			else if(block_size == 1e7)
			{
				strcat(block_size_label,"10MB");
			}
			else if(block_size == 1e7)
			{
				strcat(block_size_label,"100MB");
			}
			
			

			if(strat == 0)
			{
				printf("Input file is incorrect.\n");
				return 0;
			}
			else
			{
				
				
				if(strcmp(strat_label,"RR") == 0 || strcmp(strat_label,"RS")==0)
				{
					generate_data_file();
				}

	
				printf("Your input -> Workload is %s and, thread count is %d, Block size is %s\n",strat_label, threadc,block_size_label );
				
				char out_file[100];
				memset(out_file,0,100);
				sprintf(out_file, "%s%s%s%d%s%d%s", "output/disk_",strat_label,"\-",block_size, "\-", threadc,"thread.out.dat");				
				

				printf("Output file is %s\n",out_file);

				

				run_test(strat,strat_label,threadc,block_size,block_size_l,out_file);

			}

		}
		
		fclose(fp);

	}


	return 0; 
}


