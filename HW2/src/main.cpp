#include<iostream>
#include<fstream>
#include<map>
#include<string>
#include<sstream>
#include<vector>
#include<unordered_map>
#include<algorithm>
#include <climits>
#include<time.h>
using namespace std;

class CELLS {
public:
	string name;
	int a_size = 0;
	int b_size = 0;
	int gain = 0;
	int F;//from block
	int T;// to block
	bool locked = 0;//0:unlock, 1:locked
	bool group = 0;//A=0, B=1
	vector<string> cell_nets;
};

class NETS {
public:
	vector<string> net_cell;//連到那些cells
	int A_s = 0;//distribution
	int B_s = 0;
	//int cutstate = 0;//1=被切到, 0=uncut
	bool critical_nets = 0;//
	int lock_count = 0;
};

map<int, vector<string> > bucket;

map<string, CELLS> cell_info;

map<string, NETS> net_info;

vector<string> split(const string& str, const char& delimiter) {
	vector<string> result;
	stringstream ss(str);
	string tok;

	while (getline(ss, tok, delimiter)) {
		result.push_back(tok);
	}
	return result;
}

vector<string> Read_file(string path) {
	ifstream in;
	vector<string> temp;
	in.open(path);
	if (in.fail()) {
		cout << "input file opening failed.";
	}
	else {
		string s;
		while (getline(in, s)) {
			temp.push_back(s);
		}
	}
	in.close();
	return temp;
}

vector<string> split_cells(vector<string> vec) {
	vector<string> cell_name;
	for (int i = 0; i < vec.size(); i++) {
		vector<string> tt;
		tt = split(vec[i], ' ');
		cell_name.push_back(tt[0]);
		cell_info[tt[0]].name = tt[0];
		cell_info[tt[0]].a_size = stoi(tt[1]);
		cell_info[tt[0]].b_size = stoi(tt[2]);
	}
	return cell_name;
}

vector<string> split_nets(vector<string> vec) {
	vector<string> net_name;
	for (int i = 0; i < vec.size(); i++) {
		vector<string> tt;
		tt = split(vec[i], ' ');
		net_name.push_back(tt[1]);
		for (int j = 3; j < tt.size() - 1; j++) {
			net_info[tt[1]].net_cell.push_back(tt[j]);
		}
	}
	return net_name;
}

void preprocess_for_cell(vector<string> vec) {
	for (int i = 0; i < vec.size(); i++) {
		vector<string> tt;
		tt = split(vec[i], ' ');
		for (int j = 3; j < tt.size() - 1; j++) {
			cell_info[tt[j]].cell_nets.push_back(tt[1]);
		}
	}
}

int sumA = 0, sumB = 0;
int cut_size = 0;
int min_cutsize = INT_MAX;
//int lock_cell;

vector<string> cell_name;
vector<string> net_name;
//vector<string> lock_cell;


void initial_partition() {
	int total = 0;
	for (int i = 0; i < cell_name.size(); i++) {
		string current = cell_name[i];
		if (sumA < sumB) { //面積小的gruop多放一點
			cell_info[current].group = 0; //A group
			sumA += cell_info[current].a_size;
		}
		else {//面積大的group少放一點
			cell_info[current].group = 1; //B group
			sumB += cell_info[current].b_size;
		}
	}

}

void computing_distribution() {
	for (int i = 0; i < net_name.size(); i++) {
		net_info[net_name[i]].A_s = 0;
		net_info[net_name[i]].B_s = 0;
	}

	for (int i = 0; i < net_name.size(); i++) {
		string current_net = net_name[i];
		for (int j = 0; j < net_info[current_net].net_cell.size(); j++) {
			string current_cell = net_info[current_net].net_cell[j];
			if (cell_info[current_cell].group == 0) {
				net_info[current_net].A_s++;
			}
			else if (cell_info[current_cell].group == 1) {
				net_info[current_net].B_s++;
			}
		}
	}
}

void computing_gain() {
	string current;
	for (int i = 0; i < cell_name.size(); i++) {
		current = cell_name[i];
		cell_info[current].gain = 0;
		for (int j = 0; j < cell_info[current].cell_nets.size(); j++) {
			string current_net = cell_info[current].cell_nets[j];
			if (cell_info[current].group == 0) { //此Cell在A
				cell_info[current].F = net_info[current_net].A_s;//要移過去的partition中，這條net有幾個cell
				cell_info[current].T = net_info[current_net].B_s;
			}
			else if (cell_info[current].group == 1) {
				cell_info[current].F = net_info[current_net].B_s;//from B
				cell_info[current].T = net_info[current_net].A_s;//to A
			}
			if (cell_info[current].F == 1) {
				cell_info[current].gain++;
			}
			else if (cell_info[current].T == 0) {
				cell_info[current].gain--;
			}
		}
	}
	for (int i = 0; i < cell_name.size(); i++) {
		string current = cell_name[i];
		bucket[cell_info[current].gain].push_back(current);
	}
}

