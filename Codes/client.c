#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define TRUE 1
#define BUFFER_SIZE 256
#define IP "127.0.0.1"
#define IPB "255.255.255.255"
#define MAX_INIT 100000000
#define CLIENT_SIZE 5

void handler(int signum)
{   
    write(1, "timeout!\n", 10);
}

int set_port(char* argv) {
    int port = 0;
    for (int i = 0; i < 5; i++)
		argv[i] = '0';
	port = atoi(argv);
    return port;
}

void inttostr(char *str, int num) {
	int rem;
	int len = 0;
	int n = num;
	while (n != 0) {
		len++;
		n /= 10;
	}
	memset(str, 0, sizeof(str));
	for (int i = 0; i < len; i++) {
		rem = num % 10;
		num = num / 10;
		str[len - (i + 1)] = rem + '0';
	}
    return;
}

void set_price(int arr[], int price, int client_fd)
{
    arr[0] = price;
    arr[1] = client_fd;
}

int check_end(int cnt)
{
    if(cnt >= CLIENT_SIZE) {
        write(1, "END\n", 5);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    siginterrupt(SIGALRM, 1);
    int port, sock, projs_num, udp_port, num_of_proj, user_num;
	struct sockaddr_in serv_addr;
	char buffer[BUFFER_SIZE] = {0};
    int price_clients[5][2];

    port = set_port(argv[1]);

    sock = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	inet_pton(AF_INET, IP, &serv_addr.sin_addr.s_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        write(1, "connect failed\n", 16);
        exit(EXIT_FAILURE); 
    }
	write(1, "Connection Successful.\n", 24);

    recv(sock, buffer, BUFFER_SIZE, 0);
	write(1, buffer, strlen(buffer));
    write(1, "\n", 1);
    bzero(buffer, sizeof(buffer));
    recv(sock, buffer, BUFFER_SIZE, 0);
    projs_num = atoi(buffer);
    for(int i = 0; i < projs_num; i++) {
        bzero(buffer, sizeof(buffer));
        recv(sock, buffer, BUFFER_SIZE, 0);
        write(1, buffer, strlen(buffer));
        if(i != projs_num - 1) write(1, ", ", 3);
        else write(1, "\n", 1);
    }
    bzero(buffer, sizeof(buffer));

    read(0, buffer, BUFFER_SIZE);
	num_of_proj = atoi(buffer);
	send(sock, buffer, BUFFER_SIZE, 0);
	bzero(buffer, sizeof(buffer));

    recv(sock, buffer, BUFFER_SIZE, 0);
	write(1, buffer, strlen(buffer));
    write(1, "\n", 1);
    bzero(buffer, sizeof(buffer));

    recv(sock, buffer, BUFFER_SIZE, 0);
	write(1, buffer, strlen(buffer));
    bzero(buffer, sizeof(buffer));
    recv(sock, buffer, BUFFER_SIZE, 0);
    udp_port = atoi(buffer);
	write(1, buffer, strlen(buffer));
    write(1, "\n", 1);
    bzero(buffer, sizeof(buffer));

    write(1, "You are user ", strlen("You are user "));
	recv(sock, buffer, BUFFER_SIZE, 0);
	user_num = atoi(buffer);
    write(1, buffer, strlen(buffer));
    write(1, "\n", 1);
	bzero(buffer, sizeof(buffer));
    sleep(1);

    int broadcast_sock;
	int broadcast_opt = 1;
	int opt = 1;
	int turn = user_num;
    int winner = 0, min_price = MAX_INIT, price = 0, cnt = 0, client_fd = 0;
    char str[10];
	struct sockaddr_in broadcast_addr;
	fd_set fdset;
	char broadcast_buffer[BUFFER_SIZE];
    bzero(broadcast_buffer, sizeof(broadcast_buffer));

    broadcast_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(setsockopt(broadcast_sock, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
        write(1, "setsockopt error\n", 18); 
		exit(EXIT_FAILURE);
    }
	if(setsockopt(broadcast_sock, SOL_SOCKET, SO_BROADCAST, (char *) &broadcast_opt, sizeof(broadcast_opt)) < 0) {
        write(1, "setsockopt error\n", 18); 
		exit(EXIT_FAILURE);
    }
	broadcast_addr.sin_family = AF_INET;
	broadcast_addr.sin_port = htons(udp_port);
	inet_pton(AF_INET, IPB, &broadcast_addr.sin_addr.s_addr);

	if(bind(broadcast_sock, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
        write(1, "error binding\n", 14);
        close(broadcast_sock);
		exit(EXIT_FAILURE);
    }

    if (turn != 1) {
        write(1, "It is not your turn, please wait ...", 37);
        write(1, "\n", 1);
	}
	else {
		write(1, "It is your turn, please enter your price:", 42);
        write(1, "\n", 1);
	}
    
    while (TRUE)
    {
        FD_ZERO(&fdset);
        FD_SET(0, &fdset);
		FD_SET(broadcast_sock, &fdset);
		select(broadcast_sock + 1, &fdset, NULL, NULL, NULL);
        if(check_end(cnt)) break;
        if (FD_ISSET(0, &fdset)) {
			int valread = read(0, broadcast_buffer, BUFFER_SIZE);
			if (turn == 1) {
				sendto(broadcast_sock, broadcast_buffer, sizeof(broadcast_buffer), 0, (struct sockaddr *) &broadcast_addr, sizeof(broadcast_addr));
                sleep(1);
                bzero(broadcast_buffer, sizeof(broadcast_buffer));
                inttostr(str, sock);
                strcpy(broadcast_buffer,str);
                sendto(broadcast_sock, broadcast_buffer, sizeof(broadcast_buffer), 0, (struct sockaddr *) &broadcast_addr, sizeof(broadcast_addr));
			    sleep(1);
                cnt++;
            }
			bzero(broadcast_buffer, sizeof(broadcast_buffer));
		}

		if (FD_ISSET(broadcast_sock, &fdset)) {
            recvfrom(broadcast_sock, broadcast_buffer, BUFFER_SIZE, 0, NULL, 0);
            price = atoi(broadcast_buffer);
            bzero(broadcast_buffer, sizeof(broadcast_buffer));
            recvfrom(broadcast_sock, broadcast_buffer, BUFFER_SIZE, 0, NULL, 0);
            client_fd = atoi(broadcast_buffer);
            bzero(broadcast_buffer, sizeof(broadcast_buffer));
            if (turn != 1)
            {
                turn--;
                if (turn != 1) {
                    if (turn > 0) {
                        set_price(price_clients[cnt], price, client_fd);
                        write(1, "It is not your turn, please wait ...\n", 38);
                    }
                    else {
                        set_price(price_clients[cnt - 1], price, client_fd);
                        write(1, "Your turn is over.\n", 20);
                    }
                }
                else {
                    set_price(price_clients[cnt], price, client_fd);
                    write(1, "It is your turn, please enter your price:\n", 43);
                }
                cnt++;
            }
            else {
                set_price(price_clients[cnt - 1], price, client_fd);
                write(1, "Your turn is over.\n", 20);
                turn--;
                cnt++;
            }
        }
    }
    
    for (int i = 0; i < 5; i++) {
        if (price_clients[i][0] < min_price) {
            min_price = price_clients[i][0];
            winner = price_clients[i][1];
        }
    }
    if(winner == sock) {
        write(1, "Congratulations! You win.\n", 27);
        connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        inttostr(str, num_of_proj);
        send(sock, str, strlen(str), 0);
    }

    return 0;
}
