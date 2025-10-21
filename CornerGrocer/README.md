# CornerGrocer : CS-210 Portfolio Artifact (C++17)

**Author:** Myles Slack  
**Course:** CS-210 Programming Languages  
**Environment:** macOS (CLion + CMake, C++17)

---

## Overview (Project Summary)

CornerGrocer analyzes a daily purchase log (one item per line), computes item frequencies, writes a backup file `frequency.dat`, and provides an interactive console for:
1) searching a single item’s frequency, 2) printing a full frequency table, and 3) rendering a text histogram.

This implementation goes beyond baseline requirements with:
- Case-insensitive queries and whitespace trimming  
- Typo-tolerant suggestions (“Did you mean…?”) using prefix + small edit distance  
- Multiple sort options (name A→Z, frequency high→low/low→high)  
- ANSI-colored histograms with automatic scaling and exact counts in parentheses  
- Atomic write of `frequency.dat` and clear absolute-path logging on startup

**Why this matters:** It demonstrates robust I/O, data structures (`unordered_map`), clean separation of concerns, and user-centered console UX in modern C++.

---

## Build & Run

### CLion (macOS)
- Open folder in CLion.
- **Working Directory**: set to the project root (folder with `CMakeLists.txt`).
- Build → Run.

### Command Line
```bash
mkdir build && cd build
cmake ..
cmake --build .
./CornerGrocer --input data/CS210_Project_Three_Input_File.txt
# Optional:
#   --no-color  (disable ANSI colors)
```
On launch the app prints absolute paths and creates `data/frequency.dat`.

---

## Folder Structure

```
.
├─ CMakeLists.txt
├─ src/
│  ├─ main.cpp
│  ├─ FrequencyTable.cpp
│  └─ FrequencyTable.hpp
├─ data/
│  ├─ CS210_Project_Three_Input_File.txt
│  └─ frequency.dat
└─ docs/
   └─ CornerGrocer_Design_and_Screenshots.docx
```

---

## Portfolio Reflection (Journal)

### 1) Summarize the project and what problem it was solving.
CornerGrocer ingests a raw text log of grocery purchases and turns it into clear, queryable insights: a per-item frequency table, on-demand lookups, and a visual histogram. It solves the problem of quickly understanding what customers buy most, in a format light enough to run anywhere.

### 2) What did you do particularly well?
- **Architecture & clarity:** I separated data logic (loading, normalizing, counting, sorting) into a `FrequencyTable` class and kept UI/printing in `main.cpp`.  
- **User experience:** I added case-insensitive search, friendly suggestions for typos, sortable outputs, and colorized histograms with scaling so results are legible on any terminal width.  
- **Reliability:** The program writes `frequency.dat` atomically and prints absolute paths on startup, reducing “where is my file?” confusion.

### 3) Where could you enhance your code? How would these improvements help?
- **Unit tests & CI:** Add a lightweight test suite (e.g., Catch2 or GoogleTest) to assert known counts (Cranberries=10, etc.) and wire GitHub Actions for automatic builds on push. This would improve **correctness** and **maintainability**.  
- **Config file / flags:** Externalize options (input path, color on/off, sort default) to a small config or richer CLI parsing (e.g., `cxxopts`). This improves **adaptability**.  
- **Input validation:** Harden parsing (strip BOMs, ignore non-printable characters) and logging anomalies improves **robustness** for messy real-world data.  
- **Performance scaling:** For huge files, stream counts while reporting progress; still O(n), but user feedback improves perceived performance and **usability**.

### 4) Which pieces were most challenging, and how did you overcome them? What’s in your support network now?
- **Working directory confusion:** Initially the app looked for files in the build folder. I fixed this by setting the CLion **Working Directory** to the project root and printing absolute paths.  
- **User-friendly lookups:** Deciding how to suggest near matches required a minimal edit-distance algorithm and prefix preference; I implemented a small DP-based Levenshtein and ranked prefix hits first.  
- **Support network:** Official C++ references (cppreference), CLion/CMake docs, and targeted search queries for ANSI codes and `std::filesystem`. I also relied on disciplined debugging (small repro inputs, clear log lines).

### 5) Transferable skills to other projects/coursework
- **C++17 STL fluency:** `unordered_map`, `vector`, algorithms, and `std::filesystem`.  
- **Separation of concerns:** Keeping logic and presentation isolated scales to larger apps.  
- **Defensive I/O & UX:** Validating inputs, clear errors, and human-friendly output apply to any backend/CLI tool.  
- **Version control hygiene:** Structuring a repo with clear commits, README, and reproducible builds.

### 6) How did you make this program maintainable, readable, and adaptable?
- **Maintainable:** Small functions, clear names, comments that explain *why*, not *what*.  
- **Readable:** Stable sorted views, aligned columns, counts next to histograms, short console prompts.  
- **Adaptable:** CLI flags, ANSI toggle, and a single class that cleanly exposes sorted views; easy to extend (e.g., JSON export, CSV output, or GUI wrapper later).

---

## License
Educational use for SNHU CS-210 coursework. © 2025 Myles Slack.
