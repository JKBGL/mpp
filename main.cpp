#include <stdio.h>		//printf
#include <stdlib.h>		//atof, atoi
#include <chrono>		//timer
#include "mpp.h"

int main(int argc, char** argv) {
	if (argc <= 1) {
		printf("MPP v1.2 - Usage: mpp <file_location> [<score>] [<mod_int>]\n");
		return 0;
	}
	int score = 1000000, mods = 0;
	std::string fileLocation = "";

	try {
		fileLocation = std::string(argv[1]);
		if (argc > 2)
			score = atoi(argv[2]);
		if (argc > 3)
			mods = atoi(argv[3]);
	} catch (char *m) {
		printf("Encountered an error: %s", m);
	}
	
	if (score > 1000000 || score < 1 || (score > 500000 && mods & 256)) {
		printf("Invalid score.\n");
		return 0;
	}

	if (fileLocation != "") {
		//timer
		using std::chrono::high_resolution_clock;
		using std::chrono::duration_cast;
		using std::chrono::duration;
		using std::chrono::milliseconds;

		//timer
		auto t1 = high_resolution_clock::now();

		// MPP 2
		mpp::calculator a((char*)fileLocation.c_str());
		a.setMods(mods);
		printf("Beatmap: %s - %s [%s] | %dK\n", a.bmap_data.artist.c_str(), a.bmap_data.title.c_str(), a.bmap_data.version.c_str(), a.bmap_data.keys);
		printf("Performance: %.2lf\n", a.getPerformance(score));
		printf("Difficulty: %.2lf\n", a.bmap_data.difficulty);

		/*
		// Legacy MPP
		mpp a((char*)fileLocation.c_str());
		a.setMods(mods);
		printf("Performance: %lf\n", a.calculatePP());
		*/

		//timer
		auto t2 = high_resolution_clock::now();
		duration<double, std::milli> ms_double = t2 - t1;
		printf("\nOperation took: %.2lfms.\n\n", ms_double.count());
	}

	return 0;
}