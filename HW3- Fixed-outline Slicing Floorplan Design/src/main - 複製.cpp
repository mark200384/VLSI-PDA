#include<iostream>
#include<vector>
#include <iterator>
#include<fstream>
#include<string>
#include<sstream>
#include <algorithm>
#include<map>
#include<time.h>
#include<stack>
using namespace std;

class Dimension {
public:
	vector<pair<int, int> > orients;
	vector<bool> use;
	string name;
	int position; //position of polish expression
	int l_child, r_child;
};
vector<Dimension> global_dimension;
class Block{
public:
	int w, h;
	bool rotate=0;//0:未旋轉 1:旋轉
	pair<int, int> coord;
	string name;
};

typedef struct cluster {
	int beg, end;
	int w, h;
}cluster_t;

class Terminal {
public:
	int id;
	string name;
	pair<int, int> coord;
};

class Net {
public:
	vector<string> terminal;
	vector<string> block;
	int degree=0;
};

vector<Block> blocks;
vector<Terminal> terminals;
vector<Net> nets;
map<string, int> indexforsb;
map<string, int> indexforterm;

vector<string> polish_expression;
vector<int> operand_pos;// the position where operand is in polish_expression
vector<int> chain;
int numofhrb = 0, numofterm = 0, numofnet = 0, numofpin = 0;
float limitfloorplain = 0;
float dead_space_ratio = 0;//input
int flag = 0, current_flag = 0;
ifstream readline_hb;
ifstream readline_nets;
ifstream readline_pl;
string tmp;
vector<string> split(const string& str, const char& delimiter) {
	vector<string> result;
	stringstream ss(str);
	string tok;
	while (getline(ss, tok, delimiter)) {
		result.push_back(tok);
	}
	return result;
}
bool compare(Block& a, Block& b) {
	if (a.h == b.h) return a.w > b.w;
	else return a.h > b.h;
}

int read_hardblock(string path){
	int total_block_area=0;
	readline_hb.open(path);
	readline_hb >> tmp >> tmp >> numofhrb;
	readline_hb >> tmp >> tmp >> numofterm;
	//read_hardblocks >> tmp;//空白行
	for (int i = 0; i < numofhrb; i++) {
		Block t;
		readline_hb >> t.name;
		readline_hb >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp;
		t.w = stoi(tmp.substr(1, tmp.size() - 2));
		readline_hb >> tmp;
		t.h = stoi(tmp.substr(0, tmp.size() - 1));
		if (t.w > t.h) { //使較長的邊當高
			t.rotate = (t.rotate+1)%2;
			int temp = t.w;
			t.w = t.h;
			t.h = temp;
		}
		blocks.push_back(t);
		total_block_area += t.w * t.h;
		getline(readline_hb, tmp);
	}
	sort(blocks.begin(), blocks.end(), compare);
	readline_hb.close();
	return total_block_area;
}
void read_net(string path) {
	readline_nets.open(path);
	readline_nets >> tmp >> tmp >> numofnet;
	readline_nets >> tmp >> tmp >> numofpin;
	for (int i = 0; i < numofnet; i++) {
		Net t;
		readline_nets >> tmp >> tmp >> tmp;
		int k = stoi(tmp);//netDegree
		t.degree = k;
		for (int j = 0; j < k; j++) {
			readline_nets >> tmp;
			if (tmp[0] == 'p') {
				t.terminal.push_back(tmp);
			}
			else {
				t.block.push_back(tmp);
			}
		}
		nets.push_back(t);
	}
	readline_nets.close();
}
void read_pl(string path) {
	readline_pl.open(path);
	stringstream ss;

	for (int i = 1; i < numofterm+1; i++) {
		Terminal t;
		readline_pl >> t.name;
		t.id = stoi(t.name.substr(1, t.name.size() - 1));

		indexforterm[t.name] = i;
		readline_pl >> t.coord.first;
		readline_pl >> t.coord.second;
		terminals.push_back(t);
	}
	readline_pl.close();
}

