#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <queue>

using namespace std;
namespace fs = filesystem;

// Folder structure to represent each folder
struct Folder {
    vector<int> files;        // Files in this folder (durations only)
    int remainingCapacity;    // Remaining capacity of the folder
};

// Comparator for priority queue (max-heap)
// Ensures the folder with the most remaining capacity is at the top
struct Compare {
    bool operator()(const Folder& f1, const Folder& f2) {
        return f1.remainingCapacity < f2.remainingCapacity; // Max-heap
    }
};

// Convert HH:MM:SS to seconds
int timeToSeconds(const string& time) {
    int hours, minutes, seconds;
    sscanf_s(time.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);
    return hours * 3600 + minutes * 60 + seconds;
}

// Converts seconds to time format (HH:MM:SS)
string secondsToTime(int totalSeconds) {
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    string result =
        (hours < 10 ? "0" : "") + to_string(hours) + ":" +
        (minutes < 10 ? "0" : "") + to_string(minutes) + ":" +
        (seconds < 10 ? "0" : "") + to_string(seconds);

    return result;
}

// Function to process files and generate metadata
void processFiles(vector<pair<string, int>>& files, int folderCount, vector<int>& chosenFilesIndexes, string folderName, string testNo, bool removeFiles) {
    string directory = "../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName + "/F" + to_string(folderCount);

    if (!fs::exists(directory)) {
        fs::create_directory(directory);
    }

    cout << "Folder " << folderCount << ":\n";
    ofstream metadataFile(directory + "_metadata.txt");

    int currentFolderDuration = 0;

    for (int fileIndex : chosenFilesIndexes) {
        string fileName = files[fileIndex].first;
        int fileDuration = files[fileIndex].second;

        string sourcePath = "../Sample Tests/Sample " + testNo + "/INPUT/Audios/" + fileName;
        string destinationPath = directory + "/" + fileName;

        fs::copy(sourcePath, destinationPath, fs::copy_options::overwrite_existing);

        cout << fileName << " " << secondsToTime(fileDuration) << "\n";
        metadataFile << fileName << " " << secondsToTime(fileDuration) << "\n";

        currentFolderDuration += fileDuration;

        if (removeFiles) {
            files.erase(files.begin() + fileIndex);
        }
    }

    cout << secondsToTime(currentFolderDuration) << "\n\n";
    metadataFile << secondsToTime(currentFolderDuration) << "\n";
    metadataFile.close();
}

// Worst-Fit Decreasing Algorithm
vector<Folder> worstFitDecreasing(vector<int> durations, int capacity) {
    sort(durations.rbegin(), durations.rend());

    priority_queue<Folder, vector<Folder>, Compare> pq;

    for (int duration : durations) {
        if (!pq.empty() && pq.top().remainingCapacity >= duration) {
            Folder topFolder = pq.top();
            pq.pop();

            topFolder.files.push_back(duration);
            topFolder.remainingCapacity -= duration;

            pq.push(topFolder);
        }
        else {
            Folder newFolder;
            newFolder.files.push_back(duration);
            newFolder.remainingCapacity = capacity - duration;

            pq.push(newFolder);
        }
    }

    vector<Folder> result;
    while (!pq.empty()) {
        result.push_back(pq.top());
        pq.pop();
    }

    return result;
}

// Folder filling using Worst-Fit Decreasing (WFD) algorithm
void folderFillingWFD(int folderCapacity, vector<pair<string, int>> files, string testNo) {
    string folderName = "[5] WorstFit Decreasing";
    fs::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName);

    // Convert files into durations-only format for WFD processing
    vector<int> durations;
    for (const auto& file : files) {
        durations.push_back(file.second);
    }

    // Apply Worst-Fit Decreasing algorithm
    vector<Folder> folders = worstFitDecreasing(durations, folderCapacity);

    // Process and save each folder
    for (size_t i = 0; i < folders.size(); ++i) {
        vector<int> chosenFilesIndexes;

        // Map file durations back to original file indexes
        for (int duration : folders[i].files) {
            for (size_t j = 0; j < files.size(); ++j) {
                if (files[j].second == duration) {
                    chosenFilesIndexes.push_back(j);
                    break;
                }
            }
        }

        processFiles(files, i + 1, chosenFilesIndexes, folderName, testNo, false);
    }
}

int main() {
    int x = 0;
    while (x != 1 && x != 2 && x != 3) {
        cout << "Enter sample number (1, 2, 3): ";
        cin >> x;
    }

    string testNo = to_string(x);

    // Open and read metadata
    ifstream inputFile("../Sample Tests/Sample " + testNo + "/INPUT/AudiosInfo.txt");
    if (!inputFile.is_open()) {
        cerr << "Error: Could not open AudiosInfo.txt file." << endl;
        return 1;
    }

    // Remove old output directory if it exists
    if (fs::exists("../Sample Tests/Sample " + testNo + "/OUTPUT")) {
        fs::remove_all("../Sample Tests/Sample " + testNo + "/OUTPUT");
    }

    int numberOfFiles;
    inputFile >> numberOfFiles;

    vector<pair<string, int>> files(numberOfFiles);
    for (int i = 0; i < numberOfFiles; ++i) {
        string name, duration;
        inputFile >> name >> duration;
        files[i] = { name, timeToSeconds(duration) };
    }
    inputFile.close();

    cout << "FOLDER CAPACITY CANNOT BE LESS THAN THE MAXIMUM AUDIO CAPACITY TO AVOID INFINITE LOOP\n";
    cout << "Input folder capacity in seconds according to the sample test readme.txt: ";
    int folderCapacity;
    cin >> folderCapacity;

    fs::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT");

    cout << "\nApplying Folder Filling Algorithms:\n";
    cout << "Worst-Fit Decreasing:\n";
    folderFillingWFD(folderCapacity, files, testNo);

    // Additional algorithms can be called here
    return 0;
}
