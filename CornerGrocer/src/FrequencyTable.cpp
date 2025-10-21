//
// Created by Myles Slack on 2025.10.19.
//
#include "FrequencyTable.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

FrequencyTable::FrequencyTable(std::string inputPath)
    : inputPath_(std::move(inputPath)) {}

string FrequencyTable::toLower(const string& s) {
    string out; out.reserve(s.size());
    for (unsigned char c : s) out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}

string FrequencyTable::trim(const string& s) {
    auto is_space = [](unsigned char c){ return std::isspace(c) != 0; };
    size_t b = 0, e = s.size();
    while (b < e && is_space(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && is_space(static_cast<unsigned char>(s[e-1]))) --e;
    return s.substr(b, e - b);
}

void FrequencyTable::load() {
    counts_.clear();
    pretty_.clear();

    ifstream fin(inputPath_);
    if (!fin.is_open()) {
        throw runtime_error("Failed to open input file: " + inputPath_);
    }

    string line;
    while (std::getline(fin, line)) {
        string item = trim(line);
        if (item.empty()) continue;
        string key  = toLower(item);
        ++counts_[key];

        // First-seenn display name wins (preserves “Cranberries” instead of “cranberries”)
        if (pretty_.find(key) == pretty_.end()) {
            pretty_[key] = item;
        }
    }
}

int FrequencyTable::countOf(const string& item) const {
    string key = toLower(trim(item));
    auto it = counts_.find(key);
    return (it == counts_.end()) ? 0 : it->second;
}

int FrequencyTable::totalPurchases() const {
    int sum = 0;
    for (const auto& kv : counts_) sum += kv.second;
    return sum;
}

int FrequencyTable::uniqueItemCount() const {
    return static_cast<int>(counts_.size());
}

vector<pair<string,int>> FrequencyTable::itemsSortedByName() const {
    vector<pair<string,int>> out;
    out.reserve(counts_.size());
    for (const auto& kv : counts_) {
        const string& key = kv.first;
        const int count = kv.second;
        out.emplace_back(pretty_.at(key), count);
    }
    sort(out.begin(), out.end(),
         [](const auto& a, const auto& b){
             //Case-insensitive name sort, stable on displays
             string al = toLower(a.first), bl = toLower(b.first);
             if (al == bl) return a.first < b.first;
             return al < bl;
         });
    return out;
}

vector<pair<string,int>> FrequencyTable::itemsSortedByFreqDesc() const {
    vector<pair<string,int>> out;
    out.reserve(counts_.size());
    for (const auto& kv : counts_) {
        const string& key = kv.first;
        out.emplace_back(pretty_.at(key), kv.second);
    }
    sort(out.begin(), out.end(),
         [](const auto& a, const auto& b){
             if (a.second != b.second) return a.second > b.second; // high -> low
             // tiebreaks by name A->Z (case-insensitive)
             string al = toLower(a.first), bl = toLower(b.first);
             if (al == bl) return a.first < b.first;
             return al < bl;
         });
    return out;
}

vector<pair<string,int>> FrequencyTable::itemsSortedByFreqAsc() const {
    auto out = itemsSortedByFreqDesc();
    std::reverse(out.begin(), out.end());
    return out;
}

void FrequencyTable::writeBackup(const string& outputPath) const {
    namespace fs = std::filesystem;

    // Ensure parent directory exists
    fs::path p(outputPath);
    if (p.has_parent_path()) {
        std::error_code ec;
        fs::create_directories(p.parent_path(), ec);
        // non- fatal if it already exists
    }

    // Writ to a temporary file, then rename for atomicity
    fs::path tmp = p;
    tmp += ".tmp";

    ofstream fout(tmp);
    if (!fout.is_open()) {
        throw runtime_error("Failed to open output file: " + tmp.string());
    }

    // Header comment with timestamp + source
    auto now = chrono::system_clock::now();
    time_t t  = chrono::system_clock::to_time_t(now);
    fout << "# frequency.dat generated " << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
         << " from " << inputPath_ << "\n";

    // Sorted by name for stable, friendly diffs
    for (const auto& row : itemsSortedByName()) {
        fout << row.first << " " << row.second << "\n";
    }
    fout.close();

    std::error_code ec;
    fs::rename(tmp, p, ec);
    if (ec) {
        // a Fallback: copy + remove temp
        fs::copy_file(tmp, p, fs::copy_options::overwrite_existing, ec);
        fs::remove(tmp, ec);
        if (ec) {
            throw runtime_error("Failed to finalize output file: " + p.string());
        }
    }

}