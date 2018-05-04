#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

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

void *start_tcp_server(void *arg)
{
	struct tcp_arg *s_data = (tcp_arg *) arg;
	char *src1 = s_data->start;
	int BUFF_SIZE = s_data->block_size;
	int loop_count = s_data->loop_count;
	int port_no = s_data->port_no;
	char port_label[100];
	sprintf(port_label,"%d",port_no);

	int sockfd, newfd, rc;
	char buffer[BUFF_SIZE];
	struct addrinfo hints, *res;
	socklen_t addrlen;
	struct sockaddr_storage clt;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	rc = getaddrinfo(NULL, port_label, &hints, &res);
	if (rc != 0) {
		printf("Could not get address information!\n");
		pthread_exit(NULL);
	}
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		printf("Could not create socket!\n");
		pthread_exit(NULL);
	}
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		printf("Could not bind socked!");
		close(sockfd);
		pthread_exit(NULL);
	}
	if (listen(sockfd, 10) == -1) {
		printf("Could not listen to socket!");
		close(sockfd);
		pthread_exit(NULL);
	}
	newfd = accept(sockfd, (struct sockaddr *) &clt, &addrlen);
	if (newfd < 0) {
		printf("Could not accept client!\n");
		close(sockfd);
		pthread_exit(NULL);
	}	

		int i = 0;
		while(1)
		{
			if(i == (loop_count-1))
			{
				i =0;
			}
			memset(buffer, 0, BUFF_SIZE);
			rc = recv(newfd, &buffer[0], BUFF_SIZE, 0);			
			memcpy(src1+(i*BUFF_SIZE),buffer, BUFF_SIZE);
			
			i++;
		}
	
	
	freeaddrinfo(res);
	close(newfd);
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
	char port_label[100];
	sprintf(port_label,"%d",port_no);

	int sockfd, rc;
	char buffer[BUFF_SIZE];
	struct addrinfo hints, *res;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	rc = getaddrinfo(server_ip, port_label, &hints, &res);
	if (rc != 0) {
		printf("Could not get address information!\n");
		pthread_exit(NULL);
	}
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		printf("Could not create socket!\n");
		pthread_exit(NULL);
	}
	rc = connect(sockfd, res->ai_addr, res->ai_addrlen);
	if (rc < 0) {
		printf("Could not connect to server!\n");
		pthread_exit(NULL);
	}
	

	for(int j = 0;j<iterations;j++)
	{
		for (int i = 0;i<loop_count-1;i++)
		{
			memset(buffer, 0, BUFF_SIZE);
			memcpy(buffer,src1+(i*BUFF_SIZE), BUFF_SIZE);
			rc = send(sockfd, &buffer[0], BUFF_SIZE, 0);
			if(rc < 0)
			{	
				printf("fucked\n");	
				pthread_exit(NULL);
			}
			
			
		}
	}


	freeaddrinfo(res);
	close(sockfd);
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
				rc = pthread_create(&thread_server[tid], NULL, start_tcp_server, (void *) &data_s[tid]);
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
		printf("TCP\t\t %d\t\t %d\t\t %f\t\t %f\t\t %f\n", threadc, block_size, throughput , th_value, efficiency);

		printf("Writing output in a file...\n");
		FILE *fp = fopen(outfile,"r");
		if (fp == NULL)
		{
			initialize_out_file(outfile);
		} 
	
			FILE *fp2 = fopen(outfile,"a");
			fprintf(fp2,"TCP\t\t %d\t\t %d\t\t %f\t\t %f\t\t %f\n",threadc,block_size, throughput,th_value,efficiency);
			fclose(fp2);
		printf("Test Complete...\n");
	}
	
	

}


int main(int argc, char **argv)
{

	

	if(argc <2)
	{
		printf("Usage [input filename path] [role = S] or [input filename path] [role = C] [server ip address] \n");
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

		

		if(threadc == NULL || strat_label == NULL)
		{
			printf("Input file is incorrect.\n");
			
		}
		else
		{
				printf("Your input -> Protocol is %s , thread count is %d, block size is %d.\n",strat_label, threadc,block_size);
				
				char out_file[100];		
				memset(out_file,0,100);		
				strcat(out_file,"output/network\-TCP\-");
				strcat(out_file,in_thread_count);
				strcat(out_file,"thread.out.dat");

				printf("Output file is %s\n",out_file);

				client_data = (char *) malloc(data_size);
				server_data = (char *) malloc(data_size);

				printf("Please wait! Preparing 1 GB data..\n");
	

				printf("Protocol\t Concurrency\t BlockSize\t MyNETBenchValue\t TheoreticalValue\t MyNETBenchEfficiency\t\n");
				memset(client_data,'a',data_size);
				start_test(threadc,block_size,out_file,role);

				free(client_data);
				free(server_data);
			 
				fclose(fp);
		}

	}

    	return 0;
}
