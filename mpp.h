#include <vector>
#include <iostream>
#include <charconv> //from_chars
#include <fstream>
#include <algorithm>
#include <string_view>

namespace mpp {
    struct note {
        int key = -1;
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

    enum sections {
        Unknown,
        General,
        Difficulty,
        Metadata,
        HitObjects
    };

    enum mods {
        NF = 1 << 0,
        EZ = 1 << 1,
        HT = 1 << 8,
        DT = 1 << 6,
        NC = 1 << 9
    };

    class calculator {
        private:
            int mods = 0;
            bool loaded = false; // is the map loaded ?
            std::vector<note> notes;
            bool parse_osu(char*);
            bool get_line(std::string_view& __restrict, std::string_view& __restrict);
        public:
            beatmap_data bmap_data;
            calculator();
            calculator(char*);
            void setMods(int);
            double getDifficulty();
            double getPerformance(unsigned = 1000000);
    };
}