bool balanced(string move_cell) {//確認area A與area B是否平衡
	int tempA = sumA, tempB = sumB;
	if (cell_info[move_cell].group == 0) { //本來在A
		tempA -= cell_info[move_cell].a_size;
		tempB += cell_info[move_cell].b_size;
	}
	else if (cell_info[move_cell].group == 1) { //本來在B
		tempA += cell_info[move_cell].a_size;
		tempB -= cell_info[move_cell].b_size;
	}
	int W = tempA + tempB;
	int dis = abs(tempA - tempB);
	if (dis < W / 10) {
		sumA = tempA;
		sumB = tempB;
		return true;
	}
	else return false;
}

string MOVE_CELLS() {
	string current_cell = "null";
	for (auto pos = bucket.rbegin(); pos != bucket.rend(); pos++) { //pos指向gain值最大的
		int current_gain = pos->first;
		if (current_gain < 0) break;
		for (int i = 0; i < pos->second.size(); i++) { //遍尋該gain的所有cell
			current_cell = pos->second[i]; //現在跑到哪個cell
			if (balanced(current_cell)) { //該cell是否尚未lockEd，且移動後符合balance
				pos->second.erase(pos->second.begin() + i); //從bucket中移除
				if (bucket[current_gain].empty()) {
					bucket.erase(current_gain); //如果去掉該cell後，gain值那格沒東西，則清除
				}
				return current_cell;
			}
		}
	}
	return "null";
}

void updating_distribution(string move_cell) {
	int original_partition = !cell_info[move_cell].group;
	for (int i = 0; i < cell_info[move_cell].cell_nets.size(); i++) {
		string current_net = cell_info[move_cell].cell_nets[i];
		if (original_partition == 0) {//原本在A
			net_info[current_net].A_s--;
			net_info[current_net].B_s++;
		}
		else if (original_partition == 1) {//原本在B
			net_info[current_net].A_s++;
			net_info[current_net].B_s--;
		}
	}
}

void updating_bucket(string current_cell, int old_gain) {
	int new_gain = cell_info[current_cell].gain;
	bucket[old_gain].erase(remove(bucket[old_gain].begin(), bucket[old_gain].end(), current_cell), bucket[old_gain].end());
	if (bucket[old_gain].empty()) {
		bucket.erase(old_gain);
	}
	bucket[new_gain].push_back(current_cell);
}

//有錯!!

