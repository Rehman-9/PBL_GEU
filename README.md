# STUDYWISE — Smart Study Management System

A web-based academic management project developed as part of the **Problem-Based Learning (PBL)** curriculum at Graphic Era University, Dehradun.

STUDYWISE helps students organize their academic subjects, manage syllabus topics, identify important topics through priority-based recommendations, and keep their study materials in one place. The project combines a C++ backend with a React frontend and applies Object-Oriented Programming (OOP) and Data Structures and Algorithms (DSA) to solve common academic management problems.

## 📌 Project Overview

Students often manage their academic subjects, syllabus, notes and other study resources across different applications and folders. This makes it difficult to keep track of topics, find relevant materials and decide which topics need attention first.

**STUDYWISE** provides a centralized platform for managing these academic activities. It uses C++ for backend processing and React for the user interface. The application integrates subject and topic management, searching and sorting, a priority-based recommendation engine, study material management and interactive DSA visualizations.

## 🎯 Project Objectives

- Organize academic subjects and their associated topics.
- Maintain syllabus information, difficulty levels and estimated study hours.
- Track topic completion status and remaining syllabus.
- Apply C++ OOP concepts to develop modular and maintainable code.
- Implement searching and sorting algorithms for academic records.
- Calculate topic priorities using exam urgency and other academic factors.
- Use a custom Max Heap to generate priority-based topic recommendations.
- Provide a centralized facility for uploading and managing PDF study materials.
- Demonstrate the working of implemented DSA algorithms through interactive visualizations.

## ✨ Features

### 1. Dashboard
- Provides an overview of subjects, topics and academic records.
- Offers quick navigation to the main application modules.
- Displays the backend connection status.

### 2. Subject Management
- Add, view, update and delete academic subjects.
- Maintain subject names, course codes, semester information and exam dates.
- Validate subject details and preserve subject-topic relationships.

### 3. Topic Management
- Create and manage topics under their respective subjects.
- Maintain difficulty, academic importance, remaining syllabus, estimated hours and completion status.
- Update topic progress and status.

### 4. Searching and Sorting
- **Linear Search:** Searches subjects and topics using supported fields.
- **Binary Search:** Performs exact topic-name searches.
- **Merge Sort:** Sorts subjects and topics according to supported attributes such as name, difficulty, syllabus remaining, academic importance and estimated hours.

### 5. Smart Priority Recommendation System
- Calculates priority scores using five academic factors:
  - Exam urgency — 30%
  - Remaining syllabus — 25%
  - Academic importance — 20%
  - Difficulty — 10%
  - Subject workload — 15%
- Uses a custom vector-backed Max Heap to organize topics according to their priority scores.
- Provides recommendations across all subjects and for individual subjects.
- Excludes completed topics and topics with no remaining syllabus.

### 6. Study Material Management
- Upload PDF documents directly from the user's computer.
- Associate study materials with existing subjects and optionally with topics.
- View and download uploaded PDF files.
- Edit material details and replace uploaded documents.
- Delete material records and their associated files.
- Store uploaded PDFs locally in `backend/uploads/`.
- Maintain material metadata in `backend/data/materials.json`.

### 7. DSA Visualizer
An interactive module that demonstrates the algorithms and data structures used in STUDYWISE.

The five visualizations include:

- **Max Heap Visualization:** Demonstrates insertion, comparisons, swapping, extraction and heap restructuring.
- **Merge Sort Visualization:** Shows recursive division and merging of array elements.
- **Linear Search Visualization:** Demonstrates sequential element comparisons.
- **Binary Search Visualization:** Shows low, middle and high positions during the search process.
- **Priority Calculation Visualization:** Explains the weighted factors, their contributions and the final topic priority score.

## 🛠️ Technology Stack

| Component | Technology |
|---|---|
| Programming Language | C++ |
| Frontend | React.js |
| Frontend Build Tool | Vite |
| Styling | Tailwind CSS |
| Backend | C++ with Crow |
| API Communication | REST APIs and JSON |
| JSON Library | nlohmann/json |
| Data Persistence | JSON files |
| PDF Storage | Local filesystem |
| Build System | CMake |
| Compiler Standard | C++17 |
| Core Concepts | OOP and DSA |

## 🧠 OOP and DSA Concepts

The project demonstrates the practical application of the following concepts:

**Object-Oriented Programming**
- Classes and Objects
- Encapsulation
- Constructors and member functions
- Data validation
- Modular class and manager design

**Data Structures**
- Vectors for storing collections of records.
- Unordered Maps for efficient record lookup using unique IDs.
- Custom Max Heap for priority-based topic recommendations.

**Algorithms**
- Linear Search
- Binary Search
- Merge Sort
- Weighted Priority Calculation
- Max Heap Insertion and Extraction

These implementations are also demonstrated through the DSA Visualizer.

## 🏗️ System Architecture

