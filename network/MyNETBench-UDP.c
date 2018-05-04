#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <memory.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

const double data_size = 1e9;
const int iterations = 100;
const char *client_data;
const char *server_data;
const int initial_port = 11155;
const double th_value = 10000;
const char *server_ip = "127.0.0.1";

typedef struct tcp_arg
{
	char *start;
	char *end;
	int block_size;
	long loop_count;
	int port_no;

}tcp_arg;

typedef struct client_arg
{
	char *start;
	char *end;
	int block_size;
	long loop_count;
	int port_no;

}client_arg;


void initialize_out_file(char * filename)
{
	FILE *fp = fopen(filename,"w");
	fputs("Protocol\t Concurrency\t BlockSize\t MyNETBenchValue\t TheoreticalValue\t MyNETBenchEfficiency\t\n",fp);
	fclose(fp);
	
}
void *start_udp_server(void *arg)
{
	struct tcp_arg *s_data = (tcp_arg *) arg;
	char *src1 = s_data->start;
	int BUFF_SIZE = s_data->block_size;
	int loop_count = s_data->loop_count;
	int port_no = s_data->port_no;

	


	struct sockaddr_in si_me, si_other;
     
    	int sockfd, i, slen = sizeof(si_other) , recv_len;
    	char buf[BUFF_SIZE];
     
    	if ((sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    	{
        	pthread_exit(NULL);
    	}
     
	memset((char *) &si_me, 0, sizeof(si_me));
	     
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port_no);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    	//bind socket to port
	if( bind(sockfd , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		pthread_exit(NULL);
	}
     	
	    while(1)
	    {
		
		if ((recv_len = recvfrom(sockfd, buf, BUFF_SIZE, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			printf("ending\n");
		    	pthread_exit(NULL);
		}
		
		
	    }
 
    	close(sockfd);
	pthread_exit(NULL);
	
}


void *start_client_thread(void *arg)
{
	struct client_arg *s_data = (client_arg *) arg;
	char *src1 = s_data->start;
	int BUFF_SIZE = s_data->block_size;
	int loop_count = s_data->loop_count;
	int port_no = s_data->port_no;

	struct sockaddr_in si_other;
	int s, i, slen=sizeof(si_other);
	char buf[BUFF_SIZE];
	char message[BUFF_SIZE];
	 
	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{	
		pthread_exit(NULL);
	}
	
	memset((char *) &si_other, 0, sizeof(si_other));
    	si_other.sin_family = AF_INET;
    	si_other.sin_port = htons(port_no);
     
    	if (inet_aton(server_ip , &si_other.sin_addr) == 0) 
    	{

		pthread_exit(NULL);
    	}
	int r = 0;
	for(int j= 0;j<iterations;j++)
	{
		for(int i =0;i<loop_count;i++)
		{
			memset(message, 0, BUFF_SIZE);
			memcpy(message,src1+(i*BUFF_SIZE), BUFF_SIZE);
			if ((r = sendto(s, message, BUFF_SIZE, 0 , (struct sockaddr *) &si_other, slen))==-1)
			{
				pthread_exit(NULL);
			}
			
		}
		printf("Sent %d\n",j);
	}	
	
	pthread_exit(NULL);
}

void start_test( int threadc, int block_size, char *outfile, char *role)
{
	if(strcmp("S",role) == 0)
	{	
		printf("Server online..\n");
	}
	else if(strcmp("C",role) == 0)
	{
		printf("Running the test..\n");
	}

	pthread_t thread_server[threadc];
	pthread_t thread_client[threadc]; 
	long tid; 
	int rc; 

	long block = data_size/threadc;	
	long loop_count = block/block_size;

	tcp_arg *data_s = (tcp_arg*) malloc(threadc * sizeof (tcp_arg));
	client_arg *data_c = (client_arg*) malloc(threadc * sizeof (client_arg));

	struct timeval  start, end;
	gettimeofday(&start, NULL);
	double tstart = start.tv_sec+(start.tv_usec/1000000.0);	
	
	for (tid = 0; tid < threadc; tid++) 
	{
			int port_no = initial_port + tid;
			int start = tid * block;
			int end = start + block -1;

			data_s[tid].start = server_data + start;
			data_c[tid].start = client_data + start;

			

			data_s[tid].loop_count = loop_count;
			data_c[tid].loop_count = loop_count;

			data_s[tid].block_size = block_size;
			data_c[tid].block_size = block_size;
			
			data_s[tid].port_no = port_no;
			data_c[tid].port_no = port_no;
	 
			if(strcmp("S",role) == 0)
			{	
				rc = pthread_create(&thread_server[tid], NULL, start_udp_server, (void *) &data_s[tid]);
			}
			else if(strcmp("C",role) == 0)
			{
				rc = pthread_create(&thread_client[tid], NULL, start_client_thread, (void *) &data_c[tid]);
			}
		
		
	}

	for (tid = 0; tid < threadc; tid++) 
	{
		if(strcmp("S",role) == 0)
		{	
				rc = pthread_join(thread_server[tid], NULL);;
		}
		else if(strcmp("C",role) == 0)
		{
				rc = pthread_join(thread_client[tid], NULL);
		}
	}

	if(strcmp("C",role) == 0)
	{
		gettimeofday(&end, NULL);	
		double tend = end.tv_sec+(end.tv_usec/1000000.0);
		double time = tend - tstart -threadc;
		double throughput = ((double)iterations * data_size)/(time * (double)1e6);
		double efficiency = (throughput/th_value) * (double)100;
		printf("Time required is %f and T is %f\n", time,throughput);

		printf("Protocol\t Concurrency\t BlockSize\t MyNETBenchValue\t TheoreticalValue\t MyNETBenchEfficiency\t\n");
		printf("UDP\t\t %d\t\t %d\t\t %f\t\t %f\t\t %f\n", threadc, block_size, throughput , th_value, efficiency);

		printf("Writing output in a file...\n");
		FILE *fp = fopen(outfile,"r");
		if (fp == NULL)
		{
			initialize_out_file(outfile);
		} 
	
			FILE *fp2 = fopen(outfile,"a");
			fprintf(fp2,"UDP\t\t %d\t\t %d\t\t %f\t\t %f\t\t %f\n",threadc,block_size, throughput,th_value,efficiency);
			fclose(fp2);
		printf("Test Complete...\n");
	}
	
	

}


int main(int argc, char **argv)
{

	if(argc <2)
	{
		printf("Usage [input filename path] [role = S/C]\n");
		return 0;
	}	
	char* filename = argv[1];
	char* role = argv[2];
		
	FILE *fp = fopen(filename,"r");
	if (fp == NULL || role == NULL || (strcmp("S",role) == 0 && strcmp("C",role) == 0) )
	{
		printf("Invalid Input \n");
		return 0;
	}
	else
	{
		if(strcmp(role,"C") == 0)
		{
			if(argc < 4)
			{
				printf("Invalid input\nUsage [input filename path] [role = S] or [input filename path] [role = C] [server ip address] \n");
				return 0;
			}
			server_ip= argv[3];
		}
		char * line = NULL;
		char in_block_size[100];
		char in_thread_count[100];
		char strat_label[100];

		memset(in_block_size,0,100);
		memset(in_thread_count,0,100);
		memset(strat_label,0,100);		
		
		

		fscanf(fp,"%[^\n]\n", strat_label);
		fscanf(fp,"%[^\n]\n",in_block_size);
		fscanf(fp,"%[^\n]\n",in_thread_count);

		int threadc = atoi(in_thread_count);
		int block_size = atoi(in_block_size);

		

		if(threadc == NULL || strat_label == NULL || strcmp("UDP",strat_label) != 0)
		{
			printf("Input file is incorrect.\n");
			
		}
		else
		{
				printf("Your input -> Protocol is %s , thread count is %d, block size is %d.\n",strat_label, threadc,block_size);
				
				char out_file[100];		
				memset(out_file,0,100);		
				strcat(out_file,"output/network\-UDP\-");
				strcat(out_file,in_thread_count);
				strcat(out_file,"thread.out.dat");

				printf("Output file is %s\n",out_file);

				client_data = (char *) malloc(data_size);
				server_data = (char *) malloc(data_size);

				printf("Please wait! Preparing 1 GB data..\n");
	

				
				memset(client_data,'a',data_size);
				start_test(threadc,block_size,out_file,role);

				free(client_data);
				free(server_data);
			 
				fclose(fp);
		}

	}

    	return 0;
}
