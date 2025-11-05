#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <iomanip>
#include <filesystem>
#include <set>
#include <assert.h>

#define INVALID_COLOR_VALUE 0x7FFF

// Represents the state of a single position.
struct BackTrackState
{
    int useValAtIndex = -1;
    std::vector<int> possibleValues;
    inline int getVal() { return possibleValues[useValAtIndex]; }
    inline bool AdvanceIndex() {
        if (useValAtIndex + 1 < possibleValues.size()) {
            useValAtIndex++;
            return true;
        }
        return false;
    }
    inline void ResetIndex() { useValAtIndex = 0; }
    bool PushColorOption(int color) {
        // did we add this already ?
        for (size_t i = 0; i < possibleValues.size(); i++) {
            if (possibleValues[i] == color) {
                return false;
            }
        }
        possibleValues.push_back(color);
        return true;
    }
    inline bool CanIncreaseIndex() { return useValAtIndex + 1 < possibleValues.size(); }
};

using StateVector = std::vector<BackTrackState>;

// Save the vector of states to a file.
// Format:
//   N
//   index useValAtIndex count val1 val2 ... valCount
bool saveState(const StateVector& states, const std::string& filename, const size_t forced_limit = 0)
{
    std::ofstream ofs(filename);
    if (!ofs)
    {
        std::cerr << "ERROR: Cannot open state file for writing: "
            << filename << "\n";
        return false;
    }

    const std::size_t N = forced_limit ? forced_limit : states.size();
    ofs << N << "\n";

    for (std::size_t i = 0; i < N; ++i)
    {
        const BackTrackState& st = states[i];
        ofs << (i + 1) << " "          // 1-based index
            << st.useValAtIndex << " "
            << st.possibleValues.size();

        for (int v : st.possibleValues)
        {
            ofs << " " << v;
        }
        ofs << "\n";
    }

    return true;
}

// Load the vector of states from a file with the above format.
bool loadState(StateVector& states, const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs)
    {
        std::cerr << "ERROR: Cannot open state file for reading: "
            << filename << "\n";
        return false;
    }

    std::size_t N;
    if (!(ifs >> N))
    {
        std::cerr << "ERROR: Failed to read N from state file: "
            << filename << "\n";
        return false;
    }

    states.assign(N, BackTrackState{});

    std::size_t index1Based;
    int useVal;
    std::size_t count;

    // We expect N lines, but just read until EOF or error.
    while (ifs >> index1Based >> useVal >> count)
    {
        if (index1Based == 0 || index1Based > N)
        {
            std::cerr << "ERROR: Invalid index in state file.\n";
            return false;
        }

        BackTrackState st;
        st.useValAtIndex = useVal;
        st.possibleValues.resize(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            if (!(ifs >> st.possibleValues[i]))
            {
                std::cerr << "ERROR: Failed to read possibleValues.\n";
                return false;
            }
        }

        states[index1Based - 1] = std::move(st);
    }

    return true;
}

bool AdvanceStateMachine(StateVector& states) {
    size_t i = 0;
    size_t calculating_stage = states.size() - 1;
    for (; i < calculating_stage; i++) {
        if (states[i].AdvanceIndex()) {
            break;
        }
    }
    // we can't generate new combinations to check
    if (i == calculating_stage) {
        return false;
    }
    for (size_t j = 0; j < i; j++) {
        states[j].ResetIndex();
    }
    return true;
}

void PrintColoringCombination(StateVector& states) {
    size_t calculating_stage = states.size() - 1;
    printf("Possible coloring combination :");
    for (size_t i = 0; i < calculating_stage; i++) {
        printf("%d=%d ", (int)(i + 1), (int)(states[i].getVal() + 1));
    }
    // last one is special
    printf("%d=%d ", (int)(calculating_stage + 1), (int)(states[calculating_stage].possibleValues[states[calculating_stage].possibleValues.size() - 1] + 1));
    printf("\n");
}

bool CheckSpecificStageColoredShur(StateVector& states, size_t calculating_stage) {
    // check combinations except the one we are coloring
    size_t z_val = calculating_stage + 1;  // actual number being colored
    size_t midpoint = z_val / 2;           // floor(z/2)
    size_t z_color = states[calculating_stage].getVal();
    for (size_t x = 1; x <= midpoint; ++x) {
        size_t y = z_val - x;               // x + y = z_val

        // indices into states (0-based)
        size_t i = x - 1;
        size_t j = y - 1;

        size_t color_i = states[i].getVal();
        size_t color_j = states[j].getVal();

        // we can't be monochromatic
        if (color_i == color_j && color_i == z_color) {
            // c + c != c for a Schur-valid coloring
            return false;
        }
    }
    return true;
}

