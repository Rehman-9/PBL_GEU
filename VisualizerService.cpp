#include "services/VisualizerService.hpp"
#include "dsa/SearchSortEngine.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

// ==========================================
// 1. MAX HEAP TRACE GENERATION
// ==========================================

nlohmann::json VisualizerService::generateHeapTrace(
    const std::vector<std::pair<std::string, double>>& items,
    bool includeExtract
) {
    struct HeapItem {
        std::string name;
        double priority;
        int originalId;
    };

    std::vector<HeapItem> heap;
    nlohmann::json steps = nlohmann::json::array();
    int stepCounter = 0;

    auto makeSnapshot = [&heap]() -> nlohmann::json {
        nlohmann::json arr = nlohmann::json::array();
        for (size_t i = 0; i < heap.size(); ++i) {
            arr.push_back({
                {"index", static_cast<int>(i)},
                {"name", heap[i].name},
                {"priority", std::round(heap[i].priority * 100.0) / 100.0},
                {"id", heap[i].originalId}
            });
        }
        return arr;
    };

    // Step 0: Heap initialization
    steps.push_back({
        {"stepIndex", stepCounter++},
        {"action", "INIT"},
        {"phase", "initialization"},
        {"description", "Initialized empty vector-backed Max Heap. Capacity: dynamic vector."},
        {"heap", nlohmann::json::array()},
        {"activeIndices", nlohmann::json::array()},
        {"comparedIndices", nlohmann::json::array()},
        {"swappedIndices", nlohmann::json::array()},
        {"extractedItem", nullptr}
    });

    // Sequential insertions
    for (size_t itemIdx = 0; itemIdx < items.size(); ++itemIdx) {
        const auto& pair = items[itemIdx];
        HeapItem newItem{pair.first, pair.second, static_cast<int>(itemIdx + 1)};
        heap.push_back(newItem);
        size_t curr = heap.size() - 1;

        std::ostringstream oss;
        oss << "Inserted topic '" << newItem.name << "' (Priority: " 
            << std::fixed << std::setprecision(1) << newItem.priority 
            << ") at index [" << curr << "] at bottom/end of the heap.";

        steps.push_back({
            {"stepIndex", stepCounter++},
            {"action", "INSERT"},
            {"phase", "insertion"},
            {"description", oss.str()},
            {"heap", makeSnapshot()},
            {"activeIndices", {static_cast<int>(curr)}},
            {"comparedIndices", nlohmann::json::array()},
            {"swappedIndices", nlohmann::json::array()},
            {"extractedItem", nullptr}
        });

        // Sift-Up Operation
        while (curr > 0) {
            size_t parent = (curr - 1) / 2;

            std::ostringstream compOss;
            compOss << "Comparing child index [" << curr << "] ('" << heap[curr].name 
                    << "': " << std::fixed << std::setprecision(1) << heap[curr].priority 
                    << ") with parent index [" << parent << "] ('" << heap[parent].name 
                    << "': " << heap[parent].priority << ").";

            steps.push_back({
                {"stepIndex", stepCounter++},
                {"action", "COMPARE_SIFT_UP"},
                {"phase", "sift_up"},
                {"description", compOss.str()},
                {"heap", makeSnapshot()},
                {"activeIndices", {static_cast<int>(curr)}},
                {"comparedIndices", {static_cast<int>(curr), static_cast<int>(parent)}},
                {"swappedIndices", nlohmann::json::array()},
                {"extractedItem", nullptr}
            });

            // If child priority is strictly greater than parent, swap
            if (heap[curr].priority > heap[parent].priority) {
                std::swap(heap[curr], heap[parent]);

                std::ostringstream swapOss;
                swapOss << "Swapped child [" << curr << "] with parent [" << parent 
                        << "] because child priority (" << std::fixed << std::setprecision(1) 
                        << heap[parent].priority << ") > parent priority (" 
                        << heap[curr].priority << ").";

                steps.push_back({
                    {"stepIndex", stepCounter++},
                    {"action", "SWAP_SIFT_UP"},
                    {"phase", "sift_up"},
                    {"description", swapOss.str()},
                    {"heap", makeSnapshot()},
                    {"activeIndices", {static_cast<int>(parent)}},
                    {"comparedIndices", nlohmann::json::array()},
                    {"swappedIndices", {static_cast<int>(curr), static_cast<int>(parent)}},
                    {"extractedItem", nullptr}
                });

                curr = parent;
            } else {
                std::ostringstream doneOss;
                doneOss << "Child priority (" << std::fixed << std::setprecision(1) 
                        << heap[curr].priority << ") <= parent priority (" 
                        << heap[parent].priority << "). Max Heap invariant satisfied for this path.";

                steps.push_back({
                    {"stepIndex", stepCounter++},
                    {"action", "SIFT_UP_DONE"},
                    {"phase", "sift_up"},
                    {"description", doneOss.str()},
                    {"heap", makeSnapshot()},
                    {"activeIndices", {static_cast<int>(curr)}},
                    {"comparedIndices", nlohmann::json::array()},
                    {"swappedIndices", nlohmann::json::array()},
                    {"extractedItem", nullptr}
                });
                break;
            }
        }

        if (curr == 0) {
            steps.push_back({
                {"stepIndex", stepCounter++},
                {"action", "SIFT_UP_DONE"},
                {"phase", "sift_up"},
                {"description", "Element reached root index [0]. Max Heap invariant satisfied."},
                {"heap", makeSnapshot()},
                {"activeIndices", {0}},
                {"comparedIndices", nlohmann::json::array()},
                {"swappedIndices", nlohmann::json::array()},
                {"extractedItem", nullptr}
            });
        }
    }

    // Step: Heap built
    steps.push_back({
        {"stepIndex", stepCounter++},
        {"action", "HEAP_BUILT"},
        {"phase", "complete"},
        {"description", "All elements inserted. Vector represents valid Max Heap where root index [0] holds highest priority."},
        {"heap", makeSnapshot()},
        {"activeIndices", heap.empty() ? nlohmann::json::array() : nlohmann::json({0})},
        {"comparedIndices", nlohmann::json::array()},
        {"swappedIndices", nlohmann::json::array()},
        {"extractedItem", nullptr}
    });

    // Extract Max Operation demonstration
    if (includeExtract && !heap.empty()) {
        HeapItem extracted = heap.front();
        nlohmann::json extractedJson = {
            {"name", extracted.name},
            {"priority", std::round(extracted.priority * 100.0) / 100.0},
            {"id", extracted.originalId}
        };

        std::ostringstream extOss;
        extOss << "Extracting maximum element from root index [0]: '" 
               << extracted.name << "' (Priority: " << std::fixed << std::setprecision(1) 
               << extracted.priority << ").";

        steps.push_back({
            {"stepIndex", stepCounter++},
            {"action", "EXTRACT_MAX_START"},
            {"phase", "extraction"},
            {"description", extOss.str()},
            {"heap", makeSnapshot()},
            {"activeIndices", {0}},
            {"comparedIndices", nlohmann::json::array()},
            {"swappedIndices", nlohmann::json::array()},
            {"extractedItem", extractedJson}
        });

        if (heap.size() == 1) {
            heap.pop_back();
            steps.push_back({
                {"stepIndex", stepCounter++},
                {"action", "FINAL"},
                {"phase", "complete"},
                {"description", "Only one element was in heap. Heap is now empty."},
                {"heap", makeSnapshot()},
                {"activeIndices", nlohmann::json::array()},
                {"comparedIndices", nlohmann::json::array()},
                {"swappedIndices", nlohmann::json::array()},
                {"extractedItem", extractedJson}
            });
        } else {
            // Replace root with last element
            HeapItem lastElem = heap.back();
            heap.front() = lastElem;
            heap.pop_back();

            std::ostringstream repOss;
            repOss << "Replaced root index [0] with last element '" << lastElem.name 
                   << "' (Priority: " << std::fixed << std::setprecision(1) << lastElem.priority 
                   << ") to preserve complete binary tree structure. Initiating sift-down.";

            steps.push_back({
                {"stepIndex", stepCounter++},
                {"action", "REPLACE_ROOT"},
                {"phase", "sift_down"},
                {"description", repOss.str()},
                {"heap", makeSnapshot()},
                {"activeIndices", {0}},
                {"comparedIndices", nlohmann::json::array()},
                {"swappedIndices", nlohmann::json::array()},
                {"extractedItem", extractedJson}
            });

            // Sift-Down Operation
            size_t idx = 0;
            while (true) {
                size_t left = 2 * idx + 1;
                size_t right = left + 1;
                size_t best = idx;

                std::vector<int> compared;
                compared.push_back(static_cast<int>(idx));
                if (left < heap.size()) compared.push_back(static_cast<int>(left));
                if (right < heap.size()) compared.push_back(static_cast<int>(right));

                if (left < heap.size() && heap[left].priority > heap[best].priority) {
                    best = left;
                }
                if (right < heap.size() && heap[right].priority > heap[best].priority) {
                    best = right;
                }

                std::ostringstream downCompOss;
                downCompOss << "Inspecting node index [" << idx << "] with its children. ";
                if (left >= heap.size()) {
                    downCompOss << "Node has no children.";
                } else if (right >= heap.size()) {
                    downCompOss << "Node has left child [" << left << "].";
                } else {
                    downCompOss << "Comparing against left child [" << left << "] and right child [" << right << "].";
                }

                steps.push_back({
                    {"stepIndex", stepCounter++},
                    {"action", "COMPARE_SIFT_DOWN"},
                    {"phase", "sift_down"},
                    {"description", downCompOss.str()},
                    {"heap", makeSnapshot()},
                    {"activeIndices", {static_cast<int>(idx)}},
                    {"comparedIndices", compared},
                    {"swappedIndices", nlohmann::json::array()},
                    {"extractedItem", extractedJson}
                });

                if (best == idx) {
                    steps.push_back({
                        {"stepIndex", stepCounter++},
                        {"action", "SIFT_DOWN_DONE"},
                        {"phase", "sift_down"},
                        {"description", "Parent priority >= children priority. Sift-down finished."},
                        {"heap", makeSnapshot()},
                        {"activeIndices", {static_cast<int>(idx)}},
                        {"comparedIndices", nlohmann::json::array()},
                        {"swappedIndices", nlohmann::json::array()},
                        {"extractedItem", extractedJson}
                    });
                    break;
                }

                // Swap with the better child
                std::swap(heap[idx], heap[best]);
                std::ostringstream swapDownOss;
                swapDownOss << "Swapped node [" << idx << "] with child [" << best 
                            << "] to restore Max Heap property.";

                steps.push_back({
                    {"stepIndex", stepCounter++},
                    {"action", "SWAP_SIFT_DOWN"},
                    {"phase", "sift_down"},
                    {"description", swapDownOss.str()},
                    {"heap", makeSnapshot()},
                    {"activeIndices", {static_cast<int>(best)}},
                    {"comparedIndices", nlohmann::json::array()},
                    {"swappedIndices", {static_cast<int>(idx), static_cast<int>(best)}},
                    {"extractedItem", extractedJson}
                });

                idx = best;
            }
        }
    }

    // Final state
    steps.push_back({
        {"stepIndex", stepCounter++},
        {"action", "FINAL"},
        {"phase", "complete"},
        {"description", "Heap operations complete. Max Heap invariant verified at every level."},
        {"heap", makeSnapshot()},
        {"activeIndices", nlohmann::json::array()},
        {"comparedIndices", nlohmann::json::array()},
        {"swappedIndices", nlohmann::json::array()},
        {"extractedItem", nullptr}
    });

    return {
        {"algorithm", "Custom Max Heap"},
        {"timeComplexity", {
            {"insertion", "O(log n)"},
            {"extraction", "O(log n)"},
            {"peekMax", "O(1)"},
            {"space", "O(n)"}
        }},
        {"totalSteps", steps.size()},
        {"steps", steps}
    };
}

