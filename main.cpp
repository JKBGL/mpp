#include <stdio.h>		//printf
#include <stdlib.h>		//atof, atoi
#include <chrono>		//timer
#include "mpp.h"

//temporary loader function for tests regarding speed with file parsing excluding the file load (mpp::FROM_STRING)
std::string loadFile(std::string file_location) {
	std::ifstream file(file_location, std::ios::in);
	std::string result = "";

	if (!file.is_open()) {
		printf("ERROR OPENING FILE: %s\n", file_location.c_str());
		return "Error opening file.\n";
	}

	file.seekg(0, std::ios::end);
	result.reserve(file.tellg());
	file.seekg(0, std::ios::beg);
	result.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	return result;
}

int main(int argc, char** argv) {
	if (argc < 5) {
		printf("MPP v0.4 - Usage: mpp <difficulty> <score> <od> <note_count> [<mods_int>] [<file_location - if set, od is not needed and can be 0>]\n");
		return 0;
	}

	double difficulty = 0, od = 0;
	int score = 0, note_count = 0, mods = 0;

	std::string fileLocation = "";

	try {
		difficulty = atof(argv[1]);
		score = atoi(argv[2]);
		od = atoi(argv[3]);
		note_count = atoi(argv[4]);
		if (argc > 5)
			mods = atoi(argv[5]);
		if (argc > 6)
			fileLocation = std::string(argv[6]);

	} catch (char *m) {
		printf("Encountered an error: %s", m);
	}

	if (fileLocation != "") {
		//timer
		using std::chrono::high_resolution_clock;
		using std::chrono::duration_cast;
		using std::chrono::duration;
		using std::chrono::milliseconds;

		std::string file_content = loadFile(fileLocation);

		//timer
		auto t1 = high_resolution_clock::now();

		mpp testobj(file_content, mpp::FROM_STRING);
		testobj.setNotesAndDifficulty(note_count, difficulty);

		double result = testobj.calculatePP(score);
		if (result != -1) {
			printf("Beatmap Info:\t%s - %s [%s] <KEYS: %d>\nPerformance:\t%lf\n",
				testobj.bmap_data.artist.c_str(),
				testobj.bmap_data.title.c_str(),
				testobj.bmap_data.version.c_str(),
				testobj.bmap_data.keys,
				result
			);
		} else {
			printf("Invalid score.");
		}

		//timer
		auto t2 = high_resolution_clock::now();
		duration<double, std::milli> ms_double = t2 - t1;
		printf("\nOperation took: %.2lfms.\n\n", ms_double.count());

	} else {
		double result = mpp::calculatePP(difficulty, score, od, note_count, mods, false);
		if (result != -1) {
			printf("%lf\n", result);
		} else {
			printf("Invalid score.");
		}
	}

	return 0;
}