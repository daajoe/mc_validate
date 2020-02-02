#include <iostream>
#include <iterator> 
#include <map>
#include <string>
#include <fstream>

using namespace std;

map<string, int> mcInputExpectedOutput{ {"instance1", 10 },
		{"instance2", 12},
		{"instance3", 15}
};
map<string, double> wmcInputExpectedOutput{ {"instance1", 5.53 },
		{"instance2", 12},
		{"instance3", 15}
};

map<string, int>::iterator i;
map<string, double>::iterator j;
string fileName = "C:\\Users\\flori\\Desktop\\Computational Logic\\Third Semester\\Project\\Validator\\instance1.wmc";
string line;
ifstream inFile;
bool instance = false;
int mcCorrectSolution = -1;
double wmcCorrectSolution = -1;

void mcCheckSolutino() {
	if (inFile.is_open())
	{
		while (getline(inFile, line))
		{
			if (line.rfind("s", 0) == 0) {
				try
				{
					int solutionFromUser = stoi(line.substr(line.find_last_of("\\ ") + 1));
					if (solutionFromUser == mcCorrectSolution)
					{
						cout << "Correct Solution!" << endl;
						break;
					}
					else
					{
						cout << "Wrong solution!" << endl;;
						break;
					}
				}
				catch (invalid_argument const &e)
				{
					std::cout << "Bad input: std::invalid_argument thrown" << '\n';
				}
			}
		}
	}
	else
		cout << "Unable to open file";
}

void wmcCheckSolutino() {
	if (inFile.is_open())
	{
		while (getline(inFile, line))
		{
			if (line.rfind("s", 0) == 0) {
				try
				{
					double solutionFromUser = stod(line.substr(line.find_last_of("\\ ") + 1));
					cout << solutionFromUser << endl;
					if (solutionFromUser == wmcCorrectSolution)
					{
						cout << "Correct Solution!" << endl;
						break;
					}
					else
					{
						cout << "Wrong solution!" << endl;;
						break;
					}
				}
				catch (invalid_argument const &e)
				{
					std::cout << "Bad input: std::invalid_argument thrown" << '\n';
				}
			}
		}
	}
	else
		cout << "Unable to open file";
}

int main(int argc, char** argv) {
	cout << "Remark: the file path should not contain blank spaces or if it contains then quote it!!!" << endl;

	if (argc>1)
	{
		fileName = argv[1];
		inFile.open(argv[1]);
	}
	else 
	{
		inFile.open(fileName);
	}

	if (inFile.is_open())
	{
		string fileNameWithExtension = fileName.substr(fileName.find_last_of("\\") + 1);
		string fileNameWithExtension2 = fileName.substr(fileName.find_last_of("\\/") + 1);
		string extension = fileNameWithExtension.substr(fileNameWithExtension.find_last_of("\\.") + 1);

		string::size_type const p(fileNameWithExtension.find_last_of('.'));
		string fileNameWithoutExtension = fileNameWithExtension.substr(0, p);
		string::size_type const p2(fileNameWithExtension2.find_last_of('.'));
		string fileNameWithoutExtension2 = fileNameWithExtension2.substr(0, p2);

		if (extension == "mc")
		{
			for (i = mcInputExpectedOutput.begin(); i != mcInputExpectedOutput.end(); i++) {
				if (i->first == fileNameWithoutExtension || i->first == fileNameWithoutExtension2)
					mcCorrectSolution = i->second;
			}
		}
		else
		{
			for (j = wmcInputExpectedOutput.begin(); j != wmcInputExpectedOutput.end(); j++) {
				if (j->first == fileNameWithoutExtension || j->first == fileNameWithoutExtension2)
					wmcCorrectSolution = j->second;
			}
		}

		if (mcCorrectSolution != -1)
		{
			mcCheckSolutino();
		}
		else if (wmcCorrectSolution != -1)
		{
			wmcCheckSolutino();
		}
		inFile.close();	   
	}
	else 
	{
		cout << "Colud not open the file!" << endl;
	}

	return 0;
}