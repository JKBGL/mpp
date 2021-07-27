#include "mpp.h"

std::vector<std::string_view> mpp::split(std::string_view str, std::string_view delimeters) {
	std::vector<std::string_view> res;
	res.reserve(str.length() / 2);
	const char* ptr = str.data();
	size_t size = 0;
	for (const char c : str) {
		for (const char d : delimeters) {
			if (c == d) {
				res.emplace_back(ptr, size);
				ptr += size + 1;
				size = 0;
				goto next;
			}
		}
		++size;
	next: continue;
	}
	if (size) res.emplace_back(ptr, size);
	return res;
}

mpp::mpp() {};
mpp::mpp(std::string file, int mods) {
	if (mods != 0)
		setMods(mods);
	parse_osu(file);
}

//to be renamed to parse_file()
bool mpp::parse_osu(std::string file) {
	FILE *x;
	const size_t line_size = 300;
	char *curr_line = (char*)malloc(line_size);
	std::string line;

	fopen_s(&x, file.c_str(), "r");
	if (x == NULL)
		return false; //file could not be opened.

	//[SectionName]
	std::regex section_r(R"(\[(.*)\])");
	std::smatch section_m;
	std::string current_section = "";

	//x,y,time,type,hitSound,endTime:useless crap ...
	std::regex note_r(R"((\d+),\d+,(\d+),(\d+),\d+,(\d+))");
	//r1 x
	//r2 time
	//r3 if 128 = slider
	//r4 endTime

	int slider_count = 0;

	//section metadata stuff
	size_t pos;
	std::string key;
	std::string val;

	// Reference
	using mpp_structs::note;

	//temp array used in splitting
	int xm[6] = {0,0,0,0,0,0};
	int tkp = 0; //token pos

	int stop_read = 0;
	while (fgets(curr_line, line_size, x) != NULL) {
		curr_line[strlen(curr_line) - 1] = '\0'; //fix the new way for reading lines by removing the \n char
		line = curr_line; // char to string

		//try matching every line for a section
		if (std::regex_search(line, section_m, section_r)) {
			current_section = section_m.str(1); //set current section
			//if (_debug) printf("DEBUG: FOUND SECTION >> %s\n", current_section.c_str());
		}

		//for debugging
		std::string debug = "";

		//Difficulty section
		if (current_section == "Difficulty") {
			pos = line.find(":");
			key = line.substr(0, pos);
			val = line.substr(pos + 1);

			if (key == "CircleSize") {
				bmap_data.keys = atoi(val.c_str());
				debug = "CS";

			} else if (key == "OverallDifficulty") {
				bmap_data.od = atof(val.c_str());
				debug = "OD";

			} else if (key == "HPDrainRate") {
				bmap_data.hp = atof(val.c_str());
				debug = "HP";

			}

		//Metadata section
		} else if (current_section == "Metadata") {
			pos = line.find(":");
			key = line.substr(0, pos);
			val = line.substr(pos + 1);

			if (key == "Title") {
				bmap_data.title = val;
				debug = "Ttitle";

			} else if (key == "Artist") {
				bmap_data.artist = val;
				debug = "Artist";

			} else if (key == "Version") {
				bmap_data.version = val;
				debug = "Version";
			}
		}

		//Oh boy, here's where it gets hard, Hit objects section
		if (current_section == "HitObjects") {
			std::smatch note_m;
			if (std::regex_search(line, note_m, note_r)) {
				int x = atoi(note_m.str(1).c_str());
				int start_t = atoi(note_m.str(2).c_str());
				int end_t = atoi(note_m.str(4).c_str());
				int key = floor((double)x * (double)bmap_data.keys / 512);

				//properly parse sliders
				if (atoi(note_m.str(3).c_str()) == 128) {
					slider_count++;
				} else
					end_t = start_t;

				note temp_note = {
						key,
						start_t,
						end_t,
						1.0,
						{0,0,0,0,0,0,0,0,0,0}
				};

				notes.push_back(temp_note);
				//printf(" %s | PARSED NOTE %5.d: { key: %ld, start_t: %d, end_t: %d }\n", minispam(notes.back().key, bmap_data.keys).c_str(), notes.size(), notes.back().key, notes.back().start_t, notes.back().end_t);
			}
		}
	}
	bmap_data.note_count = notes.size();

	//free file handle and line pointer
	fclose(x);
	free(curr_line);

	printf("NOTE COUNT: %d, SLIDER COUNT: %d\n", notes.size(), slider_count);
	return true;
}

