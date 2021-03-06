#include "moduldb.h"

#include <algorithm>
#include <string.h>

#include "../module/modul.h"

using std::vector;
using std::pair;
using std::ostream;
using namespace CMSat;

ModulDB::ModulDB() {
    lvl10_start = 0;
}

ModulDB::~ModulDB() {
/*
    for (unsigned m = 0; m < module.size(); m++) {
        module[m].print(std::cout);
    }
*/
}

void ModulDB::newModul(unsigned level, const char* name, Modul* modul) {
    ModulEntry newModul;
    newModul.level = level;
    newModul.name = name;

    // Inputs
    for (unsigned i = 0; i < modul->getInputs().size(); i++) {
        if (i > 0 && newModul.ranges.back().first + newModul.ranges.back().second == modul->getInputs()[i]) {
            newModul.ranges.back().second += modul->getInputWidth()[i];
            continue;
        }
        newModul.ranges.push_back(pair<unsigned, unsigned>(modul->getInputs()[i], modul->getInputWidth()[i]));
    }

    // Additional Vars
    if (newModul.ranges.size() > 0 && newModul.ranges.back().first + newModul.ranges.back().second == modul->getStart()) {
        newModul.ranges.back().second += (modul->getAdditionalVarCount() - modul->getOutputNum());
    } else {
        newModul.ranges.push_back(pair<unsigned, unsigned>(modul->getStart(), (modul->getAdditionalVarCount() - modul->getOutputNum())));
    }

    // Output
    if (newModul.ranges.size() > 0 && newModul.ranges.back().first + newModul.ranges.back().second == modul->getOutput()) {
        newModul.ranges.back().second += modul->getOutputNum();
    } else {
        newModul.ranges.push_back(pair<unsigned, unsigned>(modul->getOutput(), modul->getOutputNum()));
    }

    for (vector<ModulEntry>::iterator it = module.begin(); it < module.end(); it++) {
        if ((*it).level > level) {
            module.insert(it, newModul);
            return;
        }
    }
    module.push_back(newModul);
}

ModulEntry* ModulDB::isInSingleModul(vector<Lit>& clause) {
    bool deepcheck = false;
    unsigned m = lvl10_start;

    if (m == 0) {
        for (m = 0; module[m].level < 10; m++);
        lvl10_start = m;
    }

    while (m < module.size()) {
        bool isInside;
        for (unsigned lit = 0; lit < clause.size(); lit++) {
            isInside = false;
            for (unsigned range = 0; range < module[m].ranges.size(); range++) {
                if (clause[lit].var() >= module[m].ranges[range].first && clause[lit].var() < module[m].ranges[range].first + module[m].ranges[range].second) {
                    isInside = true;
                    break;
                }
            }
            if (!isInside) break;
        }
        if (isInside) {
            if (deepcheck || strcmp(module[m].name, "Add_32")) return &module[m];

            // Es ist ein Addierer: Versuch die Klausel einem Halb-, Voll- oder Final-Addierer zuzuordnen
            deepcheck = true;
            m = 0;
            continue;
        }
        m++;
    }
    return NULL;
}

void ModulEntry::print(ostream& out) {
    out << "(" << level << ") " << name << ":";
    for (unsigned range = 0; range < ranges.size(); range++) {
        out << " " << ranges[range].first + 1 << ":" << ranges[range].second;
    }
    out << "\n";
}
