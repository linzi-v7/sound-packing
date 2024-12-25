#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include<queue>

using namespace std;
namespace fs = filesystem;


//########################### HELPER FUNCTIONS ###################################

// Folder structure to represent each folder
struct Folder {
    vector<pair<string, int>> files;        // Files in this folder
    int remainingCapacity;                  // Remaining capacity of the folder
};

// Comparator for priority queue (max-heap)
// Ensures the folder with the most remaining capacity is at the top
struct Compare {
    bool operator()(const Folder& f1, const Folder& f2) {
        return f1.remainingCapacity < f2.remainingCapacity; // Max-heap
    }
};

// convert HH:MM:SS to seconds
int timeToSeconds(const string& time) 
{
    int hours, minutes, seconds;
    sscanf_s(time.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);
    return hours * 3600 + minutes * 60 + seconds;
}

//converts seconds to time format (HH:MM:SS) for filling metadata file
string secondsToTime(int totalSeconds) 
{
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    string result =
        (hours < 10 ? "0" : "") + to_string(hours) + ":" +
        (minutes < 10 ? "0" : "") + to_string(minutes) + ":" +
        (seconds < 10 ? "0" : "") + to_string(seconds);

    return result;
}

// Sort files in descending order
bool compareFiles(const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
    return a.second > b.second;
}
void sortFiles(std::vector<std::pair<std::string, int>>& files) {
    std::sort(files.begin(), files.end(), compareFiles);
}


//function that copies BATCH OF FILES into a destination folder.
//this function will not work properly with processing files one by one each call.
//folderName represents the algorithm you are working on. 
//e.g: [3] FirstFit Decreasing. CHECK SAMPLE TESTS
void processFiles(vector<pair<string, int>>& files, int folderCount, vector<int>& chosenFilesIndexes, string folderName, string testNo, bool removeFiles) {
    
    string directory = "../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName + "/F" + to_string(folderCount);

    if (!fs::exists(directory))
    {
        fs::create_directory(directory);

    }

    cout << "Folder " << folderCount << ":\n";
    ofstream metadataFile(directory + "_metadata.txt");

    int currentFolderDuration = 0;

    for (int fileIndex : chosenFilesIndexes) 
    {
        string fileName = files[fileIndex].first;
        int fileDuration = files[fileIndex].second;

        string sourcePath = "../Sample Tests/Sample " + testNo + "/INPUT/Audios/" + fileName;
        string destinationPath = directory + "/" + fileName;

        fs::copy(sourcePath, destinationPath, fs::copy_options::overwrite_existing);

        //print on console and add file to metadata.txt
          cout << fileName << " " << secondsToTime(fileDuration) << "\n";
          metadataFile << fileName << " " << secondsToTime(fileDuration) << "\n";

          
          currentFolderDuration += fileDuration;
          // Remove file from the list if removeFiles is true
          if (removeFiles) {
              files.erase(files.begin() + fileIndex);
          }
    }

    cout << secondsToTime(currentFolderDuration) << "\n\n";
    metadataFile << secondsToTime(currentFolderDuration) << "\n";
    metadataFile.close();
}



//########################### WORST FIT (DECREASING) LINEAR ALGORITHM ###################################

// Worst-Fit Linear algorithm - handles file placement and folder filling
vector<Folder> worstFitLinear(int folderCapacity, vector<pair<string, int>>& files) {
    vector<Folder> folders;

    // Iterate through each file
    for (const auto& file : files) {
        string fileName = file.first;
        int fileDuration = file.second;

        // Find the folder with the most remaining capacity (linear search)
        int maxCapacityIndex = -1;
        int maxCapacity = -1;
        for (int i = 0; i < folders.size(); ++i) {
            if (folders[i].remainingCapacity >= fileDuration && folders[i].remainingCapacity > maxCapacity) {
                maxCapacity = folders[i].remainingCapacity;
                maxCapacityIndex = i;
            }
        }

        // Place file in the folder with the most capacity or create a new folder
        if (maxCapacityIndex != -1) {
            folders[maxCapacityIndex].files.emplace_back(fileName, fileDuration);
            folders[maxCapacityIndex].remainingCapacity -= fileDuration;
        }
        else {
            Folder newFolder;
            newFolder.files.emplace_back(fileName, fileDuration);
            newFolder.remainingCapacity = folderCapacity - fileDuration;
            folders.push_back(newFolder);
        }
    }

    return folders;
}

