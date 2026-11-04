#define _CRT_SECURE_NO_WARNINGS
#include "file_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <direct.h>
#include <errno.h>
#include <stdbool.h>

static GameHistoryEntry* history_entries = NULL;
static long long history_count = 0LL;
static FilePaths paths = { .history = "game_data/game_history.txt" };

static bool ensure_directory_exists(void) {
	if(_mkdir("game_data") == -1 && errno != EEXIST) {
		return false; // Failed to create directory
	}
	return true;
}

static bool retry_open_file_with(char* const file_path, const char* const mode, FILE** fp) {
	*fp = fopen(file_path, mode);
	return *fp != NULL;
}

long long get_max_score(void) {
	if (history_entries == NULL || history_count == 0) {
		return -1; // History not loaded
	}
	long long max_score = history_entries[0].score;
	for (long long i = 1; i < history_count; i++) {
		if (history_entries[i].score > max_score) {
			max_score = history_entries[i].score;
		}
	}
	return max_score;
}

Errors save_game_history(long long score, long long game_time, long long timestamp) {
	if (!ensure_directory_exists()) {
		return FAILED_TO_BUILD_DIR;
	}
	FILE* fp = fopen(paths.history, "a");
	if (!fp) {
		return FAILED_TO_OPEN_FILE; // Failed to open file
	}
	fprintf(fp, "%lld|%lld|%lld\n", score, game_time, timestamp);
	fclose(fp);
	return SUCCESSFUL_OPERATION;
}

Errors load_game_history(void) {
	if (history_entries != NULL || history_count) {
		free(history_entries);
		history_entries = NULL;
		history_count = 0LL;
	}

	if (!ensure_directory_exists())return FAILED_TO_BUILD_DIR;

	FILE* fp = fopen(paths.history, "r");
	if (!fp) {
		bool is_open = retry_open_file_with(paths.history, "a", &fp);
		if (!is_open)return FAILED_TO_OPEN_FILE;
		fclose(fp);
		return SUCCESSFUL_OPERATION;

	}

	long long cap = 100LL; // Initial capacity for history entries

	history_entries = (GameHistoryEntry*)malloc(sizeof(GameHistoryEntry) * cap); // Allocate memory for initial capacity
	if (!history_entries) {
		fclose(fp);
		return FAILED_TO_ALLOCATE_MEMORY; // Memory allocation failed
	}

	char* line = (char*)malloc(GAME_HISTORY_LINE_SIZE * sizeof(char));
	if (!line) {
		fclose(fp);
		free(history_entries);
		history_entries = NULL;
		return FAILED_TO_ALLOCATE_MEMORY; // Memory allocation failed
	}

	while (fgets(line, GAME_HISTORY_LINE_SIZE, fp) != NULL) {
		long long temp_score, temp_game_time, temp_timestamp;
		int parsed = sscanf(line, "%lld|%lld|%lld", &temp_score, &temp_game_time, &temp_timestamp);
		if (parsed != 3) {
			fclose(fp);
			free(history_entries);
			history_entries = NULL;
			history_count = 0LL;
			free(line);
			return FOUND_ERROR_PARSING_FILE;
		}

		history_count++;
		if (history_count > cap) {
			cap += 100;
			GameHistoryEntry* new_entries = (GameHistoryEntry*)realloc(history_entries, sizeof(GameHistoryEntry) * cap);
			if (!new_entries) {
				fclose(fp);
				free(history_entries);
				history_entries = NULL;
				history_count = 0LL;
				free(line);
				return FAILED_TO_ALLOCATE_MEMORY; // Memory allocation failed
			}
			history_entries = new_entries;
		}
		history_entries[history_count - 1].score = temp_score;
		history_entries[history_count - 1].game_time = temp_game_time;
		history_entries[history_count - 1].timestamp = temp_timestamp;
	}

	fclose(fp);
	free(line);

	return SUCCESSFUL_OPERATION;
}