int updating_coord(vector<string> temp_polish,vector<Block> &temp_blocks) {
	stack<cluster_t> stack;
	int i, j;
	cluster_t cluster, cluster_l, cluster_r;
	for (i = 0; i < temp_polish.size(); i++) {
		int current_block_idx = indexforsb[temp_polish[i]];
		if (temp_polish[i] != "H" && temp_polish[i] != "V") {//hardblock
			temp_blocks[current_block_idx].coord.first = 0;
			temp_blocks[current_block_idx].coord.second = 0;
			cluster.beg = i;
			cluster.end = i;
			cluster.w = temp_blocks[current_block_idx].w;
			cluster.h = temp_blocks[current_block_idx].h;
			stack.push(cluster);
		}
		else {
			cluster_r = stack.top(); stack.pop();
			cluster_l = stack.top(); stack.pop();
			cluster.beg = cluster_l.beg;
			cluster.end = cluster_r.end;
			if (temp_polish[i] == "H") {
				for (j = cluster_r.beg; j <= cluster_r.end; ++j) {
					if (temp_polish[j]=="H" || temp_polish[j] == "V" ) continue;
					temp_blocks[indexforsb[temp_polish[j]]].coord.second += cluster_l.h;
				}
				cluster.w = max(cluster_l.w, cluster_r.w);
				cluster.h = cluster_l.h + cluster_r.h;
				int cluster_area = cluster.w * cluster.h;
			}
			else { //"V"
				for (j = cluster_r.beg; j <= cluster_r.end; ++j) {
					if (temp_polish[j] == "H" || temp_polish[j] == "V") continue;
					temp_blocks[indexforsb[temp_polish[j]]].coord.first += cluster_l.w;
				}
				cluster.w = cluster_l.w + cluster_r.w;
				cluster.h = max(cluster_l.h, cluster_r.h);
			}
			stack.push(cluster);
		}
	}
	cluster_t temp;
	temp = stack.top();
	if (temp.w <= limitfloorplain && temp.h <= limitfloorplain) { flag = 1; current_flag = 1; }
	else current_flag = 0;;
	return temp.w * temp.h;
}

void operand_operator(vector<string> temp_polish) {
	chain.clear();
	operand_pos.clear();
	for (int i = 0; i < temp_polish.size(); i++) {
		if (temp_polish[i] != "H" && temp_polish[i] != "V") operand_pos.push_back(i);
		else chain.push_back(i);
	}
}