// Worst-Fit Decreasing Linear caller function - handles the setup and processing of folders
void WFDLinearCaller(int folderCapacity, vector<pair<string, int>> files, string testNo) {
    string folderName = "[2.1] WorstFit Decreasing Linear";
    filesystem::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName);

    // Sort files in descending order
    sortFiles(files);

    // Apply Worst-Fit Decreasing Linear algorithm
    vector<Folder> folders = worstFitLinear(folderCapacity, files);

    // Process and save folders
    int folderCount = 1;
    for (size_t i = 0; i < folders.size(); ++i) {
        vector<int> chosenFilesIndexes;
        for (size_t j = 0; j < folders[i].files.size(); ++j) {
            chosenFilesIndexes.push_back(j); // Push indexes relative to the current folder
        }

        // Process files and save metadata
        processFiles(folders[i].files, folderCount++, chosenFilesIndexes, folderName, testNo, false);
    }
}

void worstFitLinearCaller(int folderCapacity, vector<pair<string, int>> files, string testNo) {
    string folderName = "[1.1] WorstFit Linear";
    filesystem::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName);


    // Apply Worst-Fit Decreasing Linear algorithm
    vector<Folder> folders = worstFitLinear(folderCapacity, files);

    // Process and save folders
    int folderCount = 1;
    for (size_t i = 0; i < folders.size(); ++i) {
        vector<int> chosenFilesIndexes;
        for (size_t j = 0; j < folders[i].files.size(); ++j) {
            chosenFilesIndexes.push_back(j); // Push indexes relative to the current folder
        }

        // Process files and save metadata
        processFiles(folders[i].files, folderCount++, chosenFilesIndexes, folderName, testNo, false);
    }
}



//########################### WORST FIT (DECREASING) PQ ALGORITHM ###################################

// Worst-Fit  Algorithm using PRIORITY QUEUE, main logic is here.
vector<Folder> worstFitPQ(vector<pair<string, int>> files, int capacity)
{

    priority_queue<Folder, vector<Folder>, Compare> folderPriorityQueue;

    for (auto file : files) 
    {
        int duration = file.second;

        if (!folderPriorityQueue.empty() && folderPriorityQueue.top().remainingCapacity >= duration) 
        {
            Folder topFolder = folderPriorityQueue.top(); //get the top folder to add current file
            folderPriorityQueue.pop();

            topFolder.files.push_back(file);
            topFolder.remainingCapacity -= duration;

            folderPriorityQueue.push(topFolder); //re-add in PQ after adjusting remaining capacity
        }
        else //create new folder if current file doesnt fit
        {
            Folder newFolder;
            newFolder.files.push_back(file);
            newFolder.remainingCapacity = capacity - duration;

            folderPriorityQueue.push(newFolder);
        }
    }

    vector<Folder> result;
    while (!folderPriorityQueue.empty()) 
    {
        result.push_back(folderPriorityQueue.top());
        folderPriorityQueue.pop();
    }

    return result;
}

// Worst-Fit Decreasing (WFD) w/PQ caller
void worstFitDecreasingPQCaller(int folderCapacity, vector<pair<string, int>> files, string testNo) 
{

    sortFiles(files); //sort descendingly

    string folderName = "[2.0] WorstFit Decreasing PQ";
    fs::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName);



    // Apply Worst-Fit  algorithm
    vector<Folder> folders = worstFitPQ(files, folderCapacity);
    int folderCount = 1;

    // Process and save each folder
    for (auto folder : folders) 
    {
     
        vector<int> chosenFilesIndexes;
        int currentFolderDuration = 0;

        // Collect the indexes of files in this folder
        for (int i = 0; i < folder.files.size(); i++) 
        {
            chosenFilesIndexes.push_back(i);
        }

        // Copy the files into the folder and write metadata
        processFiles(folder.files, folderCount, chosenFilesIndexes, folderName, testNo, false);
        folderCount++;
    }
}

