#include "mpp.h"

template<char delim, typename ...T>
_inline const std::tuple<T...>& split_line(const std::string_view& line, std::tuple<T...>& v) {
	size_t i{}, size{ line.size() }, start{};
	std::apply(
		[&](auto&... value_pack) {
			(([&](auto& value) {
				if (i >= size) return;
				while (i < size && line[i] != delim) {
					++i;
					continue;
				}
				std::from_chars(line.data() + start, line.data() + std::min(i, size), value);
				start = std::min(++i, size - 1);
			} (value_pack)), ...);
		}, v
	);
	return v;
}

#define PARSE_LINE(str, ret)\
	if(line.starts_with(str##sv)){\
		decltype(ret) k;\
		std::from_chars(line.data() + sizeof(str) - 1, line.data() + line.size(), k);\
		ret = k;\
		continue;\
	}

#define PARSE_LINE_STR(str, ret)\
	if (line.starts_with(str##sv)) {\
		ret = line.substr(sizeof(str) - 1, line.size());\
		continue;\
	}

mpp::calculator::calculator() { }
mpp::calculator::calculator(char* location) {
	if (!this->parse_osu(location)) [[unlikely]] {
		printf("Failed parsing file.\n");
	}
}

bool mpp::calculator::parse_osu(char* file) {
	std::fstream x(
		file,
		std::ios::binary |
		std::ios::ate |
		std::ios::in
	);

	if (x.is_open() == 0) [[unlikely]] {
		printf("Failed opening file.\n");
		return false; //file could not be opened.
	}

	std::vector<char> read_buff;
	std::string_view mem;
	uint32_t slider_count = 0;

	// get file size
	const auto file_size { (size_t)x.tellg() };

	// return file cursor to beginning
	x.seekg(0, std::ios::beg);

	read_buff.resize(file_size);
	x.read(read_buff.data(), file_size);

	mem = std::string_view(read_buff.data(), file_size);

	//int nt = 0;
	short current_section = sections::Unknown;

	// needed for std::string_view compare
	using std::operator""sv;

	// file is opened, now loop
	for (std::string_view line; get_line(mem, line);) {

		//define line_size so we don't reuse the .size() function
		size_t line_size = line.size();

		//remove \ret from string
		if (line_size)
			line = line.substr(0, line_size -= (line[line_size - 1] == '\r'));

		//if line is empty skip
		if (line_size == 0)
			continue;

		if (line[0] == '[') [[unlikely]] {
			if (line == "[General]"sv)
				current_section = sections::General;
			else if (line == "[Difficulty]"sv)
				current_section = sections::Difficulty;
			else if (line == "[Metadata]"sv)
				current_section = sections::Metadata;
			else if (line == "[HitObjects]"sv)
				current_section = sections::HitObjects;
			else
				current_section = sections::Unknown;
			//printf("Section: %d\n", current_section);
			continue;
		}

		switch (current_section) {
			case sections::Metadata: {
				PARSE_LINE_STR("Title:", bmap_data.title);
				PARSE_LINE_STR("Artist:", bmap_data.artist);
				PARSE_LINE_STR("Version:", bmap_data.version);
				continue;
				break;
			}
			case sections::Difficulty: {
				PARSE_LINE("HPDrainRate:", bmap_data.hp);
				PARSE_LINE("CircleSize:", bmap_data.keys);
				PARSE_LINE("OverallDifficulty:", bmap_data.od);
				continue;
				break;
			};
			case sections::HitObjects: {
				std::tuple<int, int, int, int, int, int> hitobj;

				// useless y, hitSound
				const auto& [x, y, time, type, hitSound, endTime] { split_line<','>(line, hitobj) };
				int\
					key		{ (int)floor((double)x * (double)bmap_data.keys / 512) },\
					start_t	{ time },\
					end_t	{ endTime };

				// is the object a slider ?
				if (type == 128)
					slider_count++;
				else
					end_t = start_t;

				notes.push_back(
					note {
						key,
						start_t,
						end_t,
						1.0,
						{0,0,0,0,0,0,0,0,0,0}
					}
				);
				continue;
				break;
			}
			default: { continue; break; }
		}

		//nt++; // line counter, useless
	}
	bmap_data.note_count = notes.size();

	//free file handle and line pointer
	x.close();

	//map successfully loaded.
	loaded = true;

	//printf("Notes: %d, Sliders: %d\n", bmap_data.note_count, slider_count);
	return true;
}

// restrict is used to make sure we don't use the same pointer twice
bool mpp::calculator::get_line(std::string_view& __restrict memory, std::string_view& __restrict output) {
	for (size_t i = 0, size = memory.size(); i < size; ++i) {
		// shorten memory and append current line to output
		if (memory[i] == '\n') [[unlikely]] {
			output = memory.substr(0,i);
			memory = memory.substr(i + 1);
			return true;
		}
	}
	return false;
}

void mpp::calculator::setMods(int mods_int) {
	this->mods = mods_int;
}

double mpp::calculator::getDifficulty() {
	if (!this->loaded) [[unlikely]] {
		printf("No map loaded.");
		return 0;
	}

	double time_scale = 1;
	if (mods & mods::HT)
		time_scale = 0.75;
	if (mods & mods::DT)
		time_scale = 1.5;

	// there seems to be an unfixable value inconsistency between languages ???

	double strain_step = 400 * time_scale;
	const double weight_decay_base = 0.9;
	const double individual_decay_base = 0.125;
	const double overall_decay_base = 0.3;
	//const double star_scaling_factor = 0.018;
	const double star_scaling_factor = 0.01804; //attempt to make the calculator more accurate by forcing an assumed value

	// get strain for each note
	double held_until[10] = { 0,0,0,0,0,0,0,0,0,0 };

	// the previous note
	note previous_note;

	// predefine variables so we don't re-define inside the loop
	
	double time_elapsed;
	double individual_decay;
	double overall_decay;
	double hold_factor = 1;
	double hold_addition = 0;

	// difficulty set for each interval aka the strain
	std::vector<double> strain_table;

	double max_strain = 0;
	double interval_end_time = strain_step;

	double individual_decayStrain;
	double overall_decayStrain;

	for (note& n : this->notes) {

		// assign first note to previous_note
		if (previous_note.key == -1) {
			previous_note = n;
			continue;
		}

		time_elapsed = ((double)n.start_t - previous_note.start_t) / time_scale / 1000;
		individual_decay = pow(individual_decay_base, time_elapsed);
		overall_decay = pow(overall_decay_base, time_elapsed);
		hold_factor = 1;
		hold_addition = 0;

		for (int i = 0; i < this->bmap_data.keys; i++) {

			if (n.start_t < held_until[i] && n.end_t > held_until[i])
				hold_addition = 1;

			else if (n.end_t == held_until[i])
				hold_addition = 0;

			else if (n.end_t < held_until[i])
				hold_factor = 1.25;

			n.individual_strain[i] = previous_note.individual_strain[i] * individual_decay;
		}

		held_until[n.key] = n.end_t;

		n.individual_strain[n.key] += 2 * hold_factor;
		n.overall_strain = previous_note.overall_strain * overall_decay + (1 + hold_addition) * hold_factor;

		previous_note = n;
	}

	previous_note.key = -1;
	for (note& n : this->notes) { // double loop needed for stability
		while (n.start_t > interval_end_time) {
			strain_table.push_back(max_strain);
			if (previous_note.key == -1) {
				max_strain = 0;
			} else {
				individual_decayStrain = pow(individual_decay_base, ((interval_end_time - previous_note.start_t) / 1000));
				overall_decayStrain = pow(overall_decay_base, ((interval_end_time - previous_note.start_t) / 1000));
				max_strain = previous_note.individual_strain[previous_note.key] * individual_decayStrain + previous_note.overall_strain * overall_decayStrain;
			}

			interval_end_time += strain_step;
		}

		double strain = n.individual_strain[n.key] + n.overall_strain;
		if (strain > max_strain) max_strain = strain;
		previous_note = n;
	}

	// get total difficulty
	double difficulty = 0, weight = 1;
	std::sort(strain_table.begin(), strain_table.end(), [](double x, double y) { return y < x; });

	for (int i = 0; i < strain_table.size(); i++) {
		difficulty += strain_table[i] * weight;
		weight *= weight_decay_base;
	}

	bmap_data.difficulty = difficulty * star_scaling_factor;
	return bmap_data.difficulty;
}

// Calculate performance for a given map
double mpp::calculator::getPerformance(unsigned score) {
	if (score > 1000000 || score < 1 || (score > 500000 && mods & 256)) {
		printf("Invalid score.\n");
		return 0;
	}

	if (!loaded) {
		printf("No map loaded.\n");
		return 0;
	}

	if (bmap_data.difficulty == 0) {
		this->getDifficulty();
	}

	double score_rate = 1;

	//make score calculable (will require correct star rating for the HT and EZ mods)
	if (mods != 0) {
		if (mods & mods::NF) score_rate *= 0.5;
		if (mods & mods::EZ) score_rate *= 0.5;
		if (mods & mods::HT) score_rate *= 0.5;
	}

	double real_score = score / score_rate;
	if (real_score > 1000000) return -1;

	double hitwindow300 = 34 + 3 * (fmin(10, fmax(0, 10 - this->bmap_data.od)));
	double strain = pow((5 * fmax(1, this->bmap_data.difficulty / 0.2) - 4), 2.2) / 135 * (1 + 0.1 * fmin(1, (double)this->bmap_data.note_count / 1500));
	
	if (real_score <= 500000) strain *= 0.1 * (real_score / 500000); //strain = 0;
	else if (real_score <= 600000) strain *= 0.3 * (real_score - 500000) / 100000;
	else if (real_score <= 700000) strain *= 0.3 + (real_score - 600000) / 100000 * 0.25;
	else if (real_score <= 800000) strain *= 0.55 + (real_score - 700000) / 100000 * 0.2;
	else if (real_score <= 900000) strain *= 0.75 + (real_score - 800000) / 100000 * 0.15;
	else strain *= 0.9 + (real_score - 900000) / 100000 * 0.1;

	double accuracy = fmax(0, 0.2 - ((hitwindow300 - 34) * 0.006667)) * strain * pow((fmax(0, real_score - 960000) / 40000), 1.1);

	double pp_multiplier = 0.8;
	if (mods & mods::NF) pp_multiplier *= 0.9;
	if (mods & mods::EZ) pp_multiplier *= 0.5;

	return pow(pow(strain, 1.1) + pow(accuracy, 1.1), (1 / 1.1)) * pp_multiplier;
}
