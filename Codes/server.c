#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define TRUE 1
#define PROJ_NUMBER 5
#define BUFFER_SIZE 256
#define CLIENT_SIZE 5
#define UDP_PORT 6000
#define IP "127.0.0.1"
#define STR_SIZE 20
#define LISTEN_MSG "Server is listening and waiting for connection ...\n"
#define CONNECT_MSG "New connection! User ID is "
#define PROJ_SEL_MSG "Please select a project: "
#define MAX_LISTEN 7

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

int set_port(char* argv) {
    int port = 0;
    for (int i = 0; i < 5; i++)
		argv[i] = '0';
	port = atoi(argv);
    return port;
}

void add_client_to_proj(int project_num, int group[], int client_fd) {
    if (group[0] == 0) {
        send(client_fd, "Wrong project number!", 22, 0);
		sleep(1);
	}
    else{
        for(int i = 1; i < CLIENT_SIZE + 1; i++) {
            if(group[i] == 0) {
                group[i] = client_fd;
                break;
            }
        }
        send(client_fd, "You are added to project. Waiting for other members ...", 56, 0);
        sleep(1);
    }
    return;
}

int group_full(int group[]) {
	for (int i = 1; i < CLIENT_SIZE + 1; i++) {
		if (group[i] == 0)
			return 0;
	}
	return 1;
}

void start_action(int group[], int port) {
	char udp[10];
	char turn[10];
    char client_fd[10];
	inttostr(udp, port);
	for (int i = 1; i < CLIENT_SIZE + 1; i++) {
		send(group[i], "Started! On UDP port: ", 22, 0);
		sleep(1);
		send(group[i], udp, strlen(udp), 0);
		sleep(1);
		inttostr(turn, i);
		send(group[i], turn, strlen(turn), 0);
        sleep(1);
		group[i] = 0;
	}
    return;
}

void announce_winner(int project_num, int client_fd) {
    char str[STR_SIZE];
    write(1, "User with ID ", 14);
	inttostr(str, client_fd);
	write(1, str, strlen(str));
    write(1, "win project ", 13);
    inttostr(str, project_num);
	write(1, str, strlen(str));
    write(1, "\n", 1);
    return;
}

void delete_project(int project_num, int group[]) {
    group[0] = 0;
}

int main(int argc, char* argv[])
{
    int projects_groups[PROJ_NUMBER][CLIENT_SIZE + 1] = {{0}};
    int port, server_socket = 0, max_sd, new_socket, valread, activity, length, space, project_num;
	int opt = 1;
	struct sockaddr_in addr;
	char buffer[BUFFER_SIZE] = {0};
	char str[STR_SIZE];
    int client_size = CLIENT_SIZE;
	int udp_port = UDP_PORT;
	fd_set fdset;
	int *client_sockets;

    for(int i = 0; i < PROJ_NUMBER; i++) {
        projects_groups[i][0] = i + 1;
    }

	port = set_port(argv[1]);

	client_sockets = (int*)malloc(client_size * sizeof(int));
	for (int i = 0; i < client_size; i++)
		client_sockets[i] = 0;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, IP, &addr.sin_addr.s_addr);
    
    bind(server_socket, (struct sockaddr *)&addr, sizeof(addr));

	write(1, LISTEN_MSG, strlen(LISTEN_MSG));

	listen(server_socket, MAX_LISTEN);

    while (TRUE) {
        FD_ZERO(&fdset);
		FD_SET(server_socket, &fdset);
		max_sd = server_socket;
        for (int i = 0; i < client_size; i++) {
			if (client_sockets[i] > 0)
				FD_SET(client_sockets[i], &fdset);
			if (client_sockets[i] > max_sd)
				max_sd = client_sockets[i];
		}

        activity = select( max_sd + 1 , &fdset , NULL , NULL , NULL); 
		if ((activity < 0) && (errno!=EINTR)) 
		{ 
			write(1, "select error\n", 14); 
		}

        if (FD_ISSET(server_socket, &fdset)) {
            space = 0;
			length = sizeof(addr);
			new_socket = accept(server_socket, (struct sockaddr *)&addr, (socklen_t *)&length);

			for (int i = 0; i < client_size; i++) {
				if (client_sockets[i] == 0) {
					client_sockets[i] = new_socket;
					space = 1;
					break;
				}
			}
			if (space == 0) {
				client_size++;
				client_sockets = realloc(client_sockets, client_size * sizeof(int));
				client_sockets[client_size - 1] = new_socket;
			}

            write(1, CONNECT_MSG, strlen(CONNECT_MSG));
            inttostr(str, new_socket);
			write(1, str, strlen(str));
            write(1, "\n", 1);
			send(new_socket, PROJ_SEL_MSG, strlen(PROJ_SEL_MSG), 0);
            sleep(1);
            int cnt = 0;
            for(int i = 0; i < PROJ_NUMBER; i++) {
                if(projects_groups[i][0] != 0 && projects_groups[i][CLIENT_SIZE] == 0)
                    cnt++;
            }
            inttostr(str, cnt);
            send(new_socket, str, strlen(str), 0);
            sleep(1);
            for(int i = 0; i < PROJ_NUMBER; i++) {
                if(projects_groups[i][0] != 0 && projects_groups[i][CLIENT_SIZE] == 0){
                    inttostr(str, projects_groups[i][0]);
                    send(new_socket, str, strlen(str), 0);
                    sleep(1);
                }
            }
		}

        for (int i = 0; i < client_size; i++) {
			if (FD_ISSET(client_sockets[i], &fdset)) {
				if (!(valread = read(client_sockets[i], buffer, BUFFER_SIZE))) {
					write(1, "User with ID = ", 16);
					inttostr(str, client_sockets[i]);
					write(1, str, strlen(str));
                    write(1, " disconnected!", 15);
                    write(1, "\n", 1);
					close(client_sockets[i]);
					client_sockets[i] = 0;
				}
				else {
					project_num = atoi(buffer);
					write(1, "User with ID ", 14);
					inttostr(str, client_sockets[i]);
                    write(1, str, strlen(str));
					write(1, " selected project ", 18);
					inttostr(str, project_num);
                    write(1, str, strlen(str));
					write(1, "\n", 1);
                    if(group_full(projects_groups[project_num - 1])) {
                        announce_winner(project_num - 1, client_sockets[i]);
                        delete_project(project_num - 1, projects_groups[project_num - 1]);
                    }
                    else {
                        add_client_to_proj(project_num - 1, projects_groups[project_num - 1], client_sockets[i]);
                        if(group_full(projects_groups[project_num - 1])) {
                            udp_port++;
                            start_action(projects_groups[project_num - 1], udp_port);
                        }
                    }
				}
			}
        }
    }
    return 0;
}