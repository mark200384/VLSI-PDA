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

struct Dimension {
	int height, width;
	vector<Dimension> orients;
	Dimension(int h, int w) {
		height = h;
		width = w;
	}
	Dimension() { height = 0; width = 0; }
};

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
bool compare2(Dimension& a, Dimension& b) {
	if (a.height == b.height) return a.width > b.width;
	else return a.width > b.width;
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
	if (temp.w > limitfloorplain || temp.h > limitfloorplain) return INT_MAX;
	//cout << "W:" << (double)stack[top - 1].w << " H:" << (double)stack[top - 1].h << endl;
	//if (temp.w < limitfloorplain && temp.h < limitfloorplain) cout << "yes!!!";
	else return temp.w * temp.h;
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
		/*blocks[indexforsb[temp_polish[pos1]]].rotate = (blocks[indexforsb[temp_polish[pos1]]].rotate + 1) % 2;
		int temp = blocks[indexforsb[temp_polish[pos1]]].w;
		blocks[indexforsb[temp_polish[pos1]]].w = blocks[indexforsb[temp_polish[pos1]]].h;
		blocks[indexforsb[temp_polish[pos1]]].h = temp;*/
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
	/*for (int i = 0; i < polish_expression.size(); i++) {
		cout << polish_expression[i] << " ";
	}cout << endl;*/
}
map<int, Dimension> indexfordim;
void constructDim(vector<Dimension> &dim,vector<string>expression,vector<Block>& block_t) {
	for (int i = 0; i < expression.size(); i++) {
		Dimension t;
		int index2block = indexforsb[expression[i]];
		if (expression[i] != "H" && expression[i] != "V") { //operand
			t.orients.push_back(Dimension(blocks[index2block].h, blocks[index2block].w));
			if (blocks[index2block].h != blocks[index2block].w) {
				t.orients.push_back(Dimension(blocks[index2block].w, blocks[index2block].h));
			}
			indexfordim[i] = t;
		}
		dim.push_back(t);
	}
}


Dimension verticalSlice(Dimension first, Dimension second) {
	Dimension dim_t;
	dim_t.height = max(first.height, second.height);
	dim_t.width = first.width + second.width;
	dim_t.orients.push_back(dim_t);
	return dim_t;
}
Dimension verticalMerge(Dimension first, Dimension second) {
	int pos1 =0, pos2 = 0;
	Dimension dim_t;
	while (pos1 < first.orients.size() && pos2 < second.orients.size()) {
		dim_t.orients.push_back(verticalSlice(first.orients[pos1],second.orients[pos2]));
		if (first.orients[pos1].height > second.orients[pos2].height) pos1++;
		else if (first.orients[pos1].height < second.orients[pos2].height) pos2++;
		else if (first.orients[pos1].height == second.orients[pos2].height) { pos1++; pos2++; }
	}
	sort(dim_t.orients.begin(), dim_t.orients.end(),compare2);
	return dim_t;
}
Dimension HSlice(Dimension first, Dimension second){
	Dimension dim_t;
	dim_t.height = first.height  + second.height;
	dim_t.width = max(first.width, second.width);
	dim_t.orients.push_back(dim_t);
	return dim_t;
}
Dimension horizontalMerge(Dimension first, Dimension second) {
	int pos1 = 0, pos2 = 0;
	Dimension dim_t;
	while (pos1 < first.orients.size() && pos2 < second.orients.size()) {
		dim_t.orients.push_back(HSlice(first.orients[pos1], second.orients[pos2]));
		if (first.orients[pos1].width > second.orients[pos2].width) pos1++;
		else if (first.orients[pos1].width < second.orients[pos2].width) pos2++;
		else if (first.orients[pos1].width == second.orients[pos2].width) { pos1++; pos2++; }
	}
	sort(dim_t.orients.begin(), dim_t.orients.end(),compare2);
	return dim_t;
}
void stockmeyer(vector<string> expression,vector<Block> &block_t) {
	stack<Dimension> stack;
	vector<Dimension> dimension;
	int top = 0;
	constructDim(dimension,expression,block_t);
	for (int i = 0; i < expression.size(); i++) {
		int index2block = indexforsb[expression[i]];
		if (expression[i] != "H" && expression[i] != "V") {
			stack.push(indexfordim[i]);//dimension at expression pos i
		}
		else {
			Dimension dim_t;
			Dimension dim_r = stack.top(); stack.pop();
			sort(dim_r.orients.begin(), dim_r.orients.end(),compare2);
			Dimension dim_l = stack.top(); stack.pop();
			sort(dim_l.orients.begin(), dim_l .orients.end(),compare2);
			int pos_r = 0, pos_l = 0;
			if (expression[i] == "V") { //lr
				dim_t = verticalMerge(dim_r,dim_l);
			}
			else if (expression[i] == "H") {//	r/l
				dim_t = horizontalMerge(dim_r, dim_l);
			}
			indexfordim[i] = dim_t;
			stack.push(dim_t);
		}
	}
	Dimension ttt = stack.top();
}
int main() {
	srand(time(NULL));
	/*read file*/
	//--------------------------------------------------------------------------------------
	int total_block_area = read_hardblock("C:\\Users\\user\\Desktop\\VLSI PDA\\HW3\\HW3\\testcases\\n100.hardblocks"/*argv[1]*/);
	read_net(/**//*argv[2]*/"C:\\Users\\user\\Desktop\\VLSI PDA\\HW3\\HW3\\testcases\\n100.nets");
	read_pl(/**//*argv[3]*/"C:\\Users\\user\\Desktop\\VLSI PDA\\HW3\\HW3\\testcases\\n100.pl");

	//dead_space_ratio = argv[4];
	/*set initial floorplan, and its polish expression*/
	limitfloorplain =  sqrt((total_block_area) * (1 + 0.1));
	cout << "limitfloorplain: "<< limitfloorplain << endl;
	//Calculate initial floorplan area

	init_Floorplan(); //給出polish_expression、各sb coordinate
	for (int i = 0; i < polish_expression.size(); i++) {
		if (polish_expression[i] == "V" || polish_expression[i] == "H") {
			chain.push_back(i);
		}
		else operand_pos.push_back(i);
	}
	stockmeyer(polish_expression,blocks);
	//---------------------------------------------------------------------------------------
	//SA
	int temperature = 10000, limit_T = 1;
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
			if ((new_cost - last_cost) < 0) { //有改善
				polish_expression = new_polish;
				min_wirelength = wirelength;
				//min_wirelength = min(wirelength, min_wirelength);
				best_blocks = temp_blocks;
				best_indexforsb = indexforsb;
				last_cost = new_cost;
			}
			else {
				temp_blocks = best_blocks;
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
