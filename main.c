#include "cbmap.h"
#include "alloc.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

struct map_test {
	char* key;
	int value;
};

struct map_test TESTS[] = {
	{ .key = "One", .value = 1 },
	{ .key = "Three", .value = 3 },
	{ .key = "Five", .value = 5 },
	{ .key = "Four", .value = 4 },
	{ .key = "Two", .value = 2 },
};

#define NTESTS (sizeof(TESTS) / sizeof(struct map_test))

struct prefix_and_count {
	const char* prefix;
	size_t count;
};

int prefix_counter(const void* key, size_t key_len, const void* value, size_t value_len, void* user_data)
{
	struct prefix_and_count* pc = (struct prefix_and_count*)user_data;
	fprintf(stdout, "%zu:%-10s:%s = %d\n", pc->count, pc->prefix, (const char*)key, *(int*)value);
	pc->count++;
	return 1;
}

int exercise_map(cbmap_t map)
{
	int result = 0;

	/* Insertion of new elements */
	for (int i = 0; i < NTESTS; i++) {
		result = cbmap_insert(map, TESTS[i].key, strlen(TESTS[i].key) + 1, &TESTS[i].value, sizeof(int));
		assert(result == 1);
	}
	/* Retrieval of elements */
	for (int i = 0; i < NTESTS; i++) {
		int* value;
		size_t value_len;
		result = cbmap_get(map, TESTS[i].key, strlen(TESTS[i].key) + 1, (void**)&value, &value_len);
		assert(result == 1);
		assert(value_len = sizeof(int));
		assert(*value == TESTS[i].value);
	}
	/* Reinsertion of same elements */
	for (int i = 0; i < NTESTS; i++) {
		result = cbmap_insert(map, TESTS[i].key, strlen(TESTS[i].key) + 1, &TESTS[i].value, sizeof(int));
		assert(result == 2);
	}

	/* Counting prefixes */
	struct prefix_and_count prefix_t = { .prefix = "T", .count = 0 };
	result = cbmap_visit_prefix(map, (uint8_t*)prefix_t.prefix, 1, prefix_counter, &prefix_t);
	assert(result == 1);
	assert(prefix_t.count == 2);

	struct prefix_and_count prefix_f = { .prefix = "F", .count = 0 };
	result = cbmap_visit_prefix(map, (uint8_t*)prefix_f.prefix, 1, prefix_counter, &prefix_f);
	assert(result == 1);
	assert(prefix_f.count == 2);

	struct prefix_and_count prefix_all = { .prefix = "ALL", .count = 0 };
	result = cbmap_visit_all(map, prefix_counter, &prefix_all);
	assert(result == 1);
	assert(prefix_all.count == NTESTS);

	/* Count */
	assert(cbmap_count(map) == NTESTS);

	/* Deletion */
	assert(cbmap_delete(map, TESTS[0].key, strlen(TESTS[0].key) + 1) == 1);
	assert(cbmap_count(map) == NTESTS - 1);

	/* Update */
	for (int i = 0; i < NTESTS; i++) {
		TESTS[i].value += 1;
		result = cbmap_insert(map, TESTS[i].key, strlen(TESTS[i].key) + 1, &TESTS[i].value, sizeof(int));
		assert(result == 2 || result == 1);
	}
	for (int i = 0; i < NTESTS; i++) {
		int* value;
		size_t value_len;
		result = cbmap_get(map, TESTS[i].key, strlen(TESTS[i].key) + 1, (void**)&value, &value_len);
		assert(result == 1);
		assert(value_len = sizeof(int));
		assert(*value == TESTS[i].value);
	}

	/* A last visit */
	prefix_all = (struct prefix_and_count) { .prefix = "PLUS_ONE", .count = 0 };
	result = cbmap_visit_all(map, prefix_counter, &prefix_all);
	assert(result == 1);
	assert(prefix_all.count == NTESTS);

	return 0;
}

int test_cbmap(int argc, char** argv)
{
	int result = 0;

	fprintf(stdout, "Testing map without key allocation...\n");
	cbmap_t map = cbmap_new_with_static_keys_and_values();
	result = exercise_map(map);
	assert(result == 0);
	cbmap_destroy(&map);
	CBM_MEM_LOG();

	fprintf(stdout, "Testing map with key allocation...\n");
	map = cbmap_new();
	result = exercise_map(map);
	assert(result == 0);
	cbmap_destroy(&map);
	CBM_MEM_LOG();

	return 0;
}

int main(int argc, char** argv)
{
	assert(test_cbmap(argc, argv) == 0);
	return 0;
}