// Worst-Fit (WF) w/PQ caller
void worstFitPQCaller(int folderCapacity, vector<pair<string, int>> files, string testNo)
{


    string folderName = "[1.0] WorstFit PQ";
    fs::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName);



    // Apply Worst-Fit  algorithm
    vector<Folder> folders = worstFitPQ(files, folderCapacity);
    int folderCount = 1;

    // Process and save each folder
    for (auto folder : folders)
    {

        vector<int> chosenFilesIndexes;
        int currentFolderDuration = 0;

        // Collect the indexes of files in this folder
        for (int i = 0; i < folder.files.size(); i++)
        {
            chosenFilesIndexes.push_back(i);
        }

        // Copy the files into the folder and write metadata
        processFiles(folder.files, folderCount, chosenFilesIndexes, folderName, testNo, false);
        folderCount++;
    }
}


//########################### FIRST FIT DECREASING ALGORITHM ###################################

//Folder filling using First-Fit Decreasing (FFD) algorithm
void folderFillingFFD(int folderCapacity, const vector<pair<string, int>>& files, vector<vector<int>>& folderFileIndexes) {

    // Vector to store remaining capacity of each folder
    vector<int> folderCapacities;

    // Iterate over each file
    for (int fileIndex = 0; fileIndex < files.size(); ++fileIndex) {
        const auto& file = files[fileIndex];
        bool placed = false;

        // Try to place the file in the first folder with enough capacity
        for (int folderIndex = 0; folderIndex < folderCapacities.size(); ++folderIndex) {
            if (file.second <= folderCapacities[folderIndex]) {
                folderCapacities[folderIndex] -= file.second; // Update folder's remaining capacity
                folderFileIndexes[folderIndex].push_back(fileIndex); // Track file placement
                placed = true;
                break;
            }
        }

        // If the file couldn't be placed, create a new folder
        if (!placed) {
            folderCapacities.push_back(folderCapacity - file.second); // Add a new folder with updated capacity
            folderFileIndexes.emplace_back(vector<int>{fileIndex}); // Add file to new folder
        }
    }
}


void FirstFitDecreasing(int folderCapacity, vector<pair<string, int>> files, string testNo) {
    string folderName = "[3] FirstFit Decreasing";
    filesystem::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName);
    //Sort files in descending order
    sortFiles(files);

    //Assign files to folders using FFD algorithm
    vector<vector<int>> folderFileIndexes; // Holds file indices for each folder
    folderFillingFFD(folderCapacity, files, folderFileIndexes);

    //Process each folder
    for (size_t folderIndex = 0; folderIndex < folderFileIndexes.size(); ++folderIndex) {
        const auto& fileIndexes = folderFileIndexes[folderIndex];
        processFiles(files, folderIndex + 1, const_cast<vector<int>&>(fileIndexes), folderName, testNo, false);
    }
}



//########################### FOLDER FILLING DP ALGORITHM ###################################

// folder filling algorithm using dynamic programming bottom up approach
//TIME COMPLEXITY: θ(n*m)
int folderFillingAlgorithm(int capacity, int numOfFiles, vector<pair<string, int>>& files, vector<vector<int>>& dpMemory)
{

    // filling the DP table
    for (int i = 0; i <= numOfFiles; i++) //θ(n*m) where n is number of files, m is the desired capacity
    {
        for (int j = 0; j <= capacity; j++) //θ(m) where m is the desired capacity
        {
            if (i == 0 || j == 0) //no files left or no capacity //θ(1)
            {
                dpMemory[i][j] = 0;  //θ(1)
            }
            else if (files[i - 1].second <= j)  // if the current file fits  //θ(1)
            {
                // get max of including or excluding the file
                dpMemory[i][j] = max(dpMemory[i - 1][j],
                    dpMemory[i - 1][j - files[i - 1].second] + files[i - 1].second);  //θ(1)
            }
            else
            {
                // If the file doesnt fit, exclude it
                dpMemory[i][j] = dpMemory[i - 1][j];   //θ(1)
            }
        }
    }

    //max capacity of folder (no combination of files yet)
    return dpMemory[numOfFiles][capacity];  //θ(1)
}


