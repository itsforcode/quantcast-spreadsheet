#include <cctype>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <stack>
#include <queue>
#include <iomanip>
#include <unordered_set>
using namespace std;

struct Cell {
	string name;
	bool is_evaled;
	double val;
	int edge_count;
	unordered_set<string> ref;
	vector<string> tokens;
	string ex;
	Cell(int row, int col, string expression) {
		stringstream input(expression);
		string token;
		is_evaled = false;
		ex = expression;
		name += ('A'+row);
		name += to_string(col+1);
		while (input >> token) {
			if (token != "+" && token != "-" && token != "*" && token != "/" && token != "++" && token != "--" ) {
				if (!isdigit(token[0])) {
					ref.insert(token);
				}
			}
			tokens.push_back(token);
		}
		edge_count = ref.size();
		//cout << name << " ref size " <<  edge_count << endl;
	}
	
	
};

struct Sheet {
	vector<vector<Cell>> map;
	Sheet(ifstream& file, int m, int n, queue<string>& starts, unordered_map<string, vector<string>>& depends) {
		map.resize(m);
		string expression;
		getline(file,expression);
		
		for (int i = 0; i < m; ++i) {

			for (int j = 0; j < n; ++j) {
				getline(file,expression);
				map[i].push_back(Cell(i, j, expression));
				if (map[i][j].edge_count == 0) {
					starts.push(map[i][j].name);
				} else {
					for (string s : map[i][j].ref) {
						depends[s].push_back(map[i][j].name);
					}
				}
			}
		}

	}

	Cell* get_cell (string name) {
		int row = name[0] - 'A';
		int col = stoi(name.substr(1))-1;
		return &map[row][col];
	}

	void pop_two_args(double& arg1, double& arg2, stack<double>& s) {
    	arg1 = s.top();
    	s.pop();
    	arg2 = s.top();
    	s.pop();
	}

	double evaluate_cell(Cell* cell) {
		//cout << "cell name " << cell->name << " evaluated? " << (cell->is_evaled ? "yes" : "no") << endl;
		if (cell->is_evaled) return cell->val;
		stack<double> s;
		double arg1, arg2;
		for (string token : cell->tokens) {
			if (token == "+") {
				pop_two_args(arg1, arg2, s);
				s.push(arg2+arg1);
			} else if (token == "-") {
				pop_two_args(arg1, arg2, s);
				s.push(arg2-arg1);
			} else if (token == "*") {
				pop_two_args(arg1, arg2, s);
				s.push(arg2*arg1);
			} else if (token == "/") {
				pop_two_args(arg1, arg2, s);
				s.push(arg2/arg1);
			} else if (token == "++") {
				arg1 = s.top();
	    		s.pop();
				s.push(arg1+1);
			} else if (token == "--") {
				arg1 = s.top();
	    		s.pop();
				s.push(arg1-1);
			} else {
				if (!isdigit(token[0])) { // reference
					double ref_val = evaluate_cell(get_cell(token));
					s.push(ref_val);
				} else { // value
					s.push(stod(token));
				}
			}
		}
		

		cell->is_evaled = true;
		cell->val = s.top();
		//cout << "cell val " << cell->val << endl << endl;
		return cell->val;
	}
};

int main(int argc, char* argv[]) {
	ifstream file(argv[1]);
	int width, height;
	file >> width >> height;
	queue<string> starts;
	unordered_map<string, vector<string>> depends;
	Sheet sheet(file, height, width, starts, depends);
	int count_evaluated = 0;
	while (!starts.empty()) {
		Cell* curr = sheet.get_cell(starts.front());
		cout << "curr cell " << curr->name << endl;
		starts.pop();
		sheet.evaluate_cell(curr);
		
		count_evaluated++;
		Cell* cell;
		for (string cell_name : depends[curr->name]) {
			
			cell = sheet.get_cell(cell_name);

			cell->edge_count--;
			if (cell->edge_count == 0) {
				starts.push(cell->name);
			}
			cout << "dependent name edge_count" << cell_name << " " << cell->edge_count << endl;
		}
	}

	if (count_evaluated < width*height) {
		cout << "CIRCULAR" << endl;
		return 1; // circular dependency
	}
	cout << width << " " << height << endl;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			double val = sheet.evaluate_cell(&sheet.map[i][j]);
			cout << setprecision(5) << std::fixed << val << endl; 
		}
	}
	return 0;
}