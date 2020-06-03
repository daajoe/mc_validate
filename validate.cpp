#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <gmp.h>
#include <stdio.h>
#include <assert.h>
#include<string>
#include <cstring>
#include <bits/stdc++.h>

using namespace std;

//////////// HELPER //////////////////

//in milliseconds

const int TIMEOUT_TIME = 1800;
const int MIN_SOLUTION_TIME  = 120;
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
        giveVerdict(-TIMEOUT_TIME, failMsg);
#else
        giveVerdict(-TIMEOUT_TIME, " - Wrong Answer" + failMsg);
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

    if (x[0] == '-') {
        sign = -1;
        x = x.substr(1);
    }

    checkConstraint(mode, x.length() > 0, lineNumber, "Expected double, got non-double string");

    for (char ch : x) {
        checkConstraint(mode, ('0' <= ch && ch <= '9') || ch == '.', lineNumber, "Expected double, got non-double string");
    }

    return stod(x) * sign;
}

////////////// END OF HELPER //////////////////


class Solution {
public:
    string getNumOfModels() {
        return numOfModels;
    }

    string getWeightedModel() {
        return weightedModel;
    }

    void readFromStream(ifstream &is, bool doCheckConstraint, string mode) {
        DO_CHECK_CONSTRAINT = doCheckConstraint;

        numOfModels = "-9999";
        weightedModel = "-9999";
        string modeSol = modeSolution(mode);

        string line;
        while (getline(is, line)) {
            vector<string> tokens = tokenize(line);

            if (tokens.empty() || tokens[0] == "c") {
                continue;
            }

            if (tokens[0] == "s") {
                checkSolutionConstraint(numOfModels == "-9999", "Multiple header in solution");
                checkSolutionConstraint(weightedModel == "-9999", "Multiple header in solution");
                checkSolutionConstraint(tokens.size() == 3, "Header must be 3 tokens <s " + modeSol + " numOfModels>");
                checkSolutionConstraint(tokens[1] == modeSol, "Second header token must be " + modeSol);

                if (mode == "wcnf") {
                    weightedModel = tokens[2];
                } else {
                    numOfModels = tokens[2]; //parseInt(SOLUTION_READ_MODE, tokens[2]);
                }
                continue;
            }
        }

        if (numOfModels != "-9999") {
            checkSolutionConstraint(numOfModels != "-9999", "No header found1");
        } else if (weightedModel != "-9999") {
            checkSolutionConstraint(weightedModel != "-9999", "No header found2");
        } else {
            checkSolutionConstraint(false, "No header found3");
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
    string numOfModels;
    string weightedModel;
};


class ProblemInstance {
public:
    string getNumVariables() {
        return numVariables;
    }

    string getNumClauses() {
        return numClauses;
    }

    string getMode() {
        return mode;
    }

    void readFromStream(ifstream &stream) {
        numVariables = "null";
        numClauses = "null";
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
                checkInputConstraint(numVariables == "null", lineNum, "Multiple header");
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
                numVariables = tokens[2];
                numClauses = tokens[3];

                mpz_t variables, clauses;
                int flag;

                mpz_init(variables);
                mpz_init(clauses);
                mpz_set_ui(variables,0);
                mpz_set_ui(clauses,0);

                flag = mpz_set_str(variables, numVariables.c_str(), 10);
                assert (flag == 0);
                flag = mpz_set_str(clauses, numClauses.c_str(), 10);
                assert (flag == 0);

                checkInputConstraint(mpz_sgn(variables) > 0, lineNum, "numVariables must be positive");
                checkInputConstraint(mpz_sgn(clauses) > 0, lineNum, "numClauses must be positive");

                mpz_clear(variables);
                mpz_clear(clauses);

                continue;
            }
        }
        checkInputConstraint(numVariables != "null", -1, "No header found");
    }