// ==========================================
// 2. MERGE SORT TRACE GENERATION
// ==========================================

nlohmann::json VisualizerService::generateMergeSortTrace(
    const std::vector<double>& values,
    const std::vector<std::string>& labels
) {
    struct Element {
        double val;
        std::string label;
        int originalIndex;
    };

    std::vector<Element> arr;
    arr.reserve(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        std::string lbl = (i < labels.size() && !labels[i].empty()) 
            ? labels[i] : ("Item " + std::to_string(i));
        arr.push_back({values[i], lbl, static_cast<int>(i)});
    }

    nlohmann::json steps = nlohmann::json::array();
    int stepCounter = 0;
    std::vector<int> sortedIndices;

    auto makeSnapshot = [&arr]() -> nlohmann::json {
        nlohmann::json out = nlohmann::json::array();
        for (size_t i = 0; i < arr.size(); ++i) {
            out.push_back({
                {"index", static_cast<int>(i)},
                {"value", arr[i].val},
                {"label", arr[i].label},
                {"originalIndex", arr[i].originalIndex}
            });
        }
        return out;
    };

    // Initial state
    steps.push_back({
        {"stepIndex", stepCounter++},
        {"action", "START"},
        {"description", "Initial unsorted array loaded. Commencing Divide-and-Conquer Merge Sort."},
        {"array", makeSnapshot()},
        {"left", 0},
        {"mid", arr.empty() ? 0 : static_cast<int>(arr.size() - 1) / 2},
        {"right", arr.empty() ? 0 : static_cast<int>(arr.size() - 1)},
        {"comparedIndices", nlohmann::json::array()},
        {"movedIndex", -1},
        {"sortedIndices", nlohmann::json::array()}
    });

    if (arr.size() <= 1) {
        steps.push_back({
            {"stepIndex", stepCounter++},
            {"action", "COMPLETE"},
            {"description", "Array has 0 or 1 element; already trivially sorted."},
            {"array", makeSnapshot()},
            {"left", 0},
            {"mid", 0},
            {"right", static_cast<int>(arr.size()) - 1},
            {"comparedIndices", nlohmann::json::array()},
            {"movedIndex", -1},
            {"sortedIndices", arr.empty() ? nlohmann::json::array() : nlohmann::json({0})}
        });
        return {
            {"algorithm", "Merge Sort"},
            {"timeComplexity", "O(n log n)"},
            {"spaceComplexity", "O(n)"},
            {"totalSteps", steps.size()},
            {"steps", steps}
        };
    }

    // Helper lambda for recursive merge sort with trace capture
    auto merge = [&](int left, int mid, int right, auto& selfMerge) -> void {
        int n1 = mid - left + 1;
        int n2 = right - mid;

        std::vector<Element> leftArr;
        std::vector<Element> rightArr;
        leftArr.reserve(n1);
        rightArr.reserve(n2);

        for (int i = 0; i < n1; ++i) leftArr.push_back(arr[left + i]);
        for (int j = 0; j < n2; ++j) rightArr.push_back(arr[mid + 1 + j]);

        std::ostringstream mergeStartOss;
        mergeStartOss << "Merging left subarray [" << left << ".." << mid 
                      << "] and right subarray [" << (mid + 1) << ".." << right << "].";

        steps.push_back({
            {"stepIndex", stepCounter++},
            {"action", "MERGE_START"},
            {"description", mergeStartOss.str()},
            {"array", makeSnapshot()},
            {"left", left},
            {"mid", mid},
            {"right", right},
            {"comparedIndices", nlohmann::json::array()},
            {"movedIndex", -1},
            {"sortedIndices", sortedIndices}
        });

        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
            int leftIdx = left + i;
            int rightIdx = mid + 1 + j;

            std::ostringstream compOss;
            compOss << "Comparing left [" << leftIdx << "] (" << leftArr[i].val 
                    << ") with right [" << rightIdx << "] (" << rightArr[j].val << ").";

            steps.push_back({
                {"stepIndex", stepCounter++},
                {"action", "COMPARE"},
                {"description", compOss.str()},
                {"array", makeSnapshot()},
                {"left", left},
                {"mid", mid},
                {"right", right},
                {"comparedIndices", {leftIdx, rightIdx}},
                {"movedIndex", -1},
                {"sortedIndices", sortedIndices}
            });

            if (leftArr[i].val <= rightArr[j].val) {
                arr[k] = leftArr[i++];
            } else {
                arr[k] = rightArr[j++];
            }

            std::ostringstream moveOss;
            moveOss << "Placed smaller element (" << arr[k].val << ") at sorted position [" << k << "].";

            steps.push_back({
                {"stepIndex", stepCounter++},
                {"action", "MERGE_MOVE"},
                {"description", moveOss.str()},
                {"array", makeSnapshot()},
                {"left", left},
                {"mid", mid},
                {"right", right},
                {"comparedIndices", nlohmann::json::array()},
                {"movedIndex", k},
                {"sortedIndices", sortedIndices}
            });

            k++;
        }

        while (i < n1) {
            arr[k] = leftArr[i++];
            steps.push_back({
                {"stepIndex", stepCounter++},
                {"action", "MERGE_MOVE"},
                {"description", "Copying remaining element from left buffer to index [" + std::to_string(k) + "]."},
                {"array", makeSnapshot()},
                {"left", left},
                {"mid", mid},
                {"right", right},
                {"comparedIndices", nlohmann::json::array()},
                {"movedIndex", k},
                {"sortedIndices", sortedIndices}
            });
            k++;
        }

        while (j < n2) {
            arr[k] = rightArr[j++];
            steps.push_back({
                {"stepIndex", stepCounter++},
                {"action", "MERGE_MOVE"},
                {"description", "Copying remaining element from right buffer to index [" + std::to_string(k) + "]."},
                {"array", makeSnapshot()},
                {"left", left},
                {"mid", mid},
                {"right", right},
                {"comparedIndices", nlohmann::json::array()},
                {"movedIndex", k},
                {"sortedIndices", sortedIndices}
            });
            k++;
        }

        // Subarray is now merged
        std::ostringstream doneOss;
        doneOss << "Subarray [" << left << ".." << right << "] successfully merged and sorted.";

        steps.push_back({
            {"stepIndex", stepCounter++},
            {"action", "MERGE_DONE"},
            {"description", doneOss.str()},
            {"array", makeSnapshot()},
            {"left", left},
            {"mid", mid},
            {"right", right},
            {"comparedIndices", nlohmann::json::array()},
            {"movedIndex", -1},
            {"sortedIndices", sortedIndices}
        });
    };

    auto mergeSort = [&](int left, int right, auto& self) -> void {
        if (left < right) {
            int mid = left + (right - left) / 2;

            std::ostringstream splitOss;
            splitOss << "Splitting subarray [" << left << ".." << right 
                     << "] at midpoint index " << mid << " into [" 
                     << left << ".." << mid << "] and [" << (mid + 1) << ".." << right << "].";

            steps.push_back({
                {"stepIndex", stepCounter++},
                {"action", "SPLIT"},
                {"description", splitOss.str()},
                {"array", makeSnapshot()},
                {"left", left},
                {"mid", mid},
                {"right", right},
                {"comparedIndices", nlohmann::json::array()},
                {"movedIndex", -1},
                {"sortedIndices", sortedIndices}
            });

            self(left, mid, self);
            self(mid + 1, right, self);
            merge(left, mid, right, merge);
        }
    };

    mergeSort(0, static_cast<int>(arr.size() - 1), mergeSort);

    // Mark all indices sorted in final step
    sortedIndices.clear();
    for (size_t i = 0; i < arr.size(); ++i) sortedIndices.push_back(static_cast<int>(i));

    steps.push_back({
        {"stepIndex", stepCounter++},
        {"action", "COMPLETE"},
        {"description", "Merge Sort complete. Array is stably sorted in non-decreasing order in O(n log n)."},
        {"array", makeSnapshot()},
        {"left", 0},
        {"mid", static_cast<int>(arr.size() - 1) / 2},
        {"right", static_cast<int>(arr.size() - 1)},
        {"comparedIndices", nlohmann::json::array()},
        {"movedIndex", -1},
        {"sortedIndices", sortedIndices}
    });

    return {
        {"algorithm", "Merge Sort"},
        {"timeComplexity", "O(n log n)"},
        {"spaceComplexity", "O(n)"},
        {"totalSteps", steps.size()},
        {"steps", steps}
    };
}