bool balloting_property(vector<string> temp_polish,int pos1, int pos2) { // #operands > #operators
	int numofoperand = 0, numofoperator = 0;
	swap(temp_polish[pos1], temp_polish[pos2]);
	for (int i = 0; i < max(pos1, pos2); i++) {
		if (temp_polish[i] == "H" || temp_polish[i] == "V") numofoperator++;
		else numofoperand++;
		if (numofoperator >= numofoperand)
			return false;
	}
	return true;
}
void Rotate(vector<Block>& t_block,string name) {
	t_block[indexforsb[name]].rotate = (t_block[indexforsb[name]].rotate + 1) % 2;
	int temp = t_block[indexforsb[name]].w;
	t_block[indexforsb[name]].w = t_block[indexforsb[name]].h;
	t_block[indexforsb[name]].h = temp; 
}
vector<string> Perturbation(vector<Block> &blocks_t) {
	vector<string> temp_polish = polish_expression;
	int pos1 = 0, pos2 = 0;
	int r = rand() % 4;
	if (r == 0) {//rotate
		pos1 = (operand_pos.size()) * rand() / (RAND_MAX + 1);
		pos1 = operand_pos[pos1];
		string name = temp_polish[pos1];
		Rotate(blocks_t,name);
	}
	else if (r ==1) { //M1, swap two sb
		pos1 = (operand_pos.size() - 0) * rand() / (RAND_MAX + 1) + 0;
		pos1 = operand_pos[pos1];
		pos2 = pos1 + 1;
		if (pos2 == temp_polish.size()) pos2--;
		while (temp_polish[pos2] == "V" || temp_polish[pos2] == "H") {
			if (pos2 < temp_polish.size() - 1)//沒超過bound，往後找相鄰的pos2
				pos2++;
			else break;
		}
		if (pos2 == temp_polish.size()-1) {//還沒找到，但到邊界了
			pos2 = pos1 - 1;
			while (temp_polish[pos2] == "V" || temp_polish[pos2] == "H") {
				pos2--;
			}
		}
		swap(temp_polish[pos1], temp_polish[pos2]);
	}
	else if (r ==2) { //M2
		pos1 = (chain.size() - 0) * rand() / (RAND_MAX + 1) + 0;
		pos1 = chain[pos1];
		if (temp_polish[pos1] == "V") temp_polish[pos1] = "H";
		else if (temp_polish[pos1] == "H") temp_polish[pos1] = "V";
	}
	else { //M3
		pos1 = (operand_pos.size() - 0) * rand() / (RAND_MAX + 1) + 0; // pos1 is operand, so pos2 needs to be operator
		pos1 = operand_pos[pos1];//找出在polish的位置
		//往後找一定會有operator
		pos2 = pos1 + 1;
		while (temp_polish[pos2] != "H" && temp_polish[pos2] != "V") pos2++;
		
		if (balloting_property(temp_polish, pos1, pos2)) {//如果可以換
			operand_operator(temp_polish);
			swap(temp_polish[pos1], temp_polish[pos2]);
		}//如果不能換，重找operand and operator to change
	}
	return temp_polish;
}
int updating_wirelength(vector<Block> blocks) {
	int wirelength = 0;
	for (auto& n : nets) {
		int left = INT_MAX, down = INT_MAX, right = 0, up = 0;
		for (auto& a : n.block) {
			Block* t = &blocks[indexforsb[a]];
			if (t->coord.first + (t->w / 2) < left) left = t->coord.first + (t->w / 2);
			if (t->coord.first + (t->w / 2) > right) right = t->coord.first + (t->w / 2);
			if (t->coord.second + (t->h / 2) < down) down = t->coord.second + (t->h / 2);
			if (t->coord.second + (t->h / 2) > up) up = t->coord.second + (t->h / 2);
		}
		for (auto& pin : n.terminal) {
			Terminal* t = &terminals[indexforterm[pin]-1];
			if (t->coord.first < left) left = t->coord.first;
			if (t->coord.first > right) right = t->coord.first;
			if (t->coord.second < down) down = t->coord.second;
			if (t->coord.second > up) up = t->coord.second;
		}
		wirelength += (up - down) + (right - left);
	}
	return wirelength;
}
void init_Floorplan() {
	int flag = 0, root = 0, current_w = 0;
	polish_expression.push_back(blocks[0].name);
	indexforsb[blocks[0].name] = 0;
	current_w += blocks[0].w;
	for (int i = 1; i < numofhrb; i++) {
		if (current_w + blocks[i].w <= limitfloorplain) {
			current_w += blocks[i].w;
			polish_expression.push_back(blocks[i].name);
			polish_expression.push_back("V");
		}
		else {
			if (root == 0) {
				root = i; current_w = blocks[i].w; 
			}//first row先不放H
			else {
				root = i;
				current_w = blocks[i].w;
				polish_expression.push_back("H");
			}
			polish_expression.push_back(blocks[i].name);
		}
		indexforsb[blocks[i].name] = i;
	}
	polish_expression.push_back("H");
}
map<int, Dimension> indexfordim;
void constructDimension(vector<string>expression) {
	for (int i = 0; i < expression.size(); i++) {
		pair<int, int> temp;
		Dimension t;
		if (expression[i] != "H" && expression[i] != "V") {
			t.name = expression[i];
			t.position = i;
			t.l_child = t.r_child = -1;
			temp.first = blocks[indexforsb[expression[i]]].w;
			temp.second = blocks[indexforsb[expression[i]]].h;
			t.orients.push_back(temp);
			t.use.push_back(0);
			if (blocks[indexforsb[expression[i]]].w != blocks[indexforsb[expression[i]]].h) {
				t.orients.push_back({ temp.second,temp.first });
				t.use.push_back(0);
			}
			indexfordim[i] = t;
			global_dimension.push_back(t);
		}

	}
}

