#include "mpp.h"
#include <sstream>
#include <regex>
#include <list>

/*
std::list<std::string> mpp::splitToLines(std::string content) {
	std::list<std::string> result;
	std::stringstream ss(content);
	std::string line;

	if (!content.empty()) {
		while (std::getline(ss, line, '\n')) {
			result.push_back(line);
		}
	}
	return result;
}*/

//todo: switch this to regex ?
bool mpp::scanFileHeaders() {
	std::string line;
	std::string key;
	std::string val;
	std::stringstream x(_beatmap_file_contents);

	if (x.str().find("osu file format") == std::string::npos) {
		printf("@mpp: scanFileHeaders(), Not an osu map.\n");
		return false;
	}

	int stop_read = 0; //optimize by not reading the whole file for headers only
	while (std::getline(x, line) && stop_read < 45) {
		size_t pos = line.find(":");
		key = line.substr(0, pos);
		val = line.substr(pos + 1);

		//remove eventual spaces
		//val.erase(std::remove(val.begin(), val.end(), ' '), val.end());

		if (key == "Mode" && val != " 3") {
			printf("@mpp: scanFileHeaders(), Not a mania map.\n");
			return false;
		}

		if (key == "CircleSize") bmap_data.keys = atoi(val.c_str());
		else if (key == "OverallDifficulty") bmap_data.od = atof(val.c_str());
		else if (key == "HPDrainRate") bmap_data.hp = atof(val.c_str());
		else if (key == "Title") bmap_data.title = val;
		else if (key == "Artist") bmap_data.artist = val;
		else if (key == "Version") bmap_data.version = val;

		stop_read++;
	}

	//printf("DATA:\nTitle: %s\nArtist: %s\nVersion: %s\nOD: %lf\nHP: %lf\nKEYS: %d\n", bmap_data.title.c_str(), bmap_data.artist.c_str(), bmap_data.version.c_str(), bmap_data.od, bmap_data.hp, bmap_data.keys);
	return true;
}

mpp::mpp() {};
mpp::mpp(std::string file, LOAD_TYPE load_type, int mods) {
	if (load_type == mpp::FROM_FILE)
		loadFromFile(file);
	else
		loadFromString(file);
	setMods(mods);
}

void mpp::loadFromFile(std::string file_location) {
	std::ifstream file(file_location, std::ios::in);

	if (!file.is_open()) {
		printf("@mpp: Error opening file.\n");
		return;
	}

	file.seekg(0, std::ios::end);
	_beatmap_file_contents.reserve(file.tellg());
	file.seekg(0, std::ios::beg);
	_beatmap_file_contents.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	if (scanFileHeaders())
		loaded = true;
	else
		printf("@mpp: Error parsing file headers.\n");

	//printf("File Length: %d\n\n", _beatmap_file_contents.length());
}

void mpp::loadFromString(std::string file_contents) {
	_beatmap_file_contents = file_contents;
	if (scanFileHeaders())
		loaded = true;
	else
		printf("@mpp: Error parsing file headers.\n");

	//printf("File Length: %d\n\n", _beatmap_file_contents.length());
}

//todo, eventually
double mpp::calculateDifficulty() { return 0.0; }

double mpp::calculatePP(int score, bool debug) {

	if (debug)
		printf("@mpp//@calculatePP:\n(nr)\tDifficulty: %lf\n(arg)\tScore: %d\n(r)\tOD: %lf\n(nr)\tNotes: %d\n(r)\tMods: %d\n(r)\tDebug: %d\n(r)\tMODARR: { NF: %d, EZ: %d, HT: %d, DT: %d}\n",
			bmap_data.difficulty, score, bmap_data.od, bmap_data.note_count, _mods, 0,
			modarr[0],
			modarr[1],
			modarr[2],
			modarr[3]
		);

	return mpp::calculatePP(bmap_data.difficulty, score, bmap_data.od, bmap_data.note_count, _mods, 0);
}

void mpp::setMods(int mods) {
	_mods = mods;

	if (mods != 0) {
		if (mods & 1 << 0) { modarr[0] = true; }					// NF
		if (mods & 1 << 1) { modarr[1] = true; }					// EZ
		if (mods & 1 << 8) { modarr[2] = true; }					// HT
		if (mods & 1 << 9 || mods & 1 << 6) { modarr[3] = true; }	// DT / NC
	}
}

//This is temporary
void mpp::setNotesAndDifficulty(int notes, double difficulty) {
	bmap_data.note_count = notes;
	bmap_data.difficulty = difficulty;
}

