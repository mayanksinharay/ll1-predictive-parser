#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>

using namespace std;

const string EPS = "ε";
const string END = "$";

map<string, vector<vector<string>>> grammar ;

set<string> nonTerminals ;
set<string> terminals;
map<string, set<string>> FIRST;
map<string, set<string>> FOLLOW;
map<string, map<string, vector<string>>> parsingTable;

set<string> firstOfSequence(const vector<string>& symbols)
	{
	    set<string> result;

	    if (symbols.empty())
	    {
		result.insert(EPS);
		return result;
	    }

	    bool allEpsilon = true;

	    for (const string& symbol : symbols)
	    {
		if (symbol == EPS)
		{
		    result.insert(EPS);
		    continue;
		}

		
		if (terminals.count(symbol))
		{
		    result.insert(symbol);
		    allEpsilon = false;
		    break;
		}

		
		for (const string& x : FIRST[symbol])
		{
		    if (x != EPS)
		        result.insert(x);
		}

		if (!FIRST[symbol].count(EPS))
		{
		    allEpsilon = false;
		    break;
		}
	    }

	    if (allEpsilon)
		result.insert(EPS);

	    return result;
	}

void computeFIRST()
	{
	for(string t: terminals)
		FIRST[t].insert(t);
	
	for(string nt : nonTerminals)
		FIRST[nt] = {};
	
	bool changed = true ;
	
	while(changed)
		{
		changed = false ;
		for(auto &rule : grammar)
			{
			string A = rule.first ;
			
			for(auto &production : rule.second)
				{
				set<string> temp = firstOfSequence(production);
				
				for(string x : temp)
					{
					if(FIRST[A].insert(x).second)
						changed = true ;
					}
				}
			}
		}
	}

void computeFOLLOW()
	{

	    for (string nt : nonTerminals)
	    {
		FOLLOW[nt] = {};
	    }

	    FOLLOW["E"].insert(END);

	    bool changed = true;

	    while (changed)
	    {
		changed = false;

		for (auto &rule : grammar)
		{
		    string A = rule.first;

		    for (auto &production : rule.second)
		    {
		        for (int i = 0; i < production.size(); i++)
		        {
		            string B = production[i];

		            if (!nonTerminals.count(B))
		                continue;

		            vector<string> beta;

		            for (int j = i + 1; j < production.size(); j++)
		            {
		                beta.push_back(production[j]);
		            }

		            set<string> firstBeta = firstOfSequence(beta);

		            for (string x : firstBeta)
		            {
		                if (x != EPS)
		                {
		                    if (FOLLOW[B].insert(x).second)
		                        changed = true;
		                }
		            }

		            if (beta.empty() || firstBeta.count(EPS))
		            {
		                for (string x : FOLLOW[A])
		                {
		                    if (FOLLOW[B].insert(x).second)
		                        changed = true;
		                }
		            }
		        }
		    }
		}
	    }
	}

void constructParsingTable()
	{
	    for (auto &rule : grammar)
	    {
		string A = rule.first;

		for (auto &production : rule.second)
		{
		    set<string> firstAlpha = firstOfSequence(production);

		    for (string a : firstAlpha)
		    {
		        if (a != EPS)
		        {
		            parsingTable[A][a] = production;
		        }
		    }
		    
		    if (firstAlpha.count(EPS))
		    {
		        for (string b : FOLLOW[A])
		        {
		            parsingTable[A][b] = production;
		        }
		    }
		}
	    }
	}

string pad(string s, int width)
	{
	    while (s.length() < width)
		s += " ";

	    return s;
	}
	
void printParsingTable()
	{
	    cout << "\nLL(1) PARSING TABLE\n\n";

	    int width = 20;

	    cout << pad("", width);

	    for (string t : terminals)
	    {
		cout << pad(t, width);
	    }

	    cout << "\n";

	    cout << string(width * (terminals.size() + 1), '-');
	    cout << "\n";

	    for (string nt : nonTerminals)
	    {
		cout << pad(nt, width);

		for (string t : terminals)
		{
		    string entry;

		    if (parsingTable[nt].count(t))
		    {
		        entry = nt + " -> ";

		        for (string symbol : parsingTable[nt][t])
		        {
		            entry += symbol + " ";
		        }
		    }
		    else
		    {
		        entry = "error";
		    }

		    cout << pad(entry, width);
		}

		cout << "\n";
	    }
	}


