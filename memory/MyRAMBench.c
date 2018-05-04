#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>


const long data_size =1e9;
const double th_value = 68.2560;
long shared_var;
char *src;
char *dest;
const double th_l_value = 0.01400;


/*
struct to pass data to thread
*/

typedef struct test_strat 
{
	int loop_count;
    	char* src;
	char* dest;
	int block_size;
} test_strat;

void initialize_out_file(char * filename)
{
	FILE *fp = fopen(filename,"w");
	fputs("Workload \tConcurrency \tBlockSize \tMyRAMBenchValue \tTheoreticalValue \tMyRAMBenchEfficiency\n",fp);
	fclose(fp);
	
}

/*

run thread for sequential benchmark
*/
void *rws(void *strat)
{

	

	struct test_strat *s_data = (test_strat *) strat;
	char *src1 = s_data->src;
	char *dest1 = s_data->dest;
	int block_size = s_data->block_size;
	int loop_count = s_data->loop_count;
	
	int outer = 50;
	if(block_size ==1)
	{
		outer = 1;
	}

	for(int j = 0;j<outer;j++)
	{
		for(int i=0;i<loop_count;i++)
		{
			memcpy(src1 + i * block_size, dest1 + i * block_size, block_size);
		}
	}
		
	pthread_exit(NULL);
}

/*

run thread for random benchmark
*/
void *rwr(void *strat)
{
	

	struct test_strat *s_data = (test_strat *) strat;
	char *src1 = s_data->src;
	char *dest1 = s_data->dest;
	int block_size = s_data->block_size;
	int loop_count = s_data->loop_count;
		
	int outer = 50;
	if(block_size ==1)
	{
		outer = 1;
	}

	for(int j = 0;j<outer;j++)
	{
		for(int i=0;i<loop_count;i++)
		{
			int r = rand();
			int seed = r % loop_count;
			memcpy(src1 + seed * block_size, dest1 + seed * block_size, block_size);
		}
	}	

	pthread_exit(NULL);

}

/*
run_test -> runs benchmark by provided input
strat -> 1 for sequential, 2 for random
threadc -> thread count
block_size -> size of block
block_label -> block_size in char_array
*/

void run_test(int strat,char *strat_label, int threadc, double block_size, char* block_label, char* outfile)
{

	printf("Running the test..\n");
	pthread_t threads[threadc]; 
	long tid; 
	int rc; 
	
	struct timeval  start, end;
	gettimeofday(&start, NULL);
	double tstart = start.tv_sec+(start.tv_usec/1000000.0);	
	int loop_count = data_size/(threadc * block_size);
	long block = data_size/ threadc;
	test_strat *data = (test_strat*) malloc(threadc * sizeof (test_strat));

	for (tid = 0; tid < threadc; tid++) 
	{
		
		long src_a = (block * tid);
		long dest_a = (block * (tid +1)) -1;
	
		switch(strat)
		{
			case 1:
				 
				data[tid].src = src + src_a;
				data[tid].dest = dest + dest_a;
				data[tid].block_size = block_size; 
				data[tid].loop_count = loop_count;
				rc = pthread_create(&threads[tid], NULL, rws, (void *) &data[tid]);
				break;
			case 2:
				data[tid].src = src + src_a;
				data[tid].dest = dest + dest_a;
				data[tid].block_size = block_size; 
				data[tid].loop_count = loop_count;
				rc = pthread_create(&threads[tid], NULL, rwr, (void *) &data[tid]);
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

	int outer = 50;
	if(block_size ==1)
	{
		outer = 1;
	}

	gettimeofday(&end, NULL);	
	double tend = end.tv_sec+(end.tv_usec/1000000.0);
	double time = tend - tstart;

	double latency = time * (double)1e6 /(double)1e11;

	double mem_used = ((double)block_size * (double)loop_count * (double)threadc) * (double)outer;
	double throughput = mem_used/(time * (double)1e9);

	double efficiency = (throughput/ th_value) * (double)100; 
	
	printf("Workload \tConcurrency \tBlockSize \tMyRAMBenchValue \tTheoreticalValue \tMyRAMBenchEfficiency\n\n");

	if(block_size == 1)
	{
		throughput = latency;
		efficiency = (throughput/ th_l_value) * (double)100;
	}
	
	
	printf("%s\t\t %d\t\t %s\t\t %f\t\t %f\t\t %f\n", strat_label,threadc,block_label,throughput,th_value,efficiency);

	

	printf("Writing output in a file...\n");
	FILE *fp = fopen(outfile,"r");
	if (fp == NULL)
	{
		initialize_out_file(outfile);
	} 
	
		FILE *fp2 = fopen(outfile,"a");
		fprintf(fp2,"%s\t\t %d\t\t %s\t\t %f\t\t %f\t\t %f\n", strat_label,threadc,block_label,throughput,th_value,efficiency);
		fclose(fp2);
	printf("Test Complete...\n");
}

void remove_newline_ch(char *line)
{
    int new_line = strlen(line) -1;
    if (line[new_line] == '\n' || line[new_line] == '\r' )
        line[new_line] = '\0';
}

	
int main (int argc, char** argv) 
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
			if (strcmp(strat_label, "RWS") == 0) 
			{
			  	strat = 1;
			}
			else if (strcmp(strat_label, "RWR") == 0) 
			{
			  	strat = 2;
			}
			
			char block_size_label[100];
			memset(block_size_label,0,100);

			if(block_size == 1)
			{
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
			
			printf("%s\n",block_size_label);

			if(strat == 0)
			{
				printf("Input file is incorrect.\n");
				return 0;
			}
			else
			{
				src = (char *) malloc(data_size);
				dest = (char *) malloc(data_size);

				printf("Please wait! Preparing 1 GB data..\n");
	
				memset(src,'a',data_size);
				memset(dest,'b',data_size);
				printf("1GB Data Ready..\n");

				printf("Your input -> Workload is %s and, thread count is %d, Block size is %s\n",strat_label, threadc,block_size_label );
				
				char out_file[100];
				
				sprintf(out_file, "%s%s%s%d%s%d%s", "output/memory_",strat_label,"\-",block_size, "\-", threadc,"thread.out.dat");				
				

				printf("Output file is %s\n",out_file);

				
				
				run_test(strat,strat_label,threadc,block_size,block_size_label,out_file);

				free(src);
				free(dest);
			}

		}
		
		fclose(fp);

	}



	
   return(0);
}


