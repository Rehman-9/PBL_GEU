#ifndef VISUALIZER_SERVICE_HPP
#define VISUALIZER_SERVICE_HPP

#include <vector>
#include <string>
#include <utility>
#include <memory>
#include "models/Subject.hpp"
#include "models/Topic.hpp"
#include "services/SubjectManager.hpp"
#include "services/TopicManager.hpp"
#include "services/PriorityEngine.hpp"
#include "nlohmann/json.hpp"

class VisualizerService {
public:
    // ==========================================
    // 1. MAX HEAP TRACE GENERATION
    // ==========================================
    // Items are (label, priorityScore).
    // Demonstrates full lifecycle: initial state -> sequential insertions
    // with sift-up comparisons and swaps -> top extraction with sift-down.
    static nlohmann::json generateHeapTrace(
        const std::vector<std::pair<std::string, double>>& items,
        bool includeExtract = true
    );

    // ==========================================
    // 2. MERGE SORT TRACE GENERATION
    // ==========================================
    // Takes numerical values and corresponding labels.
    // Produces deterministic trace of recursive divide-and-conquer steps,
    // subarray boundaries, element comparisons, buffer movements, and sorted markers.
    static nlohmann::json generateMergeSortTrace(
        const std::vector<double>& values,
        const std::vector<std::string>& labels = {}
    );

    // ==========================================
    // 3. LINEAR SEARCH TRACE GENERATION
    // ==========================================
    // Sequential search over elements checking for query matches.
    // Records comparisons, pointer movements, and match states.
    static nlohmann::json generateLinearSearchTrace(
        const std::vector<std::string>& items,
        const std::string& query
    );

    // ==========================================
    // 4. BINARY SEARCH TRACE GENERATION
    // ==========================================
    // Shows preparatory Merge Sort step if unsorted, then logarithmic binary search
    // halving intervals with low, mid, high pointers.
    static nlohmann::json generateBinarySearchTrace(
        const std::vector<std::string>& items,
        const std::string& target,
        bool performPrepSort = true
    );

    // ==========================================
    // 5. PRIORITY CALCULATION BREAKDOWN
    // ==========================================
    // Exposes exact 5-factor mathematical breakdown using Stage 4 PriorityEngine.
    static nlohmann::json generatePriorityDetails(
        const SubjectManager& subjectManager,
        const TopicManager& topicManager,
        const std::string& topicId = ""
    );

    // Educational sample priority breakdown
    static nlohmann::json generateSamplePriorityDetails();
};

#endif // VISUALIZER_SERVICE_HPP
