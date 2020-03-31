#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;

//////////// HELPER //////////////////

const int TIMEOUT_TIME = 1800;
const int DELAY_TIME = 0;

const int INSTANCE_READ_MODE = 1;
const int SOLUTION_READ_MODE = 2;

bool DO_CHECK_CONSTRAINT;

void checkInputConstraint(bool validConstraint, int lineNumber, string failMsg) {
    if (!validConstraint) {
        cerr << "Instance Error (" << lineNumber << "): " << failMsg << endl;
        exit(2);
    }
}

void giveVerdict(double score, string msg) {
    cout << fixed << setprecision(8) << score << "|" << msg << endl;
    exit(0);
}

void checkSolutionConstraint(bool validConstraint, string failMsg) {
    if (DO_CHECK_CONSTRAINT && !validConstraint) {
#ifdef VERBOSE
        giveVerdict(-TIMEOUT_TIME * 10, failMsg);
#else
        giveVerdict(-TIMEOUT_TIME * 10, " - Wrong Answer");
#endif
    }
}

void checkConstraint(int mode, bool validConstraint, int lineNumber, string failMsg) {
    if (mode == INSTANCE_READ_MODE) {
        checkInputConstraint(validConstraint, lineNumber, failMsg);
    } else if (mode == SOLUTION_READ_MODE) {
        checkSolutionConstraint(validConstraint, failMsg);
    }
}

vector<string> tokenize(string s) {
    vector<string> tokens;
    stringstream ss(s);
    string tmp;

    while (ss >> tmp) {
        tokens.push_back(tmp);
    }

    return tokens;
}

string modeSolution(string mode) {
    if (mode == "cnf") {
        return "mc";
    } else if (mode == "pcnf") {
        return "pmc";
    } else if (mode == "wcnf") {
        return "wmc";
    }

    return "";
}

int parseInt(int mode, string x, int lineNumber = -1) {
    checkConstraint(mode, x.length() > 0, lineNumber, "Expected integer, got empty string");

    int sign = 1;
    int ret = 0;

    if (x[0] == '-') {
        sign = -1;
        x = x.substr(1);
    }

    checkConstraint(mode, x.length() > 0, lineNumber, "Expected integer, got non-integer string");

    for (char ch : x) {
        checkConstraint(mode, '0' <= ch && ch <= '9', lineNumber, "Expected integer, got non-integer string");
        ret = 10 * ret + (ch - '0');
    }

    return ret * sign;
}

double parseDouble(int mode, string x, int lineNumber = -1) {
    checkConstraint(mode, x.length() > 0, lineNumber, "Expected double, got empty string");

    int sign = 1;
    int ret = 0;

    if (x[0] == '-') {
        sign = -1;
        x = x.substr(1);
    }

    checkConstraint(mode, x.length() > 0, lineNumber, "Expected double, got non-double string");

    for (char ch : x) {
        checkConstraint(mode, ('0' <= ch && ch <= '9') || ch == '.', lineNumber, "Expected double, got non-double string");
        ret = 10 * ret + (ch - '0');
    }

    return ret * sign;
}

////////////// END OF HELPER //////////////////


class Solution {
public:
    int getNumOfModels() {
        return numOfModels;
    }

    double getWeightedModel() {
        return weightedModel;
    }

    void readFromStream(ifstream &is, bool doCheckConstraint, string mode) {
        DO_CHECK_CONSTRAINT = doCheckConstraint;

        numOfModels = -1;
        weightedModel = -1.0;
        string modeSol = modeSolution(mode);

        string line;
        while (getline(is, line)) {
            vector<string> tokens = tokenize(line);

            if (tokens.empty() || tokens[0] == "c") {
                continue;
            }

            if (tokens[0] == "s") {
                checkSolutionConstraint(numOfModels == -1, "Multiple header in solution");
                checkSolutionConstraint(weightedModel == -1.0, "Multiple header in solution");
                checkSolutionConstraint(tokens.size() == 3, "Header must be 3 tokens <s " + modeSol + " numOfModels>");
                checkSolutionConstraint(tokens[1] == modeSol, "Second header token must be " + modeSol);

                if (mode == "wcnf") {
                    weightedModel = parseDouble(SOLUTION_READ_MODE, tokens[2]);
                } else {
                    numOfModels = parseInt(SOLUTION_READ_MODE, tokens[2]);
                }
                continue;
            }
        }

        if (numOfModels != -1) {
            checkSolutionConstraint(numOfModels != -1, "No header found");
        } else if (weightedModel != -1.0) {
            checkSolutionConstraint(weightedModel != -1.0, "No header found");
        } else {
            checkSolutionConstraint(false, "No header found");
        }
    }

    void write(ostream &stream, string mode) {
        if (mode == "mc" || mode == "pmc") {
            stream << "s " << mode << " " << numOfModels << endl;
        } else {
            stream << "s " << mode << " " << weightedModel << endl;
        }
    }

private:
    int numOfModels;
    double weightedModel;
};