bool CheckAndStoreValidColoringCombination(StateVector& states, std::set<int> &AvailableColors) {
    size_t calculating_stage = states.size() - 1;
    // make sure that the seed we are using is a valid combo
    for (size_t substage = 2; substage < calculating_stage; substage++) {
        if (CheckSpecificStageColoredShur(states, substage) == false) {
            return false;
        }
    }

    // make sure we can store a temp color
    states[calculating_stage].possibleValues.push_back(INVALID_COLOR_VALUE);
    states[calculating_stage].useValAtIndex++;
    int& temp_col_index = states[calculating_stage].useValAtIndex;

    // now try to extend this valid combo with a an already existing color
    bool CanColorWithCombination = false;
    std::set<int> SkipCheckingThese;
    for (const auto& try_this_color : AvailableColors) {
        // set temp color
        states[calculating_stage].possibleValues[temp_col_index] = try_this_color;
        if (CheckSpecificStageColoredShur(states, calculating_stage) == true) {
            CanColorWithCombination = true;
            printf("Adding coloring option %d to %d\n", try_this_color + 1, (int)(calculating_stage + 1));
            PrintColoringCombination(states);
            // increase store
            states[calculating_stage].possibleValues.push_back(INVALID_COLOR_VALUE);
            temp_col_index++;
            // let's not test this color anymore
            SkipCheckingThese.insert(try_this_color);
        }
    }

    // no need to spam for possible new color addition
    for (const auto& col : SkipCheckingThese) {
        AvailableColors.erase(col);
    }

    // pop last temp value
    states[calculating_stage].possibleValues.pop_back();
    temp_col_index--;

    return CanColorWithCombination;
}

int main()
{
    StateVector states(1);
    states[0].useValAtIndex = 0;
    states[0].possibleValues = { 1 - 1 };
    saveState(states, "state_N1.txt", 1);

    states.push_back({});
    states[1].useValAtIndex = 0;
    states[1].possibleValues = { 2 - 1 };
    saveState(states, "state_N2.txt", 2);

    for (int N = 4; N < 161; N++) {

        printf("Checking coloring options for %d\n", N);
        char filename[255];
#if 1
        sprintf_s(filename, "State_N%d.txt", N);
        if (std::filesystem::exists(filename)) {
            printf("This number coloring state is already calculated. Skipping recalculation\n");
            continue;
        }
#endif
        // Create a vector of BackTrackState
        StateVector states(N);

        // do we have an abandoned state ? We interrupted the calculation due to .. power outage ?
        bool HasPartialSolveState = false;
        sprintf_s(filename, "State_N%d_partial.txt", N);
        if (std::filesystem::exists(filename)) {
            loadState(states, filename);
            HasPartialSolveState = true;
        }

        // load states calculated previously
        if (HasPartialSolveState == false)
        {
            char filename[255];
            sprintf_s(filename, "State_N%d.txt", N - 1);
            if (loadState(states, filename) == false) {
                printf("Failed to load required previous solution. Aborting.\n");
                return -1;
            }
            // make sure we reset all read indexes
            for (size_t i = 0; i < N - 1; i++) {
                states[i].ResetIndex();
            }
        }

        // start calculating new state
        int calculating_stage = N - 1; // because we are 0 based everywhere
        states.push_back({});

        std::set<int> AvailableColors;
        std::set<int> RemainingColors;
        for (size_t i = 0; i < calculating_stage; i++) {
            for (size_t j = 0; j < states[i].possibleValues.size(); j++) {
                AvailableColors.insert(states[i].possibleValues[j]);
            }
        }
        RemainingColors = AvailableColors;

        bool HasColorOptions = false;
        size_t CombinationsTried = 0;
        size_t TotalCombinationsToCheck = 1;
        for (size_t i = 0; i < N - 1; i++) {
            TotalCombinationsToCheck *= states[i].possibleValues.size();
        }
        size_t PrintInterval = TotalCombinationsToCheck * 1 / 100;
        if (PrintInterval == 0) {
            PrintInterval = 100;
        }
        printf("Need to check %llu combinations. Will print progress every %llu\n", TotalCombinationsToCheck, PrintInterval);

        do {
            HasColorOptions = CheckAndStoreValidColoringCombination(states, RemainingColors);

            if (HasColorOptions == true) {
//                char filename[255];
//                sprintf_s(filename, "State_N%d_partial.txt", calculating_stage + 1);
//                saveState(states, filename);
#if 1
                if (states[calculating_stage].possibleValues.size() == AvailableColors.size()) {
                    printf("Exhausted all available colors. Abandoning further search\n");
                    break;
                }
#endif
            }

            CombinationsTried++;
            HasColorOptions = AdvanceStateMachine(states);
            if ((CombinationsTried % PrintInterval) == 0) {
                printf("\rtried %lld at %f", CombinationsTried, (double)CombinationsTried * 100.0 / (double)TotalCombinationsToCheck);
            }

        } while (HasColorOptions == true);

        // if we failed to find a solution, we will assign a new color to this stage
        if (states[calculating_stage].possibleValues.size() == 0) {
            states[calculating_stage].useValAtIndex = 0;
            states[calculating_stage].possibleValues = { calculating_stage };
            AvailableColors.insert(calculating_stage);

//            PrintColoringCombination(states);
        }

        {
            char filename[255];
            sprintf_s(filename, "State_N%d.txt", calculating_stage + 1);
            saveState(states, filename);
        }

        // info ..
        printf("We needed %d colors to color %d: ", (int)AvailableColors.size(), calculating_stage + 1);
        for (const auto &itr: AvailableColors) {
            printf("%d ", itr + 1);
        }
        printf("\n");
        size_t total_combinations_to_check = 1;
        for (size_t i = 0; i < N; i++) {
            total_combinations_to_check *= states[i].possibleValues.size();
        }
        printf("Combinations to check next time : %llu\n", total_combinations_to_check);
        printf("\n\n");
    }
    return 0;
}