void parseInput()
	{
	    string input;

	    cout << "\nEnter input string: ";
	    getline(cin, input);

	    for (int i = 0; i < input.length(); i++)
	    {
		if (input[i] == '+' || input[i] == '*' ||
		    input[i] == '(' || input[i] == ')')
		{
		    input.insert(i + 1, " ");
		    input.insert(i, " ");
		    i += 2;
		}
	    }

	    vector<string> tokens;
	    string word = "";

	    for (char c : input)
	    {
		if (c == ' ')
		{
		    if (!word.empty())
		    {
		        tokens.push_back(word);
		        word = "";
		    }
		}
		else
		{
		    word += c;
		}
	    }

	    if (!word.empty())
		tokens.push_back(word);

	    tokens.push_back("$");

	    vector<string> st;
	    st.push_back("$");
	    st.push_back("E");

	    int ip = 0;

	    cout << "\nPARSING MOVES\n\n";

	    int w1 = 18;   // Matched
	    int w2 = 25;   // Stack
	    int w3 = 25;   // Input
	    int w4 = 30;   // Action

	    cout << pad("Matched", w1)
		 << pad("Stack", w2)
		 << pad("Input", w3)
		 << pad("Action", w4)
		 << "\n";

	    cout << string(w1 + w2 + w3 + w4, '-');
	    cout << "\n";

	    string matched = "";

	    while (!st.empty())
	    {
		string top = st.back();
		string currentInput = tokens[ip];

		string stackString = "";

		for (int i = 0; i < st.size(); i++)
		{
		    stackString += st[i];

		    if (i != st.size() - 1)
		        stackString += " ";
		}

		string inputString = "";

		for (int i = ip; i < tokens.size(); i++)
		{
		    inputString += tokens[i];

		    if (i != tokens.size() - 1)
		        inputString += " ";
		}

		if (top == "$" && currentInput == "$")
		{
		    cout << pad(matched, w1)
		         << pad(stackString, w2)
		         << pad(inputString, w3)
		         << pad("Accept", w4)
		         << "\n";

		    break;
		}

		if (terminals.count(top))
		{
		    if (top == currentInput)
		    {
		        string action = "Match " + currentInput;

		        cout << pad(matched, w1)
		             << pad(stackString, w2)
		             << pad(inputString, w3)
		             << pad(action, w4)
		             << "\n";

		        if (currentInput != "$")
		            matched += currentInput + " ";

		        st.pop_back();
		        ip++;
		    }
		    else
		    {
		        cout << pad(matched, w1)
		             << pad(stackString, w2)
		             << pad(inputString, w3)
		             << pad("ERROR", w4)
		             << "\n";

		        break;
		    }
		}

		else
		{
		    if (parsingTable[top].count(currentInput))
		    {
		        vector<string> production =
		            parsingTable[top][currentInput];

		        string action = "Output " + top + " -> ";

		        for (string symbol : production)
		            action += symbol + " ";

		        cout << pad(matched, w1)
		             << pad(stackString, w2)
		             << pad(inputString, w3)
		             << pad(action, w4)
		             << "\n";

		        st.pop_back();

		        if (!(production.size() == 1 &&
		              production[0] == EPS))
		        {
		            for (int i = production.size() - 1;
		                 i >= 0;
		                 i--)
		            {
		                st.push_back(production[i]);
		            }
		        }
		    }
		    else
		    {
		        cout << pad(matched, w1)
		             << pad(stackString, w2)
		             << pad(inputString, w3)
		             << pad("ERROR", w4)
		             << "\n";

		        break;
		    }
		}
	    }
	}
	
void printSet(set<string> s)
	{
	cout << "{ ";
	for (string x : s)
		cout << x << " ";
	cout << "}";
	}
	
int main()
	{
	    nonTerminals = {"E", "E'", "T", "T'", "F"};
	    
	    terminals = {"+", "*", "(", ")", "id", "$"};

	    grammar["E"] = {
		{"T", "E'"}
	    };

	    grammar["E'"] = {
		{"+", "T", "E'"},
		{EPS}
	    };

	    grammar["T"] = {
		{"F", "T'"}
	    };

	    grammar["T'"] = {
		{"*", "F", "T'"},
		{EPS}
	    };

	    grammar["F"] = {
		{"(", "E", ")"},
		{"id"}
		    };
	computeFIRST();
	cout << "\nFIRST SETS\n";

	for (string nt : nonTerminals)
		{
		cout << "FIRST(" << nt << ") = ";
		printSet(FIRST[nt]);
		cout << endl;
		}
		
	cout << ""<< endl ;
	computeFOLLOW();
	cout << "\nFOLLOW SETS\n";
	for (string nt : nonTerminals)
		{
		cout << "FOLLOW(" << nt << ") = ";
		printSet(FOLLOW[nt]);
		cout << endl;
		}
		
	constructParsingTable();
	printParsingTable();
	parseInput();
	return 0;
	}