pair<int,int> verticalSlice(pair<int,int> a,pair<int, int> b) {
	pair<int, int > temp;
	temp.second =  max(a.second, b.second);
	temp.first = a.first + b.first;
	return temp;
}
Dimension verticalMerge(Dimension first, Dimension second) {
	int i = 0, j = 0;
	Dimension dim_t;
	while (i < first.orients.size() &&j < second.orients.size()) {
		dim_t.orients.push_back(verticalSlice(first.orients[i], second.orients[j]));
		dim_t.use.push_back(0);
		/*if (i == first.orients.size() - 1) j++;//i不能再往後
		else if (j == first.orients.size() - 1) i++;//j不能往後
		else */if (first.orients[i].second > second.orients[j].second) i++;
		else if (first.orients[i].second < second.orients[j].second) j++;
		else if (first.orients[i].second == second.orients[j].second) {i++;j++; }
	}
	sort(dim_t.orients.begin(), dim_t.orients.end());
	return dim_t;
}
pair<int, int> horizontalSlice(pair<int, int> a, pair<int, int>  b) {
	pair<int, int > temp;
	temp.first= max(a.first, b.first); //max of width
	temp.second = a.second + b.second;//sum of height
	return temp;
}
Dimension horizontalMerge(Dimension first, Dimension second) {
	int i = 0, j = 0;
	Dimension dim_t;
	while (i < first.orients.size() && j < second.orients.size()) {
		dim_t.orients.push_back(horizontalSlice(first.orients[i], second.orients[j]));
		dim_t.use.push_back(0);
		/*if (i == first.orients.size() - 1) j++;//i不能再往後
		else if (j == first.orients.size() - 1) i++;//j不能往後
		else*/ if (first.orients[i].first > second.orients[j].first ) i++;
		else if (first.orients[i].first < second.orients[j].first ) j++;
		else if (first.orients[i].first == second.orients[j].first) { i++; j++; }
	}
	sort(dim_t.orients.begin(), dim_t.orients.end());
	return dim_t;
}
bool mycompare(Dimension a,Dimension b) {
	if (a.position > b.position) return false;
	else return true;
}
void check(string cut,pair<int,int> res,Dimension &l, Dimension &r) {
	int i = 0, j = 0;
	if (cut == "H") {
		for (int i = 0; i < l.orients.size(); i++) {
			for (int j = 0; j < r.orients.size(); j++) {
				if (res.first == max(l.orients[i].first, r.orients[j].first) && res.second == l.orients[i].second + r.orients[j].second) {
					l.use[i] = true; r.use[j] = true;
					return;
				}
			}
		}
		/*while (i < l.orients.size() && j < r.orients.size()) {
			if (res.first == max(l.orients[i].first, r.orients[j].first) && res.second == l.orients[i].second + r.orients[j].second) {
				l.use[i] = true; r.use[j] = true;
			}
			if (i == l.orients.size() - 1) j++;
			else if (j == l.orients.size() - 1) i++;
			else if (l.orients[i].first > r.orients[j].first) i++;
			else if (l.orients[i].first < r.orients[j].first) j++;
			else if (l.orients[i].first == r.orients[j].first) { i++; j++; }
		}*/
	}
	else if (cut == "V") {
		for (int i = 0; i < l.orients.size(); i++) {
			for (int j = 0; j < r.orients.size(); j++) {
				if (res.first == l.orients[i].first + r.orients[j].first && res.second == max(l.orients[i].second,  r.orients[j].second)) {
					l.use[i] = true; r.use[j] = true;
					return;
				}
			}
		}
		/*while (i < l.orients.size() && j < r.orients.size()) {
			if (res.second == max(l.orients[i].second, r.orients[j].second) && res.first == l.orients[i].first + r.orients[j].first) {
				l.use[i] = true; r.use[j] = true;
			}
			if (i == l.orients.size() - 1) j++;
			else if (j == l.orients.size() - 1) i++;
			else if (l.orients[i].second > r.orients[j].second) i++;
			else if (l.orients[i].second < r.orients[j].second) j++;
			else if (l.orients[i].second == r.orients[j].second) { i++; j++; }
		}*/
	}
}
void stockmeyer(vector<string> expression, vector<Block>& block_t) {
	stack<Dimension> stack;
	int top = 0;
	global_dimension.clear();
	constructDimension(expression);
	for (int i = 0; i < expression.size(); i++) {
		if (expression[i] != "H" && expression[i] != "V") {
			Dimension t = indexfordim[i];
			stack.push(t);
		}
		else {
			Dimension res;
			Dimension first = stack.top(); stack.pop();
			Dimension second = stack.top(); stack.pop();
			sort(first.orients.begin(), first.orients.end(), greater<pair<int, int>>());
			sort(second.orients.begin(), second.orients.end(),greater<pair<int, int>>());
			if (expression[i] == "V") {
				res = verticalMerge(first, second);
			}
			else if (expression[i] == "H") {
				res = horizontalMerge(first, second);
			}
			res.name = expression[i]; res.position = i;
			res.r_child = first.position; res.l_child = second.position;
			global_dimension.push_back(res);
			indexfordim[i] = res;
			stack.push(res);
		}
	}
	Dimension root = stack.top();
	sort(global_dimension.begin(), global_dimension.end(), mycompare);
	int area = INT_MAX,pos=0;
	for (int i = 0; i < root.orients.size(); i++) {
		if (area > root.orients[i].first * root.orients[i].second ) {
			area = root.orients[i].first * root.orients[i].second;
			pos = i;
		}
	}indexfordim[root.position].use[pos] = true;

	for (int i = expression.size() - 1; i >= 0; i--) {
		int l_pos = indexfordim[i].l_child, r_pos = indexfordim[i].r_child;
		if (expression[i] == "H" || expression[i] == "V") {
			for (int j = 0; j < indexfordim[i].use.size(); j++) {
				if (indexfordim[i].use[j] == true) {
					check(expression[i], indexfordim[i].orients[j], indexfordim[indexfordim[i].l_child], indexfordim[indexfordim[i].r_child]);
					break;
				}
			}
		}
		else {
			for (int j = 0; j < indexfordim[i].use.size(); j++) {
				if (indexfordim[i].use[j] == true) {
					//跟原本同個方向
					if (indexfordim[i].orients[j].first == block_t[indexforsb[indexfordim[i].name]].w &&
						indexfordim[i].orients[j].second == block_t[indexforsb[indexfordim[i].name]].h){
						continue;
					}
					//有轉向
					else {
						Rotate(block_t, indexfordim[i].name);
					}
				}
			}
		}
	}
}
int main() {
	srand(time(NULL));
	/*read file*/
	//--------------------------------------------------------------------------------------
	int total_block_area = read_hardblock("HW3\\testcases\\n100.hardblocks");
	read_net("HW3\\testcases\\n100.nets");
	read_pl("\HW3\\testcases\\n100.pl");
	//dead_space_ratio = argv[4];
	/*set initial floorplan, and its polish expression*/
	limitfloorplain =  sqrt((total_block_area) * (1 + 0.1));
	cout << "limitfloorplain: "<< limitfloorplain << endl;
	//Calculate initial floorplan area

	init_Floorplan(); //給出polish_expression

	for (int i = 0; i < polish_expression.size(); i++) {
		if (polish_expression[i] == "V" || polish_expression[i] == "H") {
			chain.push_back(i);
		}
		else operand_pos.push_back(i);
	}
	//stockmeyer(polish_expression,blocks);
	//---------------------------------------------------------------------------------------
	//SA
	int temperature = 10000, limit_T = 1,reject=0;
	int K = 100;
	double area, min_area;
	min_area = area = updating_coord(polish_expression, blocks);
	double min_wirelength,wirelength;
	min_wirelength = wirelength = updating_wirelength(blocks);
	vector<Block> best_blocks = blocks;
	vector<Block> temp_blocks = blocks;
	map<string, int> best_indexforsb = indexforsb;
	vector<string> new_polish;
	double last_cost = 0.5 * area + 0.5 * wirelength;
	while (temperature > limit_T) {
		for (int i = 0; i < K; i++) {
			new_polish = Perturbation(temp_blocks);
			area = updating_coord(new_polish, temp_blocks);
			wirelength = updating_wirelength(temp_blocks);
			double new_cost = 0.5 * area + 0.5 * wirelength;
			if (flag == 0) {//未曾符合
				reject++;
				if (reject <= 30) continue;
				else { temp_blocks = best_blocks; reject = 0; }
			}
			else if (flag == 1 && current_flag == 1) {//此回合符合要求，看有無改善
				if ((new_cost - last_cost) < 0) { //有改善
					polish_expression = new_polish;
					min_wirelength = wirelength;
					best_blocks = temp_blocks;
					best_indexforsb = indexforsb;
					last_cost = new_cost;
				}
				else {
					reject++;
					if (reject <= 30) continue;
					else { temp_blocks = best_blocks; reject = 0; }
				}
			}
			else { //flag==1,current_flag==0//曾符合過，但此回合不符合
				reject++;
				if (reject <= 30) continue;
				else { temp_blocks = best_blocks; reject = 0; }
			}
		}
		temperature *= 0.9;
	}
	cout <<"min wirelength:" << min_wirelength << endl;
	/*---------------output------------------*/
	ofstream out;
	out.open("n100.floorplan");
	out << "Wirelength" << " " << min_wirelength << "\n";
	out << "Blocks" << "\n";
	for (int i = 0; i < best_blocks.size(); ++i) {
		out << best_blocks[i].name << " ";
		out << best_blocks[i].coord.first << " " << best_blocks[i].coord.second << " ";
		out << best_blocks[i].rotate<< "\n";
	}
	out.close();
}