void updating_gain(string base_cell) { //更改與base cell有相連的cell的gain值即可
	int F_p = !cell_info[base_cell].group; //the Front Block of the base cell
	int T_p = cell_info[base_cell].group; //the To Block of the base cell;
	string current_cell, current_net;
	int old_gain = 0;
	int new_gain = 0;
	cell_info[base_cell].locked = 1; //pseudo code line 4
	for (int i = 0; i < cell_info[base_cell].cell_nets.size(); i++) {// for each net n on the base cell
		current_net = cell_info[base_cell].cell_nets[i]; //遍尋有連在此移動cell上的nets
		if (F_p == 0) { //A->B
			cell_info[base_cell].F = net_info[current_net].A_s;
			cell_info[base_cell].T = net_info[current_net].B_s;
		}
		else { //B->A
			cell_info[base_cell].F = net_info[current_net].B_s;
			cell_info[base_cell].T = net_info[current_net].A_s;
		}

		//pseudo code step6
		if (cell_info[base_cell].T == 0) {
			for (int j = 0; j < net_info[current_net].net_cell.size(); j++) {
				current_cell = net_info[current_net].net_cell[j];
				if (cell_info[current_cell].locked == 0) {
					old_gain = cell_info[current_cell].gain;
					cell_info[current_cell].gain++;
					updating_bucket(current_cell, old_gain);
				}
			}
		}
		else if (cell_info[base_cell].T == 1) { //只針對本來就在T的那個cell.gain-1
			for (int j = 0; j < net_info[current_net].net_cell.size(); j++) {
				if (cell_info[base_cell].group == cell_info[net_info[current_net].net_cell[j]].group) {
					current_cell = net_info[current_net].net_cell[j];
					if (cell_info[current_cell].locked == 0) { // unlock
						old_gain = cell_info[current_cell].gain;
						cell_info[current_cell].gain--;
						updating_bucket(current_cell, old_gain);
						break;
					}
				}
			}
		}
		cell_info[base_cell].F--;//移動那邊的減少
		cell_info[base_cell].T++;

		if (T_p == 1) { //A->B
			net_info[current_net].A_s = cell_info[base_cell].F;//A=F
			net_info[current_net].B_s = cell_info[base_cell].T;//B=T
		}
		else if (T_p == 0){
			net_info[current_net].A_s = cell_info[base_cell].T;
			net_info[current_net].B_s = cell_info[base_cell].F;
		}

		if (cell_info[base_cell].F == 0)//From partition of cell_to_move 'BEFORE the move'
		{
			for (int j = 0; j < net_info[current_net].net_cell.size(); j++)
			{
				current_cell = net_info[current_net].net_cell[j];
				if (cell_info[current_cell].locked == 0) //unlocked
				{
					old_gain = cell_info[current_cell].gain;
					cell_info[current_cell].gain--;
					updating_bucket(current_cell, old_gain);
				}
			}
		}
		else if (cell_info[base_cell].F == 1) {
			for (int j = 0; j < net_info[current_net].net_cell.size(); j++) {
				current_cell = net_info[current_net].net_cell[j];
				if (F_p == cell_info[net_info[current_net].net_cell[j]].group) {
					if (cell_info[current_cell].locked == 0) {
						old_gain = cell_info[current_cell].gain;
						cell_info[current_cell].gain += 1;
						updating_bucket(current_cell, old_gain);
						break;
					}
				}
			}
		}
	}
}
int main(int argc, char *argv[]) {
	vector<string> cell_file;
	vector<string> net_file;

	cell_file = Read_file(argv[1]);
	cell_name = split_cells(cell_file);
	net_file = Read_file(argv[2]);
	net_name = split_nets(net_file);
  
	preprocess_for_cell(net_file);
	initial_partition();
	//計算net的distribution
	computing_distribution();
	computing_gain(); //對於每個cell都跑一次gain計算function
	for (int i = 0; i < net_name.size(); i++) {
		string current_net = net_name[i];
		if (net_info[current_net].A_s != 0 && net_info[current_net].B_s != 0) {
			cut_size++;
		}
	}
	//cout << "initial cut_size:" << cut_size << endl;
  string base_cell;
	//從bucket中找最大的gain，開始move
  if(cell_file.size() < 200000){
  	while (!bucket.empty()) {
  		base_cell = MOVE_CELLS();
  		if (base_cell == "null") break;
  		cell_info[base_cell].group = !cell_info[base_cell].group;//change Group
  		cut_size = cut_size - cell_info[base_cell].gain;
  		updating_gain(base_cell);
      clock_t t = clock();
      //cout <<double(t)/CLOCKS_PER_SEC<<endl;
      if (double(t)/CLOCKS_PER_SEC > 295) break;
  	}
  }
  else{
    while (!bucket.empty()) {
  		base_cell = MOVE_CELLS();
  		if (base_cell == "null") break;
  		cell_info[base_cell].group = !cell_info[base_cell].group;//change Group
  		cut_size = cut_size - cell_info[base_cell].gain;
  		updating_gain(base_cell);
      clock_t t = clock();
      //cout <<double(t)/CLOCKS_PER_SEC<<endl;
      if (double(t)/CLOCKS_PER_SEC > 293.5) break;
  	}
  }

	vector<string> GroupA;
	vector<string> GroupB;
	for (int i = 0; i < cell_name.size(); i++) {
		if (cell_info[cell_name[i]].group == 0) { //在A中
			GroupA.push_back(cell_name[i]);
		}
		else GroupB.push_back(cell_name[i]);
	}
 
 	ofstream out;
	out.open(argv[3]);
	out << "cut_size " << cut_size << endl << "A " << GroupA.size() << endl;
	for (int i = 0; i < GroupA.size(); i++) {
	  out << GroupA[i] << endl;
	}
	out << "B " << GroupB.size() << endl;
	for (int i = 0; i < GroupB.size(); i++) {
	  out << GroupB[i] << endl;
	}
  out.close();
	return 0;
}