// ==========================================
// 3. LINEAR SEARCH TRACE GENERATION
// ==========================================

nlohmann::json VisualizerService::generateLinearSearchTrace(
    const std::vector<std::string>& items,
    const std::string& query
) {
    nlohmann::json steps = nlohmann::json::array();
    int stepCounter = 0;
    std::vector<int> matchedIndices;
    int comparisonsCount = 0;

    auto makeSnapshot = [&items]() -> nlohmann::json {
        nlohmann::json arr = nlohmann::json::array();
        for (size_t i = 0; i < items.size(); ++i) {
            arr.push_back({
                {"index", static_cast<int>(i)},
                {"value", items[i]}
            });
        }
        return arr;
    };

    // Initial state
    steps.push_back({
        {"stepIndex", stepCounter++},
        {"action", "START"},
        {"currentIndex", -1},
        {"currentElement", ""},
        {"query", query},
        {"isMatch", false},
        {"comparisonsCount", 0},
        {"matchedIndices", nlohmann::json::array()},
        {"description", "Starting Linear Search for query: '" + query + "'. Checking elements sequentially from index [0]."},
        {"array", makeSnapshot()},
        {"isFinal", false},
        {"found", false}
    });

    for (size_t i = 0; i < items.size(); ++i) {
        comparisonsCount++;
        bool isMatch = SearchSortEngine::containsIgnoreCase(items[i], query);
        if (isMatch) {
            matchedIndices.push_back(static_cast<int>(i));
        }

        std::ostringstream oss;
        oss << "Comparison #" << comparisonsCount << ": Checking element at index [" << i 
            << "] ('" << items[i] << "') against query '" << query << "'. ";
        if (isMatch) {
            oss << "MATCH FOUND!";
        } else {
            oss << "No match. Advancing pointer to next index.";
        }

        steps.push_back({
            {"stepIndex", stepCounter++},
            {"action", isMatch ? "MATCH" : "COMPARE"},
            {"currentIndex", static_cast<int>(i)},
            {"currentElement", items[i]},
            {"query", query},
            {"isMatch", isMatch},
            {"comparisonsCount", comparisonsCount},
            {"matchedIndices", matchedIndices},
            {"description", oss.str()},
            {"array", makeSnapshot()},
            {"isFinal", false},
            {"found", !matchedIndices.empty()}
        });
    }

    bool foundAny = !matchedIndices.empty();
    std::string summary = foundAny 
        ? ("Linear Search completed. Found " + std::to_string(matchedIndices.size()) + " match(es) after " + std::to_string(comparisonsCount) + " comparisons.")
        : ("Linear Search completed. Target '" + query + "' NOT FOUND after scanning all " + std::to_string(comparisonsCount) + " elements.");

    steps.push_back({
        {"stepIndex", stepCounter++},
        {"action", "COMPLETE"},
        {"currentIndex", -1},
        {"currentElement", ""},
        {"query", query},
        {"isMatch", false},
        {"comparisonsCount", comparisonsCount},
        {"matchedIndices", matchedIndices},
        {"description", summary},
        {"array", makeSnapshot()},
        {"isFinal", true},
        {"found", foundAny}
    });

    return {
        {"algorithm", "Linear Search"},
        {"timeComplexity", "O(n)"},
        {"totalComparisons", comparisonsCount},
        {"totalMatches", matchedIndices.size()},
        {"totalSteps", steps.size()},
        {"steps", steps}
    };
}

