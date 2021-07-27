#pragma once
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <string_view>
#include "mpp.h"

namespace mpp_structs {
    struct note {
        int key;
        int start_t;
        int end_t;
        double overall_strain = 1;
        double individual_strain[10]; //individual columns
    };

    struct beatmap_data {
        double od = 0;
        int note_count = 0;
        int keys = 0;
        double difficulty = 0;
        double hp = 0;
        std::string title;
        std::string artist;
        std::string version;
    };

    enum mods {
        NF = 1 << 0,
        EZ = 1 << 1,
        HT = 1 << 8,
        DT = 1 << 6,
        NC = 1 << 9
    };
}

class mpp {
private:
    int _mods = 0;
    bool loaded = false; //Is map data loaded?
    bool modarr[4] = { 0, 0, 0, 0 }; //NF, EZ, HT, DT
    std::vector<mpp_structs::note> notes;

    //helpers
    bool parse_osu(std::string);
    static std::vector<std::string_view> split(std::string_view str, std::string_view delimeters);

    //useless
    static std::string minispam(int cnt, int size) {
        std::string output = "";
        for (int x = 0; x < size; x++)
            output += (x == cnt) ? "* " : "  ";
        return output;
    }

public:
    mpp_structs::beatmap_data bmap_data;
    mpp();
    mpp(std::string, int = 0);
    void setMods(int = 0);
    double calculateDifficulty();
    double calculatePP(int = 1000000);

    //static definition for direct usage
    static double calculatePP(double difficulty, int score, double od, int note_count, int mods = 0) {
        if (score > 1000000) return -1;

        double score_rate = 1;

        bool mod_combo[3] = { false, false, false };
        if (mods != 0) { //boost score to be calculable (will require correct star rating for the HT and EZ mods)
            if (mods & mpp_structs::mods::NF) { mod_combo[0] = true; score_rate *= 0.5; } //NF
            if (mods & mpp_structs::mods::EZ) { mod_combo[1] = true; score_rate *= 0.5; } //EZ
            if (mods & mpp_structs::mods::HT) { mod_combo[2] = true; score_rate *= 0.5; } //HT
        }

        double real_score = score / score_rate;
        if (real_score > 1000000) return -1;

        double hitwindow300 = 34 + 3 * (fmin(10, fmax(0, 10 - od)));
        double strain = pow((5 * fmax(1, difficulty / 0.2) - 4), 2.2) / 135 * (1 + 0.1 * fmin(1, note_count / 1500));

        if (real_score <= 500000) strain *= 0.1 * (real_score / 500000); //strain = 0;
        else if (real_score <= 600000) strain *= 0.3 * (real_score - 500000) / 100000;
        else if (real_score <= 700000) strain *= 0.3 + (real_score - 600000) / 100000 * 0.25;
        else if (real_score <= 800000) strain *= 0.55 + (real_score - 700000) / 100000 * 0.2;
        else if (real_score <= 900000) strain *= 0.75 + (real_score - 800000) / 100000 * 0.15;
        else strain *= 0.9 + (real_score - 900000) / 100000 * 0.1;

        double accuracy = fmax(0, 0.2 - ((hitwindow300 - 34) * 0.006667)) * strain * pow((fmax(0, real_score - 960000) / 40000), 1.1);

        double pp_multiplier = 0.8;
        if (mod_combo[0]) pp_multiplier *= 0.9; //NF
        if (mod_combo[1]) pp_multiplier *= 0.5; //EZ

        return pow(pow(strain, 1.1) + pow(accuracy, 1.1), (1 / 1.1)) * pp_multiplier;
	}

    static std::string modsStr(int mods) {
        std::string result = "";
        if (mods != 0) {
            if (mods & mpp_structs::mods::NF) { result += "NF"; }       // NF
            if (mods & mpp_structs::mods::EZ) { result += "EZ"; }       // EZ
            if (mods & mpp_structs::mods::HT) { result += "HT"; }       // HT
            if (mods & mpp_structs::mods::NC) { result += "NC"; }       // DT
            else if (mods & mpp_structs::mods::DT) { result += "DT"; }  // NC
        }
        return result;
    }
};