#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/time.h>
#include<unistd.h>
#include<immintrin.h>
#include<string.h>


const double total_operations = 1e12;


//129.114.33.105 



void initialize_out_file(char * filename)
{
	FILE *fp = fopen(filename,"w");
	fputs("Workload Concurrency MyCPUBenchValue TheoreticalValue Efficiency\n",fp);
	fclose(fp);
	
}

void *qp_test(void *args)
{
	long loop_counter = (long *)args;
	__m256i v1 = _mm256_set_epi8(34,34,67,23,89,123,56,80,34,34,67,23,79,13,46,70,24,34,67,23,79,123,45,78,234,34,67,23,79,123,56,80);
  	__m256i v2 = _mm256_set_epi8(13,56,78,123,123,56,76,71,234,56,78,123,13,56,76,71,23,56,78,123,123,56,76,71,24,56,78,123,123,56,76,71);
	__m256i r;
	
	loop_counter = loop_counter / 128;

	for(long i = 0;i<loop_counter;i++)
	{
		r = _mm256_add_epi8(v1, v2);
		r = _mm256_sub_epi8(v1, v2);
		r = _mm256_add_epi8(v2, v1);
		r = _mm256_sub_epi8(v2, v1);
	}

}

void *hp_test(void *args)
{
	long loop_counter = (long *)args;
	__m256i v1 = _mm256_set_epi16(234,34,67,23,789,123,456,780,234,34,67,23,23,789,123,456);
  	__m256i v2 = _mm256_set_epi16(234,56,234,56,78,123,123,56,76,71,234,56,78,123,123,56);
	__m256i r;
	
	loop_counter = loop_counter / (long)128;

	for(long i = 0;i<loop_counter;i++)
	{
		r = _mm256_add_epi16(v1, v2);
		r = _mm256_sub_epi16(v1, v2);
		r = _mm256_add_epi16(v2, v1);
		r = _mm256_sub_epi16(v2, v1);
		r = _mm256_add_epi16(v1, v2);
		r = _mm256_sub_epi16(v1, v2);
		r = _mm256_add_epi16(v2, v1);
		r = _mm256_sub_epi16(v2, v1);
	}
	
}

void *sp_test(void *args)
{
	long loop_counter = (long *)args;
	__m256i v1 = _mm256_set_epi32(1,2,3,4,1,2,3,4);
  	__m256i v2 = _mm256_set_epi32(1,2,3,4,1,2,3,4);
	__m256i r;
	
	loop_counter = loop_counter / (long)128;

	for(long i = 0;i<loop_counter;i++)
	{
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
		r = _mm256_sub_epi32(v1, v2);
	}
	
}

void *dp_test(void *args)
{
	long loop_counter = (long *)args;
	__m256d v1 = _mm256_set_pd(23.4, 424.0, 51.23, 82.0);
  	__m256d v2 = _mm256_set_pd(125.34, 73.0, 56.10, 57.0);
	__m256d v3 = _mm256_set_pd(133.45, 38.0, 85.30, 23.0);
	__m256d r;
	
	loop_counter = loop_counter / (long)128;


	for(long i = 0;i<loop_counter;i++)
	{
		r = _mm256_fmadd_pd(v1, v2,v3);
		r = _mm256_fmadd_pd(v2, v3,v1);
		r = _mm256_fmadd_pd(v3, v2,v1);
		r = _mm256_fmadd_pd(v1, v2,v3);
		r = _mm256_fmadd_pd(v1, v2,v3);
		r = _mm256_fmadd_pd(v1, v2,v3);
		r = _mm256_fmadd_pd(v1, v2,v3);
		r = _mm256_fmadd_pd(v1, v2,v3);
		r = _mm256_fmadd_pd(v3, v2,v1);
		r = _mm256_fmadd_pd(v1, v2,v3);
		r = _mm256_fmadd_pd(v1, v3,v2);
		r = _mm256_fmadd_pd(v1, v2,v3);
		r = _mm256_fmadd_pd(v2, v1,v3);
		r = _mm256_fmadd_pd(v3, v2,v1);
		r = _mm256_fmadd_pd(v1, v3,v2);
		r = _mm256_fmadd_pd(v2, v1,v3);
		
	}
	
}

