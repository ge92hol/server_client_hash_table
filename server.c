#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>

#define MAX_KEY_LEN 64
#define MAX_VALUE_LEN 256

typedef struct entry {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    struct entry* next;
} entry_t;

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

typedef struct {
    char operation[MAX_KEY_LEN];
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    int response_ready;
    sem_t access_semaphore;
} shm_data_t;

shm_data_t *shm_ptr;

entry_t **hash_table;

void initialize_hash_table(int size) {
    hash_table = malloc(size * sizeof(entry_t*));
    for (int i = 0; i < size; i++) {
        hash_table[i] = NULL;
    }
}

unsigned int hash(const char* key, int size) {
    unsigned int hash = 0;
    while (*key) {
        hash = ((hash << 5) + *key++) % size;
    }
    return hash % size;
}

void insert(const char* key, const char* value, int size) {
    unsigned int index = hash(key, size);

    pthread_rwlock_wrlock(&rwlock);

    entry_t* new_entry = malloc(sizeof(entry_t));
    strncpy(new_entry->key, key, MAX_KEY_LEN);
    strncpy(new_entry->value, value, MAX_VALUE_LEN);
    new_entry->next = hash_table[index];
    hash_table[index] = new_entry;

    pthread_rwlock_unlock(&rwlock);
}

char* get(const char* key, int size) {
    unsigned int index = hash(key, size);

    pthread_rwlock_rdlock(&rwlock);

    entry_t* entry = hash_table[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            pthread_rwlock_unlock(&rwlock);
            return entry->value;
        }
        entry = entry->next;
    }

    pthread_rwlock_unlock(&rwlock);
    return NULL;
}


void delete(const char* key, int size) {
    unsigned int index = hash(key, size);

    pthread_rwlock_wrlock(&rwlock); 

    entry_t* prev = NULL;
    entry_t* current = hash_table[index];

    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                hash_table[index] = current->next;
            }
            free(current);
            pthread_rwlock_unlock(&rwlock);
            return;
        }
        prev = current;
        current = current->next;
    }

    pthread_rwlock_unlock(&rwlock);
}


void* handle_request(void* arg) {
    int size = *(int*)arg;
    while (1) {
        sem_wait(&shm_ptr->access_semaphore);

        if (strcmp(shm_ptr->operation, "insert") == 0) {
            insert(shm_ptr->key, shm_ptr->value, size);
            snprintf(shm_ptr->value, MAX_VALUE_LEN, "Inserted: %s", shm_ptr->key);
        } else if (strcmp(shm_ptr->operation, "get") == 0) {
            char* result = get(shm_ptr->key, size);
            if (result) {
                snprintf(shm_ptr->value, MAX_VALUE_LEN, "Found: %s", result);
            } else {
                snprintf(shm_ptr->value, MAX_VALUE_LEN, "Not Found");
            }
        } else if (strcmp(shm_ptr->operation, "delete") == 0) {
            delete(shm_ptr->key, size);
            snprintf(shm_ptr->value, MAX_VALUE_LEN, "Deleted: %s", shm_ptr->key);
        }

        shm_ptr->response_ready = 0;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hash_table_size> <number_of_threads>\n", argv[0]);
        return 1;
    }

    int hash_table_size = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    if (hash_table_size <= 0 || num_threads <= 0) {
        fprintf(stderr, "Error: Invalid hash table size or number of threads.\n");
        return 1;
    }

    int shm_fd = shm_open("/hash_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shm_data_t));
    shm_ptr = mmap(0, sizeof(shm_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_init(&shm_ptr->access_semaphore, 1, 1);

    initialize_hash_table(hash_table_size);

    pthread_t* server_threads = malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&server_threads[i], NULL, handle_request, &hash_table_size);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(server_threads[i], NULL);
    }

    free(server_threads);
    return 0;
}