    int validate(Solution solution, string mode) {
        DO_CHECK_CONSTRAINT = true;
        if (mode == "wcnf") {
            mpf_t weightedModel;
            mpf_init(weightedModel);
            mpf_set_str(weightedModel, solution.getWeightedModel().c_str(), 10);

            checkSolutionConstraint(mpf_sgn(weightedModel) >= 0, "Weighted model cannot be negative!");
            mpf_clear(weightedModel);
        } else {
            int flag;
            mpz_t numOfModels;
            mpz_init(numOfModels);
            flag = mpz_set_str(numOfModels,solution.getNumOfModels().c_str(), 10);
            assert (flag == 0);

            checkSolutionConstraint(mpz_sgn(numOfModels) >= 0, "Number of models cannot be negative!");
            mpz_clear(numOfModels);
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
    string numVariables, numClauses, mode;
};


ProblemInstance problemInstance;
Solution judgeSolution, userSolution;
double userTime;

void readdata()
{
    printf("%d|Wrong!\n", 0);
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s instance_input solution_output [instance_output]\n", argv[0]);
        return 0;
    }

    if (argc >= 5) {
        sscanf(argv[4], "%lf", &userTime);
        // since OPTIL give use time in milli seconds..
        userTime /= 1000.0;

        if (userTime > TIMEOUT_TIME) {
            giveVerdict(-TIMEOUT_TIME, "Time Limit Exceeded");
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

    mpf_t jsNumModels, usNumModels, usWeightedModel, jsWeightedModel,
        percentageDeviation, comparisonValue, minusPercentage, plusPercentage;
    mpf_init(jsNumModels);
    mpf_init(usNumModels);
    mpf_init(usWeightedModel);
    mpf_init(jsWeightedModel);
    mpf_init(percentageDeviation);
    mpf_init(comparisonValue);
    mpf_init(minusPercentage);
    mpf_init(plusPercentage);

    if (argc >= 4) {
        mpf_set_str(jsNumModels, judgeSolution.getNumOfModels().c_str(), 10);
        mpf_set_str(usNumModels, userSolution.getNumOfModels().c_str(), 10);

        mpf_set_str(usWeightedModel, userSolution.getWeightedModel().c_str(), 10);
        mpf_set_str(jsWeightedModel, judgeSolution.getWeightedModel().c_str(), 10);

        mpf_set_str(percentageDeviation, "0.1", 10);

        if ((userTime <= MIN_SOLUTION_TIME) && (mpf_sgn(jsNumModels) == -1 && mpf_sgn(jsWeightedModel) == -1) ){
            valid = false;
        } else {
            if (mode == "cnf") {
                mpf_mul(minusPercentage, percentageDeviation, jsNumModels);
                mpf_mul(plusPercentage, percentageDeviation, jsNumModels);
                mpf_sub(minusPercentage, jsNumModels, minusPercentage);
                mpf_add(plusPercentage, jsNumModels, plusPercentage);


                valid &= ((mpf_cmp(minusPercentage, usNumModels) == -1) &&
                        (mpf_cmp(usNumModels, plusPercentage) == -1)) ||
                        (mpf_sgn(jsNumModels) == -1 && mpf_sgn(usNumModels) == 1);

/*                valid &= ((jsNumModels - 0.01 * jsNumModels) <= usNumModels &&
                           usNumModels <= (jsNumModels + jsNumModels * 0.01)) ||
                          (jsNumModels < 0 && usNumModels > 0);*/
            } else if (mode == "wcnf") {

                mpf_mul(minusPercentage, percentageDeviation, jsWeightedModel);
                mpf_mul(plusPercentage, percentageDeviation, jsWeightedModel);
                mpf_sub(minusPercentage, jsWeightedModel, minusPercentage);
                mpf_add(plusPercentage, jsWeightedModel, plusPercentage);

                valid &= ((mpf_cmp(minusPercentage, usWeightedModel) == -1) &&
                          (mpf_cmp(usWeightedModel, plusPercentage) == -1)) ||
                         (mpf_sgn(jsWeightedModel) == -1 && mpf_sgn(usWeightedModel) == 1);

                /*valid &= ((jsWeightedModel - jsWeightedModel * 0.01) <= usWeightedModel &&
                           usWeightedModel <= (jsWeightedModel + jsWeightedModel * 0.01)) ||
                          (jsWeightedModel < 0 && usWeightedModel > 0);*/
            } else if (mode == "pcnf") {
                mpf_mul(minusPercentage, percentageDeviation, jsNumModels);
                mpf_mul(plusPercentage, percentageDeviation, jsNumModels);
                mpf_sub(minusPercentage, jsNumModels, minusPercentage);
                mpf_add(plusPercentage, jsNumModels, plusPercentage);

                valid &= ((mpf_cmp(minusPercentage, usNumModels) == -1) &&
                          (mpf_cmp(usNumModels, plusPercentage) == -1)) ||
                         (mpf_sgn(jsNumModels) == -1 && mpf_sgn(usNumModels) == 1);
            } else {
                valid = false;
            }
        }
    } else {
        readdata();
    }
    DO_CHECK_CONSTRAINT = true;
    // checkSolutionConstraint(valid, "Wrong answer!");
    //giveVerdict(userTime, "SUCCESS");

    mpf_clear(jsNumModels);
    mpf_clear(usNumModels);
    mpf_clear(usWeightedModel);
    mpf_clear(jsWeightedModel);
    mpf_clear(percentageDeviation);
    mpf_clear(comparisonValue);
    mpf_clear(minusPercentage);
    mpf_clear(plusPercentage);

    if (valid)
        printf("%d|SUCCESS\n", 1);
    else
        printf("%d|SUCCESS\n", 0);
    return 0;
}
