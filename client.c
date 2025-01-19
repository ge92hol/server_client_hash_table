#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>

#define MAX_KEY_LEN 64
#define MAX_VALUE_LEN 256

typedef struct {
    char operation[MAX_KEY_LEN];
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    int response_ready;
    sem_t access_semaphore;
} shm_data_t;

shm_data_t *shm_ptr;

void send_request(const char* operation, const char* key, const char* value) {
    snprintf(shm_ptr->operation, MAX_KEY_LEN, "%s", operation);
    snprintf(shm_ptr->key, MAX_KEY_LEN, "%s", key);
    snprintf(shm_ptr->value, MAX_VALUE_LEN, "%s", value);
    shm_ptr->response_ready = 1; 

    sem_post(&shm_ptr->access_semaphore);

    while (shm_ptr->response_ready == 1) {
        usleep(100000); 
    }
    printf("Server Response: %s\n", shm_ptr->value);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <operation> <key> [value] ...\n", argv[0]);
        return 1;
    }

    int shm_fd = shm_open("/hash_shm", O_RDWR, 0666);
    shm_ptr = mmap(0, sizeof(shm_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    for (int i = 1; i < argc; i++) {
        char* operation = argv[i];
        char* key = argv[i+1];
        char* value = NULL;

        if (strcmp(operation, "insert") == 0) {
            if (i + 2 < argc) {
                value = argv[i+2];
                send_request(operation, key, value);
                i += 2;
            } else {
                fprintf(stderr, "For insert operation, value is required.\n");
                return 1;
            }
        } else if (strcmp(operation, "get") == 0 || strcmp(operation, "delete") == 0) {
            send_request(operation, key, NULL);
            i++;  
        } else {
            fprintf(stderr, "Unknown operation: %s\n", operation);
            return 1;
        }
    }

    return 0;
}
