#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

class FrequencyTable {
public:
    explicit FrequencyTable(std::string inputPath);

    // Parses the input file and load the file into the frequency map.
    // Throws std::runtime_error on file errors.
    void load();

    // Counts / queries
    int countOf(const std::string& item) const; // case-insensitive lookup
    int totalPurchases() const;
    int uniqueItemCount() const;

    // sorted views
    std::vector<std::pair<std::string,int>> itemsSortedByName() const;      // A->Z by display name
    std::vector<std::pair<std::string,int>> itemsSortedByFreqDesc() const;   // high->low (name tiebreak)
    std::vector<std::pair<std::string,int>> itemsSortedByFreqAsc() const;    // low-> high (name tiebreak)

    // Persist a backup as "ItemName Count", one per line.
    // hopefully create parent directories if needed.
    void writeBackup(const std::string& outputPath) const;

    // Accessors
    const std::string& inputPath() const { return inputPath_; }

private:
    static std::string toLower(const std::string& s);
    static std::string trim(const std::string& s);

    // Keep rhe items normalized (lowercase) for keys, and a pretty "display" name for printing.
    // The display name is the first-seen capitalization (e.g., "Cranberries").
    std::string inputPath_;
    std::unordered_map<std::string,int> counts_;        // key: lowercase item
    std::unordered_map<std::string,std::string> pretty_; // lowercase -> original display form
};