class ProblemInstance {
public:
    int getNumVariables() {
        return numVariables;
    }

    int getNumClauses() {
        return numClauses;
    }

    string getMode() {
        return mode;
    }

    void readFromStream(ifstream &stream) {
        numVariables = -1;
        numClauses = -1;
        mode = "";

        string line;
        int lineNum = 0;

        while (getline(stream, line)) {
            lineNum++;
            vector<string> tokens = tokenize(line);

            if (tokens.empty() || tokens[0] == "c") {
                continue;
            }

            if (tokens[0] == "p") {
                checkInputConstraint(numVariables == -1, lineNum, "Multiple header");
                if (tokens[1] == "pcnf") {
                    checkInputConstraint(tokens.size() == 5, lineNum,
                                         "Header not consisting of 4 tokens <p cnf numVariables  numClauses numOfProjectedVariables>");
                } else {
                    checkInputConstraint(tokens.size() == 4, lineNum,
                                         "Header not consisting of 4 tokens <p cnf numVariables  numClauses>");
                }
                checkInputConstraint(tokens[0] == "p", lineNum, "First header token must be \"p\"");
                checkInputConstraint(tokens[1] == "cnf" || tokens[1] == "pcnf" || tokens[1] == "wcnf",
                                     lineNum, "Second header token must be \"cnf\" or \"pcnf\" or \"wcnf\"");

                mode = tokens[1];
                numVariables = parseInt(INSTANCE_READ_MODE, tokens[2], lineNum);
                numClauses = parseInt(INSTANCE_READ_MODE, tokens[3], lineNum);

                checkInputConstraint(numVariables > 0, lineNum, "numVariables must be positive");
                checkInputConstraint(numClauses > 0, lineNum, "numClauses must be positive");

                continue;
            }
        }
        checkInputConstraint(numVariables != -1, -1, "No header found");
    }

    int validate(Solution solution, string mode) {
        DO_CHECK_CONSTRAINT = true;
        if (mode == "wcnf") {
            double weightedModel = solution.getWeightedModel();
            checkSolutionConstraint(weightedModel >= 0, "Weighted model cannot be negative!");
        } else {
            int numOfModels = solution.getNumOfModels();
            checkSolutionConstraint(numOfModels >= 0, "Number of models cannot be negative!");
        }
        return 1;
    }

    /*void write(ostream &stream) {
        stream << "p td " << numVertex << " " << numEdge << endl;
        for (auto edge : edgeList) {
            stream << edge.first << " " << edge.second << endl;
        }
    }*/

private:
    int numVariables;
    int numClauses;
    string mode;
};


ProblemInstance problemInstance;
Solution judgeSolution, userSolution;
double userTime;

void readdata()
{
    printf("%d|Wrong!\n", 0);
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("Usage: %s instance_input solution_output [instance_output]\n", argv[0]);
        return 0;
    }

    if (argc >= 5) {
        sscanf(argv[4], "%lf", &userTime);
        // since OPTIL give use 100 * time in seconds..
        // userTime /= 100.0;

        if (userTime > TIMEOUT_TIME + DELAY_TIME) {
            giveVerdict(-TIMEOUT_TIME * 2, "Time Limit Exceeded");
        }
    }

    ifstream instanceInputStream(argv[1]);
    ifstream userOutputStream(argv[2]);

    if (!userOutputStream) {
        // No output
        checkSolutionConstraint(false, "No output produced by user");
    }

    problemInstance.readFromStream(instanceInputStream);
    string mode = problemInstance.getMode();

    userSolution.readFromStream(userOutputStream, true, mode);

    if (argc >= 4) {
        ifstream instanceOutputStream(argv[3]);
        judgeSolution.readFromStream(instanceOutputStream, false, mode);
    }

    int valid = problemInstance.validate(userSolution, mode);
    if (argc >= 4) {
        int jsNumModels = judgeSolution.getNumOfModels();
        int usNumModels = userSolution.getNumOfModels();
        double usWeightedModel = judgeSolution.getWeightedModel();
        double jsWeightedModel = judgeSolution.getWeightedModel();

        if (mode == "cnf") {
            valid &= ((jsNumModels - 0.1 * jsNumModels) <= usNumModels &&
                      usNumModels <= (jsNumModels + jsNumModels * 0.1));
        } else if (mode == "wcnf") {
            valid &= ((jsWeightedModel - jsWeightedModel * 0.15) <= usWeightedModel &&
                      usWeightedModel <= (jsWeightedModel + jsWeightedModel * 0.15));
        } else if (mode == "pcnf") {
            valid &= ((jsNumModels - 0.1 * jsNumModels) <= usNumModels &&
                      usNumModels <= (jsNumModels + jsNumModels * 0.1));
        } else {
            valid = false;
        }
    } else {
        readdata();
    }
    DO_CHECK_CONSTRAINT = true;
    checkSolutionConstraint(valid, "Wrong answer!");


    //giveVerdict(userTime, "SUCCESS");

    printf("%d|SUCCESS\n", 1);

    return 0;
}