// Folder filling caller
// TIME COMPLEXITY: FOLDER PROCESSING + O(n^2 * m) where n is number of files and m is desired capacity
void folderFilling(int folderCapacity, vector<pair<string, int>> files, string testNo)
{
    string folderName = "[4] FolderFilling";  //θ(1)
    filesystem::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName);

    int folderCount = 1;   //θ(1)

    //2D Dynamic Array for saving DP results
    vector<vector<int>> dpMemory(files.size() + 1, vector<int>(folderCapacity + 1, 0));  //θ(1)

    long long totalTime = 0;

    //worst case scenario: each file is put on a folder by itself, running the folderFillingAlgorithm n times.
    while (!files.empty()) // O(n*n*m) = O(n^2 * m) where n is number of files and m is desired capacity
    {

        int numberOfFiles = files.size();  //θ(1)

        auto startTime = std::chrono::high_resolution_clock::now();

        // get max duration for the current folder
        int maxDuration = folderFillingAlgorithm(folderCapacity, numberOfFiles, files, dpMemory); //θ(n * m)


        // backtracking phase
        vector<int> chosenFilesIndexes;
        int remainingCapacity = folderCapacity;  //θ(1)
        for (int i = numberOfFiles; i > 0; --i) //θ(n) where n is number of files
        {
            //if previous row has different value, means we took the file to maximize capacity
            if (dpMemory[i][remainingCapacity] != dpMemory[i - 1][remainingCapacity])  //θ(1)
            {
                chosenFilesIndexes.push_back(i - 1); // file index is 0 based  //O(1)
                remainingCapacity -= files[i - 1].second;    //θ(1)
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
        totalTime += duration.count();

        //copy chosen files to the current folder and remove them to continue filling other folders
        processFiles(files, folderCount, chosenFilesIndexes, folderName, testNo, true);
        folderCount++; //θ(1)
    }

    cout << "Total execution time of folderFillingAlgorithm across all iterations: "
        << totalTime << " nanoseconds" << std::endl;
    cout << "Total execution time in milliseconds: "
        << totalTime / 1e6 << " ms" << std::endl;
    cout << "Total execution time in seconds: "
        << totalTime / 1e9 << " s" << std::endl;
}



//########################### MAIN ###################################

int main() 
{

    int x = 0;
    while (x < 1 || x > 6 || (x == 0))
    {
        cout << "NOTE: Tests(1,2,3) are for small inputs, Tests(4,5,6) are for large inputs\n";
        cout << "Enter test number (1,2,3,4,5,6): ";
        cin >> x;
    }

    string testNo = to_string(x);
    
    //open and read metadata 
    ifstream inputFile("../Sample Tests/Sample " + testNo + "/INPUT/AudiosInfo.txt");
    if (!inputFile.is_open()) 
    {
        cerr << "Error: Could not open AudiosInfo.txt file." << endl;
        return 1;
    }

    //if output file already exist, remove it to start on a fresh page
    if (fs::exists("../Sample Tests/Sample " + testNo + "/OUTPUT"))
    {
        fs::remove_all("../Sample Tests/Sample " + testNo + "/OUTPUT");
    }

    int numberOfFiles;
    inputFile >> numberOfFiles;

    //reads file names and converts durations to seconds
    vector<pair<string, int>> files(numberOfFiles);
    for (int i = 0; i < numberOfFiles; ++i) 
    {
        string name, duration;
        inputFile >> name >> duration;
        files[i] = { name, timeToSeconds(duration) };
    }


    inputFile.close();

    cout << "FOLDER CAPACITY CANT BE LESS THAN THE MAXIMUM AUDIO CAPACITY TO AVOID INFINITE LOOP\n";
    cout << "Input folder capacity in seconds according to the sample test readme.txt: ";
    int folderCapacity; 
    cin >> folderCapacity;

    filesystem::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT");

    cout << "1: Worst-Fit using Priority Queue:\n";
    worstFitPQCaller(folderCapacity, files, testNo);

    cout << "1.1: Worst-Fit using Linear Search:\n";
    worstFitLinearCaller(folderCapacity, files, testNo);

    cout << "2: Worst-Fit Decreasing using Priority Queue:\n";
    worstFitDecreasingPQCaller(folderCapacity, files, testNo);

    cout << "2.1: Worst-Fit Decreasing using Linear Search:\n";
    WFDLinearCaller(folderCapacity, files, testNo);

    cout << "3: First-Fit Decreasing: \n";
    FirstFitDecreasing(folderCapacity, files, testNo);

    cout << "4: Folder Filling Algorithm:\n";
    folderFilling(folderCapacity, files, testNo);


    //rest of algorithms should be called here
    return 0;
}

