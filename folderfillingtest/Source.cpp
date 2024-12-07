#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

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


// folder filling algorithm using dynamic programming bottom up approach
int folderFillingAlgorithm(int capacity, int numOfFiles, vector<pair<string, int>>& files, vector<vector<int>>& dpMemory) 
{
    
    // filling the DP table
    for (int i = 0; i <= numOfFiles; i++) 
    {
        for (int j = 0; j <= capacity; j++) 
        {
            if (i == 0 || j == 0) //no files left or no capacity
            {
                dpMemory[i][j] = 0;
            }
            else if (files[i - 1].second <= j)  // if the current file fits
            {
                // get max of including or excluding the file
                dpMemory[i][j] = max(dpMemory[i - 1][j], 
                                dpMemory[i - 1][j - files[i - 1].second] + files[i - 1].second);
            }
            else 
            {
                // If the file doesnt fit, exclude it
                dpMemory[i][j] = dpMemory[i - 1][j];
            }
        }
    }

    //max capacity of folder (no combination of files yet)
    return dpMemory[numOfFiles][capacity];
}


// Folder filling caller
void folderFilling(int folderCapacity, vector<pair<string, int>> files, string testNo) 
{
    string folderName = "[4] FolderFilling";
    filesystem::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName);

    int folderCount = 1;

    //2D Dynamic Array for saving DP results
    vector<vector<int>> dpMemory(files.size() + 1, vector<int>(folderCapacity + 1, 0));

    while (!files.empty()) 
    {

        int numberOfFiles = files.size();

        // get max duration for the current folder
        int maxDuration = folderFillingAlgorithm(folderCapacity, numberOfFiles, files, dpMemory);

        // backtracking phase
        vector<int> chosenFilesIndexes;
        int remainingCapacity = folderCapacity;
        for (int i = numberOfFiles; i > 0; --i) 
        {
            //if previous row has different value, means we took the file to maximize capacity
            if (dpMemory[i][remainingCapacity] != dpMemory[i - 1][remainingCapacity]) 
            {
                chosenFilesIndexes.push_back(i - 1); // file index is 0 based
                remainingCapacity -= files[i - 1].second;
            }
        }


        //copy chosen files to the current folder and remove them to continue filling other folders
        processFiles(files, folderCount, chosenFilesIndexes, folderName, testNo, true);
        folderCount++;
    }
}


// Sort files in descending order
bool compareFiles(const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
    return a.second > b.second;
}
void sortFiles(std::vector<std::pair<std::string, int>>& files) {
    std::sort(files.begin(), files.end(), compareFiles);
}

//Folder filling using First-Fit Decreasing (FFD) algorithm
void folderFillingFFD(int folderCapacity, std::vector<pair<string, int>>& files, std::string testNo) {
    std::string folderName = "[3] FirstFit Decreasing";
    filesystem::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName);
    int folderCount = 1;
    int remainingCapacity = folderCapacity;

    while (!files.empty()) {
        std::vector<int> chosenFilesIndexes;

        for (size_t i = 0; i < files.size(); ++i) {
            if (files[i].second <= remainingCapacity) {
                chosenFilesIndexes.push_back(i);
                remainingCapacity -= files[i].second;
            }
        }

        // Process chosen files
        processFiles(files, folderCount, chosenFilesIndexes, folderName, testNo, false);

        // Remove processed files
        std::vector<pair<string, int>> updatedFiles;
        for (size_t i = 0; i < files.size(); ++i) {
            if (std::find(chosenFilesIndexes.begin(), chosenFilesIndexes.end(), i) == chosenFilesIndexes.end()) {
                updatedFiles.push_back(files[i]);
            }
        }
        files = updatedFiles;

        folderCount++;
        remainingCapacity = folderCapacity;
    }
}



int main() 
{
    int x = 0;
    while (x != 1 && x != 2 && x != 3) 
    {
        cout << "enter sample number (1,2,3): ";
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
        fs::remove_all("../Sample Tests/sample " + testNo + "/OUTPUT");
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


    cout << "Folder Filling Algorithm:\n";
    folderFilling(folderCapacity, files, testNo);

    cout << "first fit descending: \n";
    sortFiles(files);
    folderFillingFFD(folderCapacity, files, testNo);

    //rest of algorithms should be called here
    return 0;
}

