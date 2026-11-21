#ifndef SEARCH_SORT_ENGINE_HPP
#define SEARCH_SORT_ENGINE_HPP

#include <vector>
#include <string>
#include <optional>
#include <functional>
#include "models/Topic.hpp"
#include "models/Subject.hpp"

class SearchSortEngine {
public:
    // ==========================================
    // STRING UTILITIES (FOR ALGORITHMIC NORMALIZATION)
    // ==========================================
    static std::string toLowerCase(const std::string& str);
    static bool containsIgnoreCase(const std::string& haystack, const std::string& needle);

    // ==========================================
    // 1. LINEAR SEARCH ALGORITHM - O(n)
    // ==========================================
    // Performs partial, multi-field, case-insensitive sequential search
    static std::vector<Topic> linearSearchTopics(
        const std::vector<Topic>& topics,
        const std::string& query,
        const std::function<std::string(const std::string&)>& getSubjectNameById = nullptr
    );

    static std::vector<Subject> linearSearchSubjects(
        const std::vector<Subject>& subjects,
        const std::string& query
    );

    // ==========================================
    // 2. BINARY SEARCH ALGORITHM - O(log n)
    // ==========================================
    // Performs exact case-insensitive lookup on a collection sorted by topic name
    static std::optional<Topic> binarySearchTopicExact(
        const std::vector<Topic>& topics,
        const std::string& targetName
    );

    // ==========================================
    // 3. MERGE SORT ALGORITHM - O(n log n)
    // ==========================================
    // Divide-and-conquer stable sorting for Topics
    static std::vector<Topic> sortTopics(
        const std::vector<Topic>& topics,
        const std::string& criteria,
        bool ascending = true
    );

    // Divide-and-conquer stable sorting for Subjects
    static std::vector<Subject> sortSubjects(
        const std::vector<Subject>& subjects,
        const std::string& criteria,
        bool ascending = true
    );

private:
    // Core generic recursive Merge Sort routines
    template<typename T, typename Comparator>
    static void mergeSortRecursive(std::vector<T>& arr, int left, int right, Comparator comp);

    template<typename T, typename Comparator>
    static void merge(std::vector<T>& arr, int left, int mid, int right, Comparator comp);
};

#endif // SEARCH_SORT_ENGINE_HPP
