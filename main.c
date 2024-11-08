#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>

#define MAX_THREADS 10
#define MAX_FILES 100

char *directory_path = "/path/to/directory"; // Chemin du r�pertoire � explorer
char *target_file = "target_file.txt"; // Nom du fichier recherch�

char *files[MAX_FILES]; // Tableau pour stocker les chemins des fichiers trouv�s
int file_count = 0; // Nombre de fichiers trouv�s

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex pour synchroniser l'acc�s � la liste de fichiers

// Fonction pour ajouter un fichier � la liste partag�e
void add_file(const char *path) {
    pthread_mutex_lock(&mutex); // Verrouillage du mutex pour acc�der de mani�re s�curis�e � la liste de fichiers
    if (file_count < MAX_FILES) {
        files[file_count] = strdup(path); // Duplication du chemin du fichier pour �viter les probl�mes de m�moire
        file_count++; // Incr�menter le nombre de fichiers trouv�s
    }
    pthread_mutex_unlock(&mutex); // D�verrouillage du mutex
}

// Fonction r�cursive pour rechercher les fichiers dans le r�pertoire
void search_files(const char *path) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue; // Ignorer les r�pertoires "." et ".."
            }

            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            add_file(full_path); // Ajouter le chemin du r�pertoire � la liste

            pthread_t thread;
            pthread_create(&thread, NULL, (void *(*)(void *))search_files, (void *)strdup(full_path)); // Cr�er un thread pour explorer le sous-r�pertoire
            pthread_join(thread, NULL); // Attendre la fin du thread
        } else {
            if (strcmp(entry->d_name, target_file) == 0) {
                add_file(path); // Ajouter le chemin du fichier trouv� � la liste
            }
        }
    }

    closedir(dir); // Fermer le r�pertoire apr�s avoir termin� de le parcourir
}

int main() {
    pthread_t threads[MAX_THREADS];

    // Cr�er un thread pour explorer le r�pertoire principal
    pthread_create(&threads[0], NULL, (void *(*)(void *))search_files, (void *)directory_path);
    pthread_join(threads[0], NULL); // Attendre la fin du thread principal

    // Afficher les chemins des fichiers trouv�s
    printf("Fichiers trouv�s :\n");
    for (int i = 0; i < file_count; i++) {
        printf("%s\n", files[i]);
    }

    return 0;
}