STUDYWISE follows a modular frontend-backend architecture:

```text
              REACT FRONTEND
                    |
                 REST APIs
                    |
              C++ CROW BACKEND
                    |
          C++ MODELS AND MANAGERS
                    |
          DATA STRUCTURES & ALGORITHMS
                    |
           JSON PERSISTENCE
                    |
          LOCAL PDF FILE STORAGE
```

The React frontend communicates with the C++ backend through REST APIs. The backend manages academic records, performs searching, sorting and priority calculations, and handles PDF operations. JSON files preserve application records, while uploaded PDF documents are stored separately in the local filesystem.

## 📂 Main Project Structure

```text
STUDYWISE/
│
├── backend/
│   ├── include/
│   │   ├── models/
│   │   │   ├── Subject.hpp
│   │   │   ├── Topic.hpp
│   │   │   └── StudyMaterial.hpp
│   │   │
│   │   ├── services/
│   │   │   ├── SubjectManager.hpp
│   │   │   ├── TopicManager.hpp
│   │   │   ├── MaterialManager.hpp
│   │   │   ├── PriorityEngine.hpp
│   │   │   └── VisualizerService.hpp
│   │   │
│   │   └── dsa/
│   │       ├── MaxHeap.hpp
│   │       └── SearchSortEngine.hpp
│   │
│   ├── src/
│   │   ├── main.cpp
│   │   ├── dsa/
│   │   │   └── SearchSortEngine.cpp
│   │   └── services/
│   │       ├── PriorityEngine.cpp
│   │       └── VisualizerService.cpp
│   │
│   ├── data/
│   │   ├── subjects.json
│   │   ├── topics.json
│   │   └── materials.json
│   │
│   ├── uploads/
│   ├── third_party/
│   ├── CMakeLists.txt
│   └── build/
│
├── frontend/
│   ├── src/
│   │   ├── api/
│   │   │   └── client.js
│   │   ├── components/
│   │   │   └── Navbar.jsx
│   │   ├── pages/
│   │   │   ├── Dashboard.jsx
│   │   │   ├── SubjectsPage.jsx
│   │   │   ├── TopicsPage.jsx
│   │   │   ├── RecommendationsPage.jsx
│   │   │   ├── MaterialsPage.jsx
│   │   │   └── DSAVisualizerPage.jsx
│   │   ├── App.jsx
│   │   └── main.jsx
│   │
│   ├── package.json
│   └── vite.config.js
│
├── build_backend.bat
├── run_backend.bat
└── run_frontend.bat
```

*Note: This is the main logical project structure. Some generated build files, third-party headers and configuration files are not shown.*

## 🚀 Running the Application

### Option 1: Using the batch files

1. Run `build_backend.bat` to compile the C++ backend if a rebuild is required.
2. Run `run_backend.bat` to start the backend server.
3. Run `run_frontend.bat` to start the React development server.
4. Open the frontend in your browser at:

   `http://localhost:5173`

### Option 2: Manual execution

**Backend:**

```bash
cmake -B backend/build -S backend
cmake --build backend/build
```

Run the generated backend executable using the project's configured working directory and runtime dependencies.

**Frontend:**

```bash
cd frontend
npm install
npm run dev
```

## 🧪 Testing and Validation

The completed project was tested through backend compilation, frontend production builds, API verification and regression testing.

The reported testing activities include:

- Subject and topic CRUD operations.
- Searching and sorting algorithms.
- Weighted priority calculations and Max Heap recommendations.
- PDF upload, validation, replacement, viewing, downloading and deletion.
- JSON persistence after backend restarts.
- DSA visualization API testing.
- Integration checks for previously implemented modules.

The Stage 7 automated verification report recorded **25 out of 25 tests passed**. Backend and frontend build checks were also reported successful.

## 👥 Project Team

**Rehman Ullah Khan — Team Lead**
- SubjectManager module.
- SearchSortEngine implementation.
- Subject management, searching and sorting.
- Project coordination and documentation.

**Manan Lohni**
- TopicManager module.
- PriorityEngine implementation.
- Topic management and priority calculation.
- Testing and verification of recommendations.

**Dipanshu Sirohi**
- Custom Max Heap implementation.
- Study Material Management module.
- PDF upload and file-handling functionality.
- Testing and debugging of heap and material operations.

All three members contributed to integration, testing and final project development.

## 🎓 Academic Information

- **University:** Graphic Era (Deemed to be University), Dehradun
- **Course:** B.Tech — Computer Science and Engineering
- **Project Type:** Problem-Based Learning (PBL)
- **Project Name:** STUDYWISE — Smart Study Management and Planning System
- **Project Phase:** Phase III
- **Project Status:** Final development stage completed

---

*Developed as an academic project to demonstrate the practical application of C++, Object-Oriented Programming, Data Structures and Algorithms, and web development technologies.*
