#include <stdio.h>		//printf
#include <stdlib.h>		//atof, atoi
#include <chrono>		//timer
#include "mpp.h"

int main(int argc, char** argv) {
	if (argc < 1) {
		printf("MPP v0.93 - Usage: mpp <file_location> <score> [<mods_int>]\n");
		// <file_location> <score> [<mods_int>]
		return 0;
	}
	int score = 0, mods = 0;
	std::string fileLocation = "";

	try {
		fileLocation = std::string(argv[1]);
		score = atoi(argv[2]);
		if (argc > 3)
			mods = atoi(argv[3]);
	} catch (char *m) {
		printf("Encountered an error: %s", m);
	}

	if (fileLocation != "") {
		//timer
		using std::chrono::high_resolution_clock;
		using std::chrono::duration_cast;
		using std::chrono::duration;
		using std::chrono::milliseconds;

		//timer
		auto t1 = high_resolution_clock::now();

		mpp testobj(fileLocation);
		testobj.setMods(mods);
		//64 dt


		double result = testobj.calculatePP(score);
		if (result != -1) {
			printf("Beatmap Info: %s - %s [%s] <KEYS: %d>\nMods: %s\nPerformance: %lf\n",
				testobj.bmap_data.artist.c_str(),
				testobj.bmap_data.title.c_str(),
				testobj.bmap_data.version.c_str(),
				testobj.bmap_data.keys,
				mpp::modsStr(mods).c_str(),
				result
			);
			printf("DIFFICULTY CALC: %.2lf\n", testobj.calculateDifficulty());
		} else {
			printf("Invalid score.");
		}

		//timer
		auto t2 = high_resolution_clock::now();
		duration<double, std::milli> ms_double = t2 - t1;
		printf("\nOperation took: %.2lfms.\n\n", ms_double.count());

	}

	return 0;
}