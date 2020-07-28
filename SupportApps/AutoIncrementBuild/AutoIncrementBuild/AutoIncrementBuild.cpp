// AutoIncrementBuild.cpp : Updates a build number defined in a header file (with nothing else)
//
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;

// Nasty, nasty source

// The last line of the file must be a definition name followed by a number or this won't work.
// See Build_Number.h in My_OS source for an exmaple formatting.
int main(int argc, char* argv[])
{
    // open the build number header
    fstream hFile, myfile2;

    string filename = "C:\\Users\\Administrator\\Documents\\Visual Studio 2015\\Projects\\MyOS\\MyOS_1\\Build_Number.h";

    // Open a particular file if its name was given on the command-line
    if (argc > 1)
    {
        filename = argv[1];
    }

    hFile.open(filename, ios::in | ios::out);
    //myfile2.open(filename);
    if (!hFile.is_open())
    {
        cout << "Error: couldn't find " << filename << " input file!\n";
        return -1;
    }

    string line = "test";
    string oldLine;
    int gPos = 0, oldGPos = 0;

    // Read each line of the file, saving the file pointer at the beginning of the line, until we reach the last line
    while (getline(hFile, line))
    {
        oldGPos = gPos;
        oldLine = line;
        gPos = hFile.tellg();
    }

    // Stream in the string "BUILD_NUMBER" followed by an int.
    istringstream lineStream(oldLine);
    string buildVar;
    int buildNumber;
    lineStream >> buildVar >> buildNumber;
    //cout << "String: " << buildVar << " number: " << buildNumber << endl;
    
    // increment build number
    buildNumber++;

    // update the h file with the new definition
    hFile.clear();              // Clear EOF bit of stream
    hFile.seekg(oldGPos);       // return to the beginning of the last line

    hFile << buildVar << " " << buildNumber;
    hFile.flush();
    hFile.close();

    return 0;
}