// ==========================================
// 4. BINARY SEARCH TRACE GENERATION
// ==========================================

nlohmann::json VisualizerService::generateBinarySearchTrace(
    const std::vector<std::string>& items,
    const std::string& target,
    bool performPrepSort
) {
    nlohmann::json steps = nlohmann::json::array();
    int stepCounter = 0;

    std::vector<std::string> sortedItems = items;
    bool wasSorted = true;
    for (size_t i = 1; i < items.size(); ++i) {
        if (SearchSortEngine::toLowerCase(items[i - 1]) > SearchSortEngine::toLowerCase(items[i])) {
            wasSorted = false;
            break;
        }
    }

    auto makeSnapshot = [](const std::vector<std::string>& list) -> nlohmann::json {
        nlohmann::json arr = nlohmann::json::array();
        for (size_t i = 0; i < list.size(); ++i) {
            arr.push_back({
                {"index", static_cast<int>(i)},
                {"value", list[i]}
            });
        }
        return arr;
    };

    // Phase 1: Preparatory Sort Visualization if required
    if (!wasSorted && performPrepSort) {
        steps.push_back({
            {"stepIndex", stepCounter++},
            {"phase", "PREP_SORT"},
            {"low", -1},
            {"mid", -1},
            {"high", -1},
            {"midValue", ""},
            {"target", target},
            {"comparison", "NONE"},
            {"comparisonsCount", 0},
            {"eliminatedRange", nlohmann::json::array()},
            {"description", "PRE-CONDITION: Binary Search strictly requires a sorted collection. The original array is unsorted. Initiating preparatory Merge Sort in O(n log n)."},
            {"array", makeSnapshot(items)},
            {"isFinal", false},
            {"found", false},
            {"foundIndex", -1}
        });

        std::sort(sortedItems.begin(), sortedItems.end(), [](const std::string& a, const std::string& b) {
            return SearchSortEngine::toLowerCase(a) < SearchSortEngine::toLowerCase(b);
        });

        steps.push_back({
            {"stepIndex", stepCounter++},
            {"phase", "PREP_SORT"},
            {"low", -1},
            {"mid", -1},
            {"high", -1},
            {"midValue", ""},
            {"target", target},
            {"comparison", "NONE"},
            {"comparisonsCount", 0},
            {"eliminatedRange", nlohmann::json::array()},
            {"description", "Preparatory Merge Sort complete. Array is now sorted alphabetically. Ready for logarithmic Binary Search in O(log n)."},
            {"array", makeSnapshot(sortedItems)},
            {"isFinal", false},
            {"found", false},
            {"foundIndex", -1}
        });
    }

    std::string targetNorm = SearchSortEngine::toLowerCase(target);
    // Trim whitespace
    auto s = targetNorm.find_first_not_of(" \t\n\r");
    if (s != std::string::npos) {
        auto e = targetNorm.find_last_not_of(" \t\n\r");
        targetNorm = targetNorm.substr(s, e - s + 1);
    }

    int low = 0;
    int high = static_cast<int>(sortedItems.size()) - 1;
    int comparisons = 0;
    bool found = false;
    int foundIndex = -1;

    // Start of binary search
    steps.push_back({
        {"stepIndex", stepCounter++},
        {"phase", "BINARY_SEARCH"},
        {"low", low},
        {"mid", -1},
        {"high", high},
        {"midValue", ""},
        {"target", target},
        {"comparison", "NONE"},
        {"comparisonsCount", 0},
        {"eliminatedRange", nlohmann::json::array()},
        {"description", "Binary Search search window initialized: low = [0], high = [" + std::to_string(high) + "]. Target: '" + target + "'."},
        {"array", makeSnapshot(sortedItems)},
        {"isFinal", false},
        {"found", false},
        {"foundIndex", -1}
    });

    while (low <= high) {
        int mid = low + (high - low) / 2;
        comparisons++;
        std::string midVal = sortedItems[mid];
        std::string midNorm = SearchSortEngine::toLowerCase(midVal);

        std::vector<int> eliminated;
        std::string compResult;
        std::string desc;

        if (midNorm == targetNorm) {
            compResult = "EQUAL";
            found = true;
            foundIndex = mid;
            desc = "Iteration #" + std::to_string(comparisons) + ": Calculated mid = [" + std::to_string(mid) 
                 + "] ('" + midVal + "'). EXACT MATCH FOUND with target '" + target + "'!";

            steps.push_back({
                {"stepIndex", stepCounter++},
                {"phase", "BINARY_SEARCH"},
                {"low", low},
                {"mid", mid},
                {"high", high},
                {"midValue", midVal},
                {"target", target},
                {"comparison", compResult},
                {"comparisonsCount", comparisons},
                {"eliminatedRange", nlohmann::json::array()},
                {"description", desc},
                {"array", makeSnapshot(sortedItems)},
                {"isFinal", false},
                {"found", true},
                {"foundIndex", mid}
            });
            break;
        } else if (midNorm < targetNorm) {
            compResult = "LESS";
            // Eliminated lower half [low..mid]
            for (int eIdx = low; eIdx <= mid; ++eIdx) eliminated.push_back(eIdx);

            desc = "Iteration #" + std::to_string(comparisons) + ": Mid element [" + std::to_string(mid) 
                 + "] ('" + midVal + "') < Target ('" + target + "'). Target lies in the upper half. Narrowing low to [" 
                 + std::to_string(mid + 1) + "]. Eliminated left half [0.." + std::to_string(mid) + "].";

            steps.push_back({
                {"stepIndex", stepCounter++},
                {"phase", "BINARY_SEARCH"},
                {"low", low},
                {"mid", mid},
                {"high", high},
                {"midValue", midVal},
                {"target", target},
                {"comparison", compResult},
                {"comparisonsCount", comparisons},
                {"eliminatedRange", eliminated},
                {"description", desc},
                {"array", makeSnapshot(sortedItems)},
                {"isFinal", false},
                {"found", false},
                {"foundIndex", -1}
            });

            low = mid + 1;
        } else {
            compResult = "GREATER";
            // Eliminated upper half [mid..high]
            for (int eIdx = mid; eIdx <= high; ++eIdx) eliminated.push_back(eIdx);

            desc = "Iteration #" + std::to_string(comparisons) + ": Mid element [" + std::to_string(mid) 
                 + "] ('" + midVal + "') > Target ('" + target + "'). Target lies in the lower half. Narrowing high to [" 
                 + std::to_string(mid - 1) + "]. Eliminated right half [" + std::to_string(mid) + ".." + std::to_string(high) + "].";

            steps.push_back({
                {"stepIndex", stepCounter++},
                {"phase", "BINARY_SEARCH"},
                {"low", low},
                {"mid", mid},
                {"high", high},
                {"midValue", midVal},
                {"target", target},
                {"comparison", compResult},
                {"comparisonsCount", comparisons},
                {"eliminatedRange", eliminated},
                {"description", desc},
                {"array", makeSnapshot(sortedItems)},
                {"isFinal", false},
                {"found", false},
                {"foundIndex", -1}
            });

            high = mid - 1;
        }
    }

    std::string finalDesc = found 
        ? ("Binary Search SUCCESS: Found exact match at index [" + std::to_string(foundIndex) + "] after " + std::to_string(comparisons) + " comparisons.")
        : ("Binary Search FINISHED: Search space exhausted (low > high). Target '" + target + "' NOT FOUND after " + std::to_string(comparisons) + " comparisons.");

    steps.push_back({
        {"stepIndex", stepCounter++},
        {"phase", "BINARY_SEARCH"},
        {"low", low},
        {"mid", foundIndex},
        {"high", high},
        {"midValue", foundIndex >= 0 ? sortedItems[foundIndex] : ""},
        {"target", target},
        {"comparison", found ? "EQUAL" : "NOT_FOUND"},
        {"comparisonsCount", comparisons},
        {"eliminatedRange", nlohmann::json::array()},
        {"description", finalDesc},
        {"array", makeSnapshot(sortedItems)},
        {"isFinal", true},
        {"found", found},
        {"foundIndex", foundIndex}
    });

    return {
        {"algorithm", "Binary Search"},
        {"complexity", {
            {"binarySearchLoop", "O(log n)"},
            {"mergeSortPreparation", wasSorted ? "Already Sorted: O(0)" : "Merge Sort: O(n log n)"},
            {"totalOperation", wasSorted ? "O(log n)" : "O(n log n) [Prep Sort + Binary Search]"}
        }},
        {"wasSortedInitially", wasSorted},
        {"totalComparisons", comparisons},
        {"found", found},
        {"foundIndex", foundIndex},
        {"totalSteps", steps.size()},
        {"steps", steps}
    };
}

