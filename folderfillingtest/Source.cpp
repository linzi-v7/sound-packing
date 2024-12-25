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
bool compareFiles(const pair<string, int>& a, const pair<string, int>& b) {
    return a.second > b.second;
}
void sortFiles(vector<pair<string, int>>& files) {
    sort(files.begin(), files.end(), compareFiles);
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

    //cout << "Folder " << folderCount << ":\n";
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
          //cout << fileName << " " << secondsToTime(fileDuration) << "\n";
          metadataFile << fileName << " " << secondsToTime(fileDuration) << "\n";

          
          currentFolderDuration += fileDuration;
          // Remove file from the list if removeFiles is true
          if (removeFiles) {
              files.erase(files.begin() + fileIndex);
          }
    }

    //cout << secondsToTime(currentFolderDuration) << "\n\n";
    metadataFile << secondsToTime(currentFolderDuration) << "\n";
    metadataFile.close();
}



//########################### WORST FIT (DECREASING) LINEAR ALGORITHM ###################################

// Worst-Fit Linear algorithm - handles file placement and folder filling
// TIME COMPLEXITY O(n*m) where n is the number of files and m is the number of folders
vector<Folder> worstFitLinear(int folderCapacity, vector<pair<string, int>>& files) { // O(n*m)
	vector<Folder> folders; //O(1)

    // Iterate through each file
	for (const auto& file : files) { //O(n)
		string fileName = file.first; //O(1)
		int fileDuration = file.second; //O(1)

        // Find the folder with the most remaining capacity (linear search)
		int maxCapacityIndex = -1; //O(1)
		int maxCapacity = -1; //O(1)
		for (int i = 0; i < folders.size(); ++i) { //O(m)
			if (folders[i].remainingCapacity >= fileDuration && folders[i].remainingCapacity > maxCapacity) { //O(1)
				maxCapacity = folders[i].remainingCapacity; //O(1)
				maxCapacityIndex = i; //O(1)
            }
        }

        // Place file in the folder with the most capacity or create a new folder
		if (maxCapacityIndex != -1) { //O(1)
			folders[maxCapacityIndex].files.emplace_back(fileName, fileDuration); //O(1)
			folders[maxCapacityIndex].remainingCapacity -= fileDuration; //O(1)
        }
		else { //O(1)
			Folder newFolder; //O(1)
			newFolder.files.emplace_back(fileName, fileDuration); //O(1)
			newFolder.remainingCapacity = folderCapacity - fileDuration; //O(1)
			folders.push_back(newFolder);   //O(1)
        }
    }

	return folders; //O(1)
}

// Worst-Fit Decreasing Linear caller function - handles the setup and processing of folders
// TIME COMPLEXITY O(max(nlogn, n*m)) where n is the number of files and m is the number of folders
void WFDLinearCaller(int folderCapacity, vector<pair<string, int>> files, string testNo) { //O(n*m)
	string folderName = "[2.1] WorstFit Decreasing Linear"; //O(1)
	filesystem::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName); //O(1)

    // Sort files in descending order
	sortFiles(files); //O(nlogn)

    // Apply Worst-Fit Decreasing Linear algorithm
	vector<Folder> folders = worstFitLinear(folderCapacity, files); //O(n*m)

    // Process and save folders
	int folderCount = 1; //O(1)
	for (size_t i = 0; i < folders.size(); ++i) { //O(m)
		vector<int> chosenFilesIndexes; //O(1)
		for (size_t j = 0; j < folders[i].files.size(); ++j) { //O(n)
			chosenFilesIndexes.push_back(j); //O(1) Push indexes relative to the current folder
        }

        // Process files and save metadata
		processFiles(folders[i].files, folderCount++, chosenFilesIndexes, folderName, testNo, false); //O(1)
    }
	cout << "folder count: " << folderCount - 1 << endl; //O(1)
}

//the caller function for Worst-Fit Linear algorithm
// TIME COMPLEXITY O(n*m) where n is the number of files and m is the number of folders
void worstFitLinearCaller(int folderCapacity, vector<pair<string, int>> files, string testNo) { //O(n*m)
	string folderName = "[1.1] WorstFit Linear"; //O(1)
	filesystem::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName); //O(1)


    // Apply Worst-Fit Decreasing Linear algorithm
	vector<Folder> folders = worstFitLinear(folderCapacity, files); //O(n*m)

    // Process and save folders
	int folderCount = 1; //O(1)
	for (size_t i = 0; i < folders.size(); ++i) { //O(m)
		vector<int> chosenFilesIndexes; //O(1)
		for (size_t j = 0; j < folders[i].files.size(); ++j) { //O(n)
			chosenFilesIndexes.push_back(j); //O(1) Push indexes relative to the current folder
        }
        // Process files and save metadata
		processFiles(folders[i].files, folderCount++, chosenFilesIndexes, folderName, testNo, false); //O(1)
    }
	cout << "folder count: " << folderCount - 1 << endl; //O(1)
}



