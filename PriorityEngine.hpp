#ifndef PRIORITY_ENGINE_HPP
#define PRIORITY_ENGINE_HPP

#include <string>
#include <vector>
#include "models/Subject.hpp"
#include "models/Topic.hpp"

struct PriorityBreakdown {
    double examUrgency{0.0};
    double syllabusRemaining{0.0};
    double academicImportance{0.0};
    double difficulty{0.0};
    double subjectWorkload{0.0};
    double score{0.0};
    int daysUntilExam{0};
    bool hasValidExamDate{false};
};

struct TopicRecommendation {
    Topic topic;
    std::string subjectName;
    std::string examDate;
    PriorityBreakdown breakdown;
};

struct RecommendationHigherPriority {
    bool operator()(const TopicRecommendation& a, const TopicRecommendation& b) const;
};

// Normalization: ratings map linearly from 1..5 to 0..100; syllabus is already
// a percentage; subject workload is eligible unfinished adjusted hours divided
// by the maximum such workload across subjects. Score = urgency*.30 +
// syllabus*.25 + importance*.20 + difficulty*.10 + workload*.15.
class PriorityEngine {
public:
    // Date arguments use YYYY-MM-DD. Empty today selects the machine's local date.
    static int examUrgency(const std::string& examDate, const std::string& today,
                           int& daysUntilExam, bool& validDate);
    static double normalizeRating(int value, int minimum = 1, int maximum = 5);
    static std::vector<TopicRecommendation> recommend(
        const std::vector<Subject>& subjects,
        const std::vector<Topic>& topics,
        const std::string& subjectId = "",
        const std::string& today = "");
};

#endif