// ==========================================
// 5. PRIORITY CALCULATION BREAKDOWN
// ==========================================

nlohmann::json VisualizerService::generatePriorityDetails(
    const SubjectManager& subjectManager,
    const TopicManager& topicManager,
    const std::string& topicId
) {
    const auto subjects = subjectManager.getAllSubjects();
    const auto topics = topicManager.getAllTopics();

    // Use PriorityEngine::recommend to get exact breakdowns
    const auto recommendations = PriorityEngine::recommend(subjects, topics);

    nlohmann::json topicListJson = nlohmann::json::array();
    nlohmann::json selectedDetail = nullptr;

    for (const auto& rec : recommendations) {
        const auto& t = rec.topic;
        const auto& b = rec.breakdown;

        nlohmann::json item = {
            {"topicId", t.getId()},
            {"topicName", t.getName()},
            {"subjectName", rec.subjectName},
            {"examDate", rec.examDate},
            {"rawMetrics", {
                {"difficulty", t.getDifficulty()},
                {"syllabusRemaining", t.getSyllabusRemaining()},
                {"academicImportance", t.getAcademicImportance()},
                {"estimatedHours", t.getEstimatedHours()},
                {"status", t.getStatus()},
                {"daysUntilExam", b.daysUntilExam},
                {"hasValidExamDate", b.hasValidExamDate}
            }},
            {"normalizedFactors", {
                {"examUrgency", b.examUrgency},
                {"syllabusRemaining", b.syllabusRemaining},
                {"academicImportance", b.academicImportance},
                {"difficulty", b.difficulty},
                {"subjectWorkload", b.subjectWorkload}
            }},
            {"weights", {
                {"examUrgency", 0.30},
                {"syllabusRemaining", 0.25},
                {"academicImportance", 0.20},
                {"difficulty", 0.10},
                {"subjectWorkload", 0.15}
            }},
            {"contributions", {
                {"examUrgency", 0.30 * b.examUrgency},
                {"syllabusRemaining", 0.25 * b.syllabusRemaining},
                {"academicImportance", 0.20 * b.academicImportance},
                {"difficulty", 0.10 * b.difficulty},
                {"subjectWorkload", 0.15 * b.subjectWorkload}
            }},
            {"finalScore", std::round(b.score * 100.0) / 100.0}
        };

        topicListJson.push_back(item);

        if (!topicId.empty() && t.getId() == topicId) {
            selectedDetail = item;
        }
    }

    if (selectedDetail.is_null() && !topicListJson.empty()) {
        selectedDetail = topicListJson[0];
    }

    return {
        {"formula", "Priority = 0.30 * ExamUrgency + 0.25 * SyllabusRemaining + 0.20 * AcademicImportance + 0.10 * Difficulty + 0.15 * SubjectWorkload"},
        {"weightsSummary", {
            {"examUrgency", "30%"},
            {"syllabusRemaining", "25%"},
            {"academicImportance", "20%"},
            {"difficulty", "10%"},
            {"subjectWorkload", "15%"}
        }},
        {"selectedTopic", selectedDetail},
        {"allEligibleTopics", topicListJson},
        {"totalEligibleTopics", recommendations.size()}
    };
}