//########################### WORST FIT (DECREASING) PQ ALGORITHM ###################################

// Worst-Fit  Algorithm using PRIORITY QUEUE, main logic is here.
// Function to distribute files into folders using the Worst-Fit algorithm
// TIME COMPLEXITY: O(n log m) where n is the number of files and m is the number of folders
vector<Folder> worstFitPQ(vector<pair<string, int>> files, int capacity) // O(n log m)
{
    // Priority queue to manage folders, sorted by remaining capacity
	priority_queue<Folder, vector<Folder>, Compare> folderPriorityQueue; // O(1)

    // Loop through all the files
    for (auto file : files) // O(n)
    {
        int duration = file.second; // O(1)

        // Check if the largest folder in the queue can fit the file
        if (!folderPriorityQueue.empty() && folderPriorityQueue.top().remainingCapacity >= duration) // O(log m)
        {
            // Update the top folder and re-insert into the queue
            Folder topFolder = folderPriorityQueue.top(); // O(1)
            folderPriorityQueue.pop(); // O(log m)
            topFolder.files.push_back(file); // O(1)
            topFolder.remainingCapacity -= duration; // O(1)
            folderPriorityQueue.push(topFolder); // O(log m)
        }
        else
        {
            // Create a new folder and insert into the queue
            Folder newFolder; // O(1)
            newFolder.files.push_back(file); // O(1)
            newFolder.remainingCapacity = capacity - duration; // O(1)
            folderPriorityQueue.push(newFolder); // O(log m)
        }
    }

    // Convert the priority queue into a vector of folders
    vector<Folder> result; // O(1)
    while (!folderPriorityQueue.empty()) // O(m)
    {
        result.push_back(folderPriorityQueue.top()); // O(1)
        folderPriorityQueue.pop(); // O(log m)
    }

    return result; // O(1)
}


 // Function to apply Worst-Fit Decreasing algorithm
//TIME COMPLEXITY: O(n log n)
void worstFitDecreasingPQCaller(int folderCapacity, vector<pair<string, int>> files, string testNo) // O(n log n)
{
    // Sort files in descending order based on size (preprocessing step for WFD)
	sortFiles(files); // O(n log n)

    // Create a directory for output
	string folderName = "[2.0] WorstFit Decreasing PQ"; // O(1)
	fs::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName); // O(1)

    // Apply the Worst-Fit algorithm
	vector<Folder> folders = worstFitPQ(files, folderCapacity); // O(n log m)
    int folderCount = 1; // O(1) To number folders sequentially

    // Process each folder to save its results
	for (auto folder : folders) // O(m)
    {
        vector<int> chosenFilesIndexes; // O(1)

        // Collect the indexes of files in this folder
		for (int i = 0; i < folder.files.size(); i++) // O(n)
		{
			chosenFilesIndexes.push_back(i); // O(1)
		}

        // Copy the files to the folder and save metadata
		processFiles(folder.files, folderCount, chosenFilesIndexes, folderName, testNo, false); // O(1)
		folderCount++; // O(1)
    }
	cout << "folder count: " << folderCount - 1 << endl; // O(1)    
}

/* Complexity Analysis:
 * - Sorting files: O(n log n).
 * - Applying the Worst-Fit algorithm: O(n log m)
 * Combined complexity: O(n log n), dominated by sorting.
 */



 // Function to apply the Worst-Fit algorithm
void worstFitPQCaller(int folderCapacity, vector<pair<string, int>> files, string testNo) // O(n log m)
{
    // Create a directory for output
	string folderName = "[1.0] WorstFit PQ"; // O(1)
	fs::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName); // O(1)

    // Apply the Worst-Fit algorithm
	vector<Folder> folders = worstFitPQ(files, folderCapacity); // O(n log m)
    int folderCount = 1; // To number folders sequentially

    // Process each folder to save its results
	for (auto folder : folders) // O(m)
    {
		vector<int> chosenFilesIndexes; // O(1)

        // Collect the indexes of files in this folder
		for (int i = 0; i < folder.files.size(); i++) // O(n)
        {
			chosenFilesIndexes.push_back(i); // O(1)
        }

        // Copy the files to the folder and save metadata
		processFiles(folder.files, folderCount, chosenFilesIndexes, folderName, testNo, false); // O(1)
		folderCount++; // O(1)
    }
	cout << "folder count: " << folderCount - 1 << endl; // O(1)
}

/* Complexity Analysis:
 * - Applying the Worst-Fit algorithm: O(n log m)
 * Combined complexity: O(n log m).
 */