// strat - test type ex QP, HP, SP or DP, threadc - thread count, strat-label - strategy label, outfile - output file name in string
void run_test(int strat, int threadc, char* strat_label, char* outfile )
{

	printf("Running the test..\n");
	pthread_t threads[threadc]; 
	long tid; 
	int rc; 
	
	struct timeval  start, end;
	gettimeofday(&start, NULL);
	long loop_count = total_operations/threadc;	
	double tstart = start.tv_sec+(start.tv_usec/1000000.0);	
	double th_value = 0;

	for (tid = 0; tid < threadc; tid++) 
	{
		switch(strat)
		{
			case 1:
				th_value = 588.80;
				rc = pthread_create(&threads[tid], NULL, qp_test, (void *) loop_count);
				break;
			case 2:
				th_value = 294.40;
				rc = pthread_create(&threads[tid], NULL, hp_test, (void *) loop_count);
				break;
			case 3:	
				th_value = 147.20;		
				rc = pthread_create(&threads[tid], NULL, sp_test, (void *) loop_count);
				break;
			case 4:
				th_value = 73.60;
				rc = pthread_create(&threads[tid], NULL, dp_test, (void *) loop_count);
				break;	
			default:
				printf("This is default");
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

	double throughput = total_operations / (time * (double) 1e9);
	
	double efficiency = (throughput/ th_value) * (double)100; 


	printf("%s\t\t %d\t\t %f\t\t %f\t\t %f\n",strat_label, threadc, throughput , th_value, efficiency);

	printf("Writing output in a file...\n");
	FILE *fp = fopen(outfile,"r");
	if (fp == NULL)
	{
		initialize_out_file(outfile);
	} 
	
		FILE *fp2 = fopen(outfile,"a");
		fprintf(fp2,"%s\t\t %d\t\t %f\t\t %f\t\t %f\n", strat_label,threadc,throughput,th_value,efficiency);
		fclose(fp2);
	printf("Test Complete...\n");
	
}

void remove_newline_ch(char *line)
{
    int new_line = strlen(line) -1;
    if (line[new_line] == '\n')
        line[new_line] = '\0';
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
		char * line = NULL;
		size_t len = 0;
		char *strat_label;
		char *thread_count;

		getline(&strat_label,&len,fp);
		getline(&thread_count,&len,fp);

		int threadc = atoi(thread_count);
		remove_newline_ch(strat_label);
		

		if(threadc == NULL || strat_label == NULL)
		{
			printf("Input file is incorrect.\n");
			
		}
		else
		{
			int strat = 0; 
			if (strcmp(strat_label, "QP") == 0) 
			{
			  	strat = 1;
			}
			else if (strcmp(strat_label, "HP") == 0) 
			{
			  	strat = 2;
			}
			else if (strcmp(strat_label, "SP") == 0) 
			{
			  	strat = 3;
			}
			else if (strcmp(strat_label, "DP") == 0) 
			{
			  	strat = 4;
			}

			if(strat == 0)
			{
				printf("Input file is incorrect.\n");
				return 0;
			}
			else
			{
				printf("Your input -> Workload is %s and, thread count is %d.\n",strat_label, threadc );
				
				char out_file[100];				
				strcat(out_file,"output/cpu_");
				strcat(out_file,strat_label);
				strcat(out_file,"_");
				remove_newline_ch(thread_count);
				strcat(out_file,thread_count);
				strcat(out_file,"thread.out.dat");

				printf("Out file is %s\n",out_file);

	

				run_test(strat,threadc,strat_label,out_file);
			}	
			 
			fclose(fp);
		}

	}

	return 0;
}










