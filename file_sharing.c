#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>

#define SHM_NAME "/shm_file_sharing"
#define SEM_NAME "/sem_file_sharing"
#define MAX_FILE_SIZE 1024

typedef struct {
    char filename[256];
    char content[MAX_FILE_SIZE];
    int size;
} SharedFile;

int main() {
    int shm_fd;
    SharedFile *shared_file;
    sem_t *sem;

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(SharedFile));
    shared_file = mmap(0, sizeof(SharedFile), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);


    sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);

    if (fork() == 0) {  
        while (1) {
            sem_wait(sem);
            if (shared_file->size > 0) {
                printf("Citac: Procitan fajl '%s' velicine %d bajtova\n", shared_file->filename, shared_file->size);
                printf("Sadrzaj: %s\n", shared_file->content);
                shared_file->size = 0;  
            }
            sem_post(sem);
            sleep(1);
        }
    } else {  
        char filename[256];
        char content[MAX_FILE_SIZE];
        
        while (1) {
            printf("Unesite ime fajla za deljenje (ili q za izlaz): ");
            scanf("%s", filename);
            
            if (strcmp(filename, "q") == 0) break;

            printf("Unesite sadrzaj fajla: ");
            scanf(" %[^\n]", content);

            sem_wait(sem);
            strcpy(shared_file->filename, filename);
            strcpy(shared_file->content, content);
            shared_file->size = strlen(content);
            sem_post(sem);

            printf("Pisac: Fajl '%s' je podeljen\n", filename);
            sleep(2);
        }
    }


    sem_close(sem);
    sem_unlink(SEM_NAME);
    munmap(shared_file, sizeof(SharedFile));
    close(shm_fd);
    shm_unlink(SHM_NAME);

    return 0;
}