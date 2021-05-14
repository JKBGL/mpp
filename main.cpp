#include <stdio.h>		//printf
#include <stdlib.h>		//atof
#include "mpp.h"

int main(int argc, char** argv) {
	if (argc < 5) {
		printf("MPP v0.4 - Usage: mpp <difficulty> <score> <od> <note_count> [<mods_int>]");
		return 0;
	}

	double difficulty = 0, od = 0;
	int score = 0, note_count = 0, mods = 0;

	try {
		difficulty = atof(argv[1]);
		score = atoi(argv[2]);
		od = atoi(argv[3]);
		note_count = atoi(argv[4]);
		if (argc > 5)
			mods = atoi(argv[5]);
	} catch (char *m) {
		printf("Encountered an error: %s", m);
	}

	double result = mpp::calculatePP(difficulty, score, od, note_count, mods, true);
	if (result != -1) {
		printf("Result: %lfpp\n", result);
	}
	else
		printf("Invalid score.");

	return 0;
}