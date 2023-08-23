#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#pragma warning(disable : 4996)

int i_time, d_time;
int instr_cache_size, instr_block_size, instr_associativity;
int data_cache_size, data_block_size, data_associativity;

struct cache_line {
    bool valid;
    unsigned int tag;
    int cache_time;
};

struct cache_line** instr_cache;
struct cache_line** data_cache;

void init_cache(struct cache_line** cache, int num_cache_sets, int associativity) {
    for (int i = 0; i < num_cache_sets; i++) {
        cache[i] = (struct cache_line*)malloc(associativity * sizeof(struct cache_line));
        for (int j = 0; j < associativity; j++) {
            cache[i][j].valid = false;
            cache[i][j].tag = 0;
            cache[i][j].cache_time = 0;
        }
    }
}

int LRU(struct cache_line** cache, int num_cache_sets, int associativity, int index) {
    int min = INT_MAX, min_index = 0;
    for (int i = 0; i < associativity; i++) {
        if (cache[index][i].cache_time < min) {
            min = cache[index][i].cache_time;
            min_index = i;
        }
    }
    return min_index;
}

int cache_access(struct cache_line** cache, int block_size, int num_cache_sets, int associativity, unsigned int address, int access_time) {

    unsigned int index_mask = (num_cache_sets - 1), index = (address / block_size) & index_mask;
    unsigned int tag = address / (block_size * num_cache_sets);
    int line = 11;

    for (int i = 0; i < associativity; i++) {
        if (cache[index][i].valid && cache[index][i].tag == tag) {
            cache[index][i].cache_time = access_time;
            return 0;
        }
        if (cache[index][i].valid == false) line = i;
    }
    if (line == 11) line = LRU(cache, num_cache_sets, associativity, index);
    cache[index][line].tag = tag;
    cache[index][line].valid = true;
    cache[index][line].cache_time = access_time;
    return 1;
}

int main() {
    int access_type, address, ins_access_count = 0, ins_miss_count = 0, data_access_count = 0, data_miss_count = 0;
    i_time = 0, d_time = 0;
    char filename[20];

    printf("Enter filename: ");
    scanf("%s", filename);

    printf("Enter Instruction Cache Size (32KB ~ 512KB): ");
    scanf("%d", &instr_cache_size);

    printf("Enter Instruction Block Size (4B ~ 64B): ");
    scanf("%d", &instr_block_size);

    printf("Enter Instruction Cache Associativity: ");
    scanf("%d", &instr_associativity);

    printf("Enter Data Cache Size (32KB ~ 512KB): ");
    scanf("%d", &data_cache_size);

    printf("Enter Data Block Size (4B ~ 64B): ");
    scanf("%d", &data_block_size);

    printf("Enter Data Cache Associativity: ");
    scanf("%d", &data_associativity);

    instr_cache_size *= 1024; data_cache_size *= 1024;

    int num_instr_cache_sets = instr_cache_size / (instr_block_size * instr_associativity);
    int num_data_cache_sets = data_cache_size / (data_block_size * data_associativity);

    instr_cache = (struct cache_line**)malloc(num_instr_cache_sets * sizeof(struct cache_line*));
    data_cache = (struct cache_line**)malloc(num_data_cache_sets * sizeof(struct cache_line*));

    init_cache(instr_cache, num_instr_cache_sets, instr_associativity);
    init_cache(data_cache, num_data_cache_sets, data_associativity);


    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    while (!feof(fp)) {
        fscanf(fp, "%d %x\n", &access_type, &address);
        if (access_type == 2) {
            ins_access_count++;
            ins_miss_count += cache_access(instr_cache, instr_block_size, num_instr_cache_sets, instr_associativity, address, i_time);
            i_time++;
        }
        else {
            data_access_count++;
            data_miss_count += cache_access(data_cache,data_block_size, num_data_cache_sets, data_associativity, address, d_time);
            d_time++;
        }
    }

    fclose(fp);

    printf("%d %d\n", ins_miss_count, data_miss_count);

    float ins_miss_ratio = (float)ins_miss_count / ins_access_count;
    float data_miss_ratio = (float)data_miss_count / data_access_count;

    printf("Instruction Cache Miss Ratio: %.4f\n", ins_miss_ratio);
    printf("Data Cache Miss Ratio: %.4f\n", data_miss_ratio);

    for (int i = 0; i < num_instr_cache_sets; i++) {
        free(instr_cache[i]);}
    for (int i = 0; i < num_data_cache_sets; i++) {
        free(data_cache[i]);}

    free(instr_cache);
    free(data_cache);
    return 0;
}