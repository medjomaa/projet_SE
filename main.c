#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>

#define MAX_THREADS 10
#define MAX_FILES 100

char *directory_path = "/path/to/directory"; // Chemin du répertoire à explorer
char *target_file = "target_file.txt"; // Nom du fichier recherché

char *files[MAX_FILES]; // Tableau pour stocker les chemins des fichiers trouvés
int file_count = 0; // Nombre de fichiers trouvés

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex pour synchroniser l'accès à la liste de fichiers

// Fonction pour ajouter un fichier à la liste partagée
void add_file(const char *path) {
    pthread_mutex_lock(&mutex); // Verrouillage du mutex pour accéder de manière sécurisée à la liste de fichiers
    if (file_count < MAX_FILES) {
        files[file_count] = strdup(path); // Duplication du chemin du fichier pour éviter les problèmes de mémoire
        file_count++; // Incrémenter le nombre de fichiers trouvés
    }
    pthread_mutex_unlock(&mutex); // Déverrouillage du mutex
}

// Fonction récursive pour rechercher les fichiers dans le répertoire
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
                continue; // Ignorer les répertoires "." et ".."
            }

            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            add_file(full_path); // Ajouter le chemin du répertoire à la liste

            pthread_t thread;
            pthread_create(&thread, NULL, (void *(*)(void *))search_files, (void *)strdup(full_path)); // Créer un thread pour explorer le sous-répertoire
            pthread_join(thread, NULL); // Attendre la fin du thread
        } else {
            if (strcmp(entry->d_name, target_file) == 0) {
                add_file(path); // Ajouter le chemin du fichier trouvé à la liste
            }
        }
    }

    closedir(dir); // Fermer le répertoire après avoir terminé de le parcourir
}

int main() {
    pthread_t threads[MAX_THREADS];

    // Créer un thread pour explorer le répertoire principal
    pthread_create(&threads[0], NULL, (void *(*)(void *))search_files, (void *)directory_path);
    pthread_join(threads[0], NULL); // Attendre la fin du thread principal

    // Afficher les chemins des fichiers trouvés
    printf("Fichiers trouvés :\n");
    for (int i = 0; i < file_count; i++) {
        printf("%s\n", files[i]);
    }

    return 0;
}