//########################### FIRST FIT DECREASING ALGORITHM ###################################

//Folder filling using First-Fit Decreasing (FFD) algorithm
//TIME COMPLEXITY: O(n*m) where n is the number of files and m is the number of folders
void folderFillingFFD(int folderCapacity, const vector<pair<string, int>>& files, vector<vector<int>>& folderFileIndexes) { //O(n*m)

    // Vector to store remaining capacity of each folder
	vector<int> folderCapacities; //O(1)

    // Iterate over each file
	for (int fileIndex = 0; fileIndex < files.size(); ++fileIndex) { //O(n)
		const auto& file = files[fileIndex]; //O(1)
		bool placed = false; //O(1)

        // Try to place the file in the first folder with enough capacity
		for (int folderIndex = 0; folderIndex < folderCapacities.size(); ++folderIndex) { //O(m)
			if (file.second <= folderCapacities[folderIndex]) { //O(1)
                folderCapacities[folderIndex] -= file.second; //O(1)
                folderFileIndexes[folderIndex].push_back(fileIndex); //O(1)
				placed = true; //O(1)
				break; //O(1)
            }
        }

        // If the file couldn't be placed, create a new folder
		if (!placed) { //O(1)
			folderCapacities.push_back(folderCapacity - file.second); //O(1)
			folderFileIndexes.emplace_back(vector<int>{fileIndex}); //O(1)
        }
    }
}

//First-Fit Decreasing (FFD) caller
//TIME COMPLEXITY: O(max(nlogn, n*m)) where n is the number of files and m is the number of folders
void FirstFitDecreasing(int folderCapacity, vector<pair<string, int>> files, string testNo) { //O(n*m)
	string folderName = "[3] FirstFit Decreasing"; //O(1)
	filesystem::create_directory("../Sample Tests/Sample " + testNo + "/OUTPUT/" + folderName); //O(1)
    //Sort files in descending order
    sortFiles(files); //O(nlogn)

    //Assign files to folders using FFD algorithm
	vector<vector<int>> folderFileIndexes; //O(1)
	folderFillingFFD(folderCapacity, files, folderFileIndexes); //O(n*m)

    //Process each folder
	int folderIndex; //O(1)
	for (folderIndex = 0; folderIndex < folderFileIndexes.size(); ++folderIndex) { //O(m)
		const auto& fileIndexes = folderFileIndexes[folderIndex]; //O(1)
		processFiles(files, folderIndex + 1, const_cast<vector<int>&>(fileIndexes), folderName, testNo, false); //O(1)
    }
	cout << "folder count: " << folderIndex << endl; //O(1)
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

        auto startTime = chrono::high_resolution_clock::now();

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

        auto endTime = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::nanoseconds>(endTime - startTime);
        totalTime += duration.count();

        //copy chosen files to the current folder and remove them to continue filling other folders
		processFiles(files, folderCount, chosenFilesIndexes, folderName, testNo, true);
        folderCount++; //θ(1)
    }
    cout << "folder count: " << folderCount-1 << "\n" << endl;

    cout << "Total execution time of folderFillingAlgorithm across all iterations: "
        << totalTime << " nanoseconds" << endl;
    cout << "Total execution time in milliseconds: "
        << totalTime / 1e6 << " ms" << endl;
    cout << "Total execution time in seconds: "
        << totalTime / 1e9 << " s" << endl;
    cout << "\n-------------------------------------------------------------------------\n";
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

    cout << "\n-------------------------------------------------------------------------\n";
    cout << "\n1: Worst-Fit using Priority Queue:\n";
    worstFitPQCaller(folderCapacity, files, testNo);

    cout << "\n-------------------------------------------------------------------------\n";
    cout << "\n1.1: Worst-Fit using Linear Search:\n";
    worstFitLinearCaller(folderCapacity, files, testNo);

    cout << "\n-------------------------------------------------------------------------\n";
    cout << "\n2: Worst-Fit Decreasing using Priority Queue:\n";
    worstFitDecreasingPQCaller(folderCapacity, files, testNo);

    cout << "\n-------------------------------------------------------------------------\n";
    cout << "\n2.1: Worst-Fit Decreasing using Linear Search:\n";
    WFDLinearCaller(folderCapacity, files, testNo);

    cout << "\n-------------------------------------------------------------------------\n";
    cout << "\n3: First-Fit Decreasing: \n";
    FirstFitDecreasing(folderCapacity, files, testNo);

    cout << "\n-------------------------------------------------------------------------\n";
    cout << "\n4: Folder Filling Algorithm:\n";
    folderFilling(folderCapacity, files, testNo);


    //rest of algorithms should be called here
    return 0;
}