double mpp::calculateDifficulty() {
	using mpp_structs::note;

	double time_scale = 1;				//NM
	if (modarr[2]) time_scale = 0.75;	//HT
	if (modarr[3]) time_scale = 1.5;	//DT

	double strain_step = 400 * time_scale;
	double weight_decay_base = 0.9;
	double individual_decay_base = 0.125;
	double overall_decay_base = 0.3;
	double star_scaling_factor = 0.018;

	// get strain for each note
	double held_until[10] = {0,0,0,0,0,0,0,0,0,0};

	note previous_note;
	previous_note.key = -1; //invalidate previous note

	for (note &_note : notes) {
		if (previous_note.key == -1) {
			previous_note = _note;
			continue;
		} //assign first note to previous_note

		double time_elapsed = (_note.start_t - previous_note.start_t) / time_scale / 1000;
		double individual_decay = pow(individual_decay_base, time_elapsed);
		double overall_decay = pow(overall_decay_base, time_elapsed);
		double hold_factor = 1;
		double hold_addition = 0;

		for (int i = 0; i < bmap_data.keys; i++) {
			if (_note.start_t < held_until[i] && _note.end_t > held_until[i]) {
				hold_addition = 1;
			} else if (_note.end_t == held_until[i]) {
				hold_addition = 0;
			} else if (_note.end_t < held_until[i]) {
				hold_factor = 1.25;
			}

			_note.individual_strain[i] = previous_note.individual_strain[i] * individual_decay;
		}

		held_until[_note.key] = _note.end_t;

		_note.individual_strain[_note.key] += 2 * hold_factor;
		_note.overall_strain = previous_note.overall_strain * overall_decay + (1 + hold_addition) * hold_factor;

		previous_note = _note;
	}

	// get difficulty for each interval
	std::vector<double> strain_table;

	double max_strain = 0;
	double interval_end_time = strain_step;
	previous_note.key = -1; //invalidate previous note again

	for (note &_note : notes) {
		while (_note.start_t > interval_end_time) {
			strain_table.push_back(max_strain);
			if (previous_note.key == -1) {
				max_strain = 0;
			} else {
				const double individual_decay = pow(individual_decay_base, ((interval_end_time - previous_note.start_t) / 1000));
				const double overall_decay = pow(overall_decay_base, ((interval_end_time - previous_note.start_t) / 1000));
				max_strain = previous_note.individual_strain[previous_note.key] * individual_decay + previous_note.overall_strain * overall_decay;
			}
			
			interval_end_time += strain_step;
		}

		double strain = _note.individual_strain[_note.key] + _note.overall_strain;
		if (strain > max_strain) max_strain = strain;
		previous_note = _note;
	}

	// get total difficulty
	double difficulty = 0, weight = 1;
	//strain_table.sort((x, y) = > {return y - x});
	std::sort(strain_table.begin(), strain_table.end(), [](double x, double y) { return y < x; });

	for (int i = 0; i < strain_table.size(); i++) {
		difficulty += strain_table[i] * weight;
		weight *= weight_decay_base;
	}

	return difficulty * star_scaling_factor;
}

double mpp::calculatePP(int score) {
	return mpp::calculatePP(bmap_data.difficulty = calculateDifficulty(), score, bmap_data.od, bmap_data.note_count, _mods);
}

void mpp::setMods(int mods) {
	_mods = mods;

	if (mods != 0) {
		if (mods & mpp_structs::mods::NF) { modarr[0] = true; }	// NF
		if (mods & mpp_structs::mods::EZ) { modarr[1] = true; }	// EZ
		if (mods & mpp_structs::mods::HT) { modarr[2] = true; }	// HT
		if (mods & mpp_structs::mods::NC || mods & mpp_structs::mods::DT) { modarr[3] = true; }	// DT / NC
	}
}

