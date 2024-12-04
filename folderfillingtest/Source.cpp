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
void processFiles(vector<pair<string, int>>& files, int folderCount, vector<int>& chosenFilesIndexes, string folderName) {
    
    string directory = "./OUTPUT/" + folderName + "/F" + to_string(folderCount);

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

        string sourcePath = "./INPUT/Audios/" + fileName;
        string destinationPath = directory + "/" + fileName;

        fs::copy(sourcePath, destinationPath, fs::copy_options::overwrite_existing);

        //print on console and add file to metadata.txt
          cout << fileName << " " << secondsToTime(fileDuration) << "\n";
          metadataFile << fileName << " " << secondsToTime(fileDuration) << "\n";

          
          currentFolderDuration += fileDuration;

          files.erase(files.begin() + fileIndex); // Remove file from the list
        
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
void folderFilling(int folderCapacity, vector<pair<string, int>> files) 
{
    string folderName = "[4] FolderFilling";
    filesystem::create_directory("./OUTPUT/" + folderName);

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
        processFiles(files, folderCount, chosenFilesIndexes, folderName);
        folderCount++;
    }
}

int main() 
{
    //open and read metadata 
    ifstream inputFile("./INPUT/AudiosInfo.txt");
    if (!inputFile.is_open()) 
    {
        cerr << "Error: Could not open AudiosInfo.txt file." << endl;
        return 1;
    }

    //if output file already exist, remove it to start on a fresh page
    if (fs::exists("./OUTPUT"))
    {
        fs::remove_all("./OUTPUT");
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

    filesystem::create_directory("./OUTPUT");


    cout << "Folder Filling Algorithm:\n";
    folderFilling(folderCapacity, files);


    //rest of algorithms should be called here
    return 0;
}

