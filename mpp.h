#pragma once
#include <math.h>

class mpp {
public:
    static double calculatePP(double difficulty, int score, double od, int note_count, int mods = 0, bool debug = false) {
        if (score > 1000000) return -1;

        double score_rate = 1;

        bool mod_combo[3] = { false, false, false };
        if (mods != 0) { //boost score to be calculable (will require correct star rating for the HT and EZ mods)
            if (mods & 1 << 0) { mod_combo[0] = true; score_rate *= 0.5; } //NF
            if (mods & 1 << 1) { mod_combo[1] = true; score_rate *= 0.5; } //EZ
            if (mods & 1 << 8) { mod_combo[2] = true; score_rate *= 0.5; } //HT
        }

        double real_score = score / score_rate;
        if (real_score > 1000000) return -1;

        double hitwindow300 = 34 + 3 * (fmin(10, fmax(0, 10 - od)));
        double strain = pow((5 * fmax(1, difficulty / 0.2) - 4), 2.2) / 135 * (1 + 0.1 * fmin(1, note_count / 1500));

        if (real_score <= 500000)
            strain *= 0.1 * (real_score / 500000); //strain = 0;

        else if (real_score <= 600000)
            strain *= 0.3 * (real_score - 500000) / 100000;

        else if (real_score <= 700000)
            strain *= 0.3 + (real_score - 600000) / 100000 * 0.25;

        else if (real_score <= 800000)
            strain *= 0.55 + (real_score - 700000) / 100000 * 0.2;

        else if (real_score <= 900000)
            strain *= 0.75 + (real_score - 800000) / 100000 * 0.15;

        else
            strain *= 0.9 + (real_score - 900000) / 100000 * 0.1;

        double accuracy = fmax(0, 0.2 - ((hitwindow300 - 34) * 0.006667)) * strain * pow((fmax(0, real_score - 960000) / 40000), 1.1);

        double pp_multiplier = 0.8;
        if (mod_combo[0]) pp_multiplier *= 0.9; //NF
        if (mod_combo[1]) pp_multiplier *= 0.5; //EZ

        if (debug) {
            printf("DIFF %lf*, SCORE: %d, OD: %lf, NOTES: %d, MODS: %d\n", difficulty, score, od, note_count, mods);
            printf("NF: %d EZ: %d HT: %d\nscore_rate: %lf\nreal_score: %lf\nhit_window300: %lf\nstrain: %lf\npp_multiplier: %lf\naccuracy: %lf\n\n", mod_combo[0], mod_combo[1], mod_combo[2], score_rate, real_score, hitwindow300, strain, pp_multiplier, accuracy);
        }

        return pow(pow(strain, 1.1) + pow(accuracy, 1.1), (1 / 1.1)) * pp_multiplier;
	}
};