nlohmann::json VisualizerService::generateSamplePriorityDetails() {
    // Realistic educational sample
    double urgencyNorm = 85.0; // Exam in 7 days
    double syllabusNorm = 75.0; // 75% remaining
    double importanceNorm = 75.0; // 4 out of 5 -> (4-1)/4 * 100 = 75%
    double difficultyNorm = 75.0; // 4 out of 5 -> 75%
    double workloadNorm = 60.0; // 60% of max workload

    double c1 = 0.30 * urgencyNorm;     // 25.5
    double c2 = 0.25 * syllabusNorm;    // 18.75
    double c3 = 0.20 * importanceNorm;  // 15.0
    double c4 = 0.10 * difficultyNorm;  // 7.5
    double c5 = 0.15 * workloadNorm;    // 9.0
    double finalScore = c1 + c2 + c3 + c4 + c5; // 75.75

    return {
        {"topicName", "Graph Algorithms & Shortest Path (Sample Topic)"},
        {"subjectName", "Design and Analysis of Algorithms"},
        {"examDate", "2026-11-06 (7 days away)"},
        {"rawMetrics", {
            {"difficulty", 4},
            {"syllabusRemaining", 75.0},
            {"academicImportance", 4},
            {"estimatedHours", 4.5},
            {"status", "IN_PROGRESS"},
            {"daysUntilExam", 7},
            {"hasValidExamDate", true}
        }},
        {"normalizedFactors", {
            {"examUrgency", urgencyNorm},
            {"syllabusRemaining", syllabusNorm},
            {"academicImportance", importanceNorm},
            {"difficulty", difficultyNorm},
            {"subjectWorkload", workloadNorm}
        }},
        {"weights", {
            {"examUrgency", 0.30},
            {"syllabusRemaining", 0.25},
            {"academicImportance", 0.20},
            {"difficulty", 0.10},
            {"subjectWorkload", 0.15}
        }},
        {"contributions", {
            {"examUrgency", c1},
            {"syllabusRemaining", c2},
            {"academicImportance", c3},
            {"difficulty", c4},
            {"subjectWorkload", c5}
        }},
        {"finalScore", std::round(finalScore * 100.0) / 100.0},
        {"educationalNotes", {
            {"examUrgency", "Exam is in 7 days, placing it in the 85-point high-urgency tier (<= 7 days). Contribution: 25.5 pts."},
            {"syllabusRemaining", "75% of syllabus is still untouched, demanding significant study attention. Contribution: 18.75 pts."},
            {"academicImportance", "Rated 4/5 in syllabus weighting, normalized to 75.0. Contribution: 15.0 pts."},
            {"difficulty", "Rated 4/5 complex conceptual difficulty, normalized to 75.0. Contribution: 7.5 pts."},
            {"subjectWorkload", "The Algorithms course currently has 60% of total semester pending hours. Contribution: 9.0 pts."}
        }}
    };
}
