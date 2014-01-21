//============================================================================
// Name        : Knew.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
typedef unsigned int uint;
typedef unsigned short ushort;
using namespace std;

const int MINPAIR = 50; //Minimum Number of concurrent occurrences
const int SHORTSIZE = 16; //In bits
const int LOOKUPSIZE = (1 << SHORTSIZE); //Size of lookup table

/**************

INSTRUCTIONS TO COMPILE :
g++ Knew.cpp -std=c++0x -o Knew
The above command should generate a binary Knew

INSTRUCTIONS TO RUN:
Please copy the (unzipped) text file to the same location as the binary (Knew)
The expected file name of the text file is "Artist_lists_small.txt"
To run the binary :
./Knew

This should print out the comma separated pairs of Artists whose concurrent occurrences is at least MINPAIR (50)

**************/

//Lookup table is used to speed up bit count -- we could have used __inbuilt_popcount (which is used to
//initialize the table but not all architectures might have a special popcount instruction in which
//case it might resort to serial shift and bit check which will be quite slow for repeated checks done
//for every potential pair of artists). Also there is a tradeoff between size of lookupTable and compute speed
//chose 2^16 since it is a reasonable size of 64KB, I went ahead with that.

static char lookupTable[LOOKUPSIZE];

//BitV is a class used as a array of bools
//Used to represent the occurrence of artist in a specific list by setting that bit position
//Overloads & operator for matching the common list occurrences
//Provides a count method which returns the number of bits set
//Boost has a dynamic bitset functionality but was not sure if I can use it for this exercise
class BitV
{
private:
	vector<ushort> intBit; //Internal Bit Vector representation; Size is expanded as required
public:
	BitV(int size = 1024) {
		int elem = size / SHORTSIZE;
		intBit.resize(elem, 0);
	}

	BitV (const BitV &bV) {
		intBit = bV.intBit;
	}

	//Set sets the specific bit position to 1
	void set(int idx) {
		uint shIdx = idx / SHORTSIZE;
		uint offset = idx % SHORTSIZE;
		if (intBit.size() <= shIdx)
			intBit.resize(shIdx+1, 0);
		intBit[shIdx] |= (1 << offset);
	}

	//Clear clears the specified bit position
	void clear(int idx) {
		uint shIdx = idx / SHORTSIZE;
		uint offset = idx % SHORTSIZE;
		if (intBit.size() <= shIdx)
			intBit.resize(shIdx+1, 0);
		else
			intBit[shIdx] &= ~(1 << offset);
	}

	//Returns the population count (number of bits set to 1)
	int count() {
		int popCount = 0;
		for (size_t i = 0; i < intBit.size(); i++)
			popCount += lookupTable[intBit[i]];
		return popCount;
	}

	BitV& operator= (const BitV &right) {
		this->intBit = right.intBit;
		return *this;
	}

	BitV& operator&= (const BitV &right) {
		//If the right bitV is larger, expand ours to accommodate the resulting vector
		if (this->intBit.size() < right.intBit.size())
			(this->intBit).resize(right.intBit.size(), 0);
		for (uint i = 0; i < right.intBit.size(); i++)
			(this->intBit)[i] &= right.intBit[i];
		for (uint i = right.intBit.size(); i < this->intBit.size(); i++)
			(this->intBit)[i] = 0;
		return *this;
	}

	BitV operator& (const BitV &right) {
		BitV newVec = *this;
		newVec &= right;
		return newVec;
	}
};

int main() {
	//Initializing the lookupTable (hopefully amortized over the runtime of the program)
	for (int i = 0; i < LOOKUPSIZE; i++)
		lookupTable[i] = __builtin_popcount(i);

	//HashTable to store the artist name to bitvector (has info about lists in which the artist appears)
	unordered_map<string, BitV> artToLineMap;
	vector<string> filtName;
	string fileName = "Artist_lists_small.txt";
	ifstream aFile (fileName.c_str());
	if (!aFile.is_open()) {
		cout << "Couldn't open file " << fileName << endl;
		return 1;
	}

	string line;
	int lineNum = 0;

	//Construction of HashTable - we go through the lines one-by-one and separate the comma
	//separated strings (time complexity will be to O(N) - where N is number of lines in the file)
	//since the number of artists in each line is limited to 50
	//By representing the HashTable in this way we save a lot on space! Only one entry per Artist and
	//occurrence of each Artist's name in each line is represented by a bit in a BitVector instead of
	//a vector of integers. The size occupied will be O(M * log(N)) where M is number of unique artists
	//and N is the number of lines in the file

	while(getline(aFile, line)) {
		stringstream ss(line);
		string artName;
		while (getline(ss, artName, ',')) {
			if (artToLineMap.find(artName) != artToLineMap.end())
				(artToLineMap[artName]).set(lineNum);
			else {
				BitV nV;
				nV.set(lineNum);
				artToLineMap.insert(pair<string, BitV> (artName, nV));
			}
		}
		lineNum++;
	}
	aFile.close();

	//Here we prune the list by eliminating the artists with occurrences less than MINPAIR (50)
	//We also save the names to a vector since when we want a way to check pairs of artists and thus
	//an ordered representation is required
	//Time complexity : O(M) where M is number of unique artists
	//Space complexity : O(H) number of artists with occurrences greater than 50 (hopefully H << M)
	unordered_map<string, BitV>::iterator it = artToLineMap.begin();
	while (it != artToLineMap.end()) {
		if (it->second.count() < MINPAIR)
			artToLineMap.erase(it++);
		else {
			filtName.push_back(it->first);
			it++;
		}
	}

	//Now we check every pair (nC2) = N*(N-1)/2 of artists - 'and' their occurrence list and do a popcount on the result
	//And we print the names of the pairs whose resulting popcount is greater than or equal to MINPAIR
	//Time Complexity : O(H*H*log(N)) where H -> number of artists with occurences greater than MINPAIR
	//                                where N -> number of lines in the file (this is included because the AND and COUNT
	//                                operations are still proportional to size of text -- since the size of the input
	//                                file is proportional to size of BitV's intBit vector and since AND and COUNT
	//                                walks through these vector)
	for (size_t i = 0; i < filtName.size(); i++) {
		for (size_t j = i + 1; j < filtName.size(); j++) {
			BitV iList = artToLineMap[filtName[i]];
			BitV jList = artToLineMap[filtName[j]];
			iList &= jList;
			if (iList.count() >= MINPAIR)
				cout << filtName[i] << ", " << filtName[j] << endl;
		}
	}

	//Overall the program tries to achieve significant space savings and also tries to optimize on
	//time complexity wherever possible
	return 0;
}
