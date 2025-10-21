#include "FrequencyTable.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <tuple>
#include <vector>

// ==== ANSI color helpers (toggle with --no-color) ======
namespace Ansi {
    static bool gUseColor = true;

    constexpr const char* reset = "\x1b[0m";
    constexpr const char* bold = "\x1b[1m";
    constexpr const char* dim = "\x1b[2m";

    // Foreground colors
    constexpr const char* fgBrightGreen = "\x1b[92m";
    constexpr const char* fgGreen = "\x1b[32m";
    constexpr const char* fgGray = "\x1b[90m";

    inline std::string maybe(const char* code) {
        return gUseColor ? std::string(code) : std::string();
    }
    inline std::string resetIf() { return gUseColor ? std::string(reset) : std::string(); }
}

// ====== Utility: safe integer input ======
int readIntInRange(int lo, int hi) {
    int x;
    for (;;) {
        if (std::cin >> x && x >= lo && x <= hi) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return x;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Please enter a number in [" << lo << "..." << hi << "]: ";
    }
}

// ====== Utility: lowercase ======
std::string toLowerCopy(const std::string& s) {
    std::string out; out.reserve(s.size());
    for (unsigned char c : s) out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}

// ====== Suggestion engine (prefix + small edit distance) ======
int editDistanceLevenshtein(const std::string& a, const std::string& b) {
    // Small, simple O(nm) DP-fine for a few dozen items.
    const size_t n = a.size(), m = b.size();
    std::vector<int> prev(m + 1), cur(m + 1);
    for (size_t j = 0; j <= m; ++j) prev[j] = static_cast<int>(j);
    for (size_t i = 1; i <= n; ++i) {
        cur[0] = static_cast<int>(i);
        for (size_t j = 1; j <= m; ++j) {
            int cost = (a[i-1] == b[j-1]) ? 0 : 1;
            cur[j] = std::min({ prev[j] + 1, cur[j-1] + 1, prev[j-1] + cost });
        }
        std::swap(prev, cur);
    }
    return prev[m];
}

std::vector<std::string> suggestionsFor(const std::string& queryLower,
                                        const std::vector<std::pair<std::string,int>>& allByName,
                                        size_t maxResults = 3)
{
    // Strategy: prefer case-insensitive prefix matches, then small edit distance (<=2)
    std::vector<std::tuple<int,bool,std::string>> ranked; // (score, isPrefix, name)
    ranked.reserve(allByName.size());

    for (const auto& row : allByName) {
        std::string nameLower = toLowerCopy(row.first);
        bool prefix = nameLower.rfind(queryLower, 0) == 0;
        int dist = editDistanceLevenshtein(queryLower, nameLower);
        // Lower score is better; prefix beats distance-only results via the boolean
        int score = prefix ? 0 : dist;
        ranked.emplace_back(score, prefix, row.first);
    }

    std::sort(ranked.begin(), ranked.end(), [](const auto& A, const auto& B){
        // Primary: score asc; Secondary: prefix=true first; Tertiary: name A→Z
        if (std::get<0>(A) != std::get<0>(B)) return std::get<0>(A) < std::get<0>(B);
        if (std::get<1>(A) != std::get<1>(B)) return std::get<1>(A) > std::get<1>(B);
        return std::get<2>(A) < std::get<2>(B);
    });

    std::vector<std::string> out;
    for (const auto& t : ranked) {
        int score; bool isPrefix; std::string name;
        std::tie(score, isPrefix, name) = t;
        if (!isPrefix && score > 2) continue; // keep only close-ish edits
        out.push_back(name);
        if (out.size() >= maxResults) break;
    }
    return out;
}

// ====== Pretty printing ======
void printHeader(const std::string& title) {
    using namespace Ansi;
    std::cout << maybe(Ansi::bold) << title << resetIf() << "\n";
}

void printSummary(int uniqueCount, int totalCount) {
    std::cout << "\n"
              << uniqueCount << " unique items, "
              << totalCount  << " total purchases.\n";
}

void printTable(const std::vector<std::pair<std::string,int>>& rows) {
    // Compute column width for names
    size_t w = 4;
    for (const auto& r : rows) w = std::max(w, r.first.size());
    for (const auto& r : rows) {
        std::cout << std::left << std::setw(static_cast<int>(w)) << r.first
                  << "  " << r.second << "\n";
    }
}

void printHistogram(const std::vector<std::pair<std::string,int>>& rows) {
    using namespace Ansi;

    if (rows.empty()) {
        std::cout << "(no data)\n";
        return;
    }
    // Determine scaling so the longest bar is ~50 chars
    int maxCount = 0;
    for (const auto& r : rows) maxCount = std::max(maxCount, r.second);
    int scale = (maxCount > 50) ? ( (maxCount + 49) / 50 ) : 1; // ceil(max/50)

    size_t w = 10;
    for (const auto& r : rows) w = std::max(w, r.first.size());

    std::cout << "Legend: * = " << scale << " purchase" << (scale > 1 ? "s" : "") << "\n";

    for (const auto& r : rows) {
        int bars = (r.second + scale - 1) / scale; // ceil(count/scale)

        // Color by frequency band
        std::string color;
        if (r.second >= std::max(8, maxCount - 2)) {
            color = Ansi::maybe(Ansi::fgBrightGreen); // top hitters
        } else if (r.second >= 5) {
            color = Ansi::maybe(Ansi::fgGreen);
        } else {
            color = Ansi::maybe(Ansi::fgGray);
        }

        std::cout << std::left << std::setw(static_cast<int>(w)) << r.first << "  "
                  << color << std::string(static_cast<size_t>(bars), '*') << Ansi::resetIf()
                  << "  (" << r.second << ")\n";
    }
}

// ====== Menu ======
void menuLoop(FrequencyTable& ft) {
    for (;;) {
        std::cout << "\n========= Corner Grocer =========\n"
                  << "(1) Search item frequency\n"
                  << "(2) Print all frequencies\n"
                  << "(3) Print histogram\n"
                  << "(4) Exit\n"
                  << "> ";

        int choice = readIntInRange(1, 4);
        if (choice == 4) {
            std::cout << "Goodbye!\n";
            return;
        }

        if (choice == 1) {
            std::cout << "Enter item name: ";
            std::string query;
            std::getline(std::cin, query);

            int count = ft.countOf(query);
            if (count > 0) {
                std::cout << query << " occurs " << count << " time"
                          << (count == 1 ? "" : "s") << ".\n";
                continue;
            }

            // Not found -> suggest
            auto allByName = ft.itemsSortedByName();
            auto sugg = suggestionsFor(toLowerCopy(query), allByName, 3);
            if (!sugg.empty()) {
                std::cout << "Item not found. Did you mean:\n";
                for (const auto& s : sugg) std::cout << "  - " << s << "\n";
            } else {
                std::cout << "Item not found.\n";
            }
        }
        else if (choice == 2) {
            std::cout << "\nSort by: (1) Name A→Z  (2) Freq high→low  (3) Freq low→high\n> ";
            int sChoice = readIntInRange(1, 3);

            std::vector<std::pair<std::string,int>> rows;
            if (sChoice == 1) rows = ft.itemsSortedByName();
            if (sChoice == 2) rows = ft.itemsSortedByFreqDesc();
            if (sChoice == 3) rows = ft.itemsSortedByFreqAsc();

            printHeader("All Frequencies");
            printTable(rows);
            printSummary(ft.uniqueItemCount(), ft.totalPurchases());
        }
        else if (choice == 3) {
            std::cout << "\nHistogram basis: (1) Name A→Z  (2) Freq high→low  (3) Freq low→high\n> ";
            int sChoice = readIntInRange(1, 3);

            std::vector<std::pair<std::string,int>> rows;
            if (sChoice == 1) rows = ft.itemsSortedByName();
            if (sChoice == 2) rows = ft.itemsSortedByFreqDesc();
            if (sChoice == 3) rows = ft.itemsSortedByFreqAsc();

            printHeader("Purchase Histogram");
            printHistogram(rows);
            printSummary(ft.uniqueItemCount(), ft.totalPurchases());
        }
    }
}

// Program entry point ======
int main(int argc, char** argv) {
    namespace fs = std::filesystem;

    // Defaults (matches SNHU-provided file name and a friendly data/ folder)
    std::string inputPath = "data/CS210_Project_Three_Input_File.txt";
    std::string backupPath = "data/frequency.dat";

    // Flags:
    //--input <path> : choose a custom input file
    //--no-color  : disable ANSI colors for plain text environments
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--no-color") {
            Ansi::gUseColor = false;
        } else if (a == "--input" && i + 1 < argc) {
            inputPath = argv[++i];
        } else {
            std::cerr << "Unknown argument: " << a << "\n";
            std::cerr << "Usage: " << argv[0] << " [--input <path>] [--no-color]\n";
            return 2;
        }
    }

    try {
        // Normalizes to absolute paths for friendly logs
        std::error_code ec;
        fs::path inputAbs  = fs::absolute(inputPath, ec);
        fs::path backupAbs = fs::absolute(backupPath, ec);

        FrequencyTable ft(inputAbs.string());
        ft.load();

        // Create frequency.dat immediately per rubric
        ft.writeBackup(backupAbs.string());

        std::cout << "Loaded input:   " << inputAbs.string()  << "\n";
        std::cout << "Wrote backup:   " << backupAbs.string() << "\n";

        menuLoop(ft);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}