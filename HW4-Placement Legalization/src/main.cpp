#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <climits>
#include <time.h>
#include <algorithm>
using namespace std;
int NumRows = 0, MaxDisplacement = 0, NumNodes = 0, NumTerminals = 0, NumMovableNodes = 0;
int total_displacement = 0;
map<string, int> indexfornode;
int siteWidth;
int bestXfornode;
class NODE_info {
public:
    string nodeName;
    int width, height;
    int weight = 4;
    bool moveType = 0;//1是
    pair<double, double> initial_coord;
    //pair<double, double> new_coord;
    pair<int, int> new_coord;
};
class cluster {
public:
    int x_coord;//start x coord
    int width = 0;//total width
    double q;
    int weight = 0;
    vector<NODE_info> nodes;
};
class subRow {
public:
    int minX = 0, maxX = 0, width = 0;
    vector<cluster> clusters;
    int indextoRow;
};
class ROW_info {
public:
    int y_coord;
    int height;
    int Sitewidth;
    int NumSites;
    int SubrowOrigin;
    vector<subRow> subrow;
};
vector<subRow> subrow;
vector<ROW_info> row_t;
vector<NODE_info> node_t;
vector<string> split(const string& str, const char& delimiter) {
    vector<string> result;
    stringstream ss(str);
    string tok;
    while (getline(ss, tok, delimiter)) {
        result.push_back(tok);
    }
    return result;
}
vector<NODE_info> Readnode(string path) {
    ifstream readnode;
    readnode.open(path);
    if (!readnode) cout << "read .node error" << endl;
    string tmp;
    getline(readnode, tmp);
    vector<string> tt = split(tmp, ' ');
    tt.erase(remove(tt.begin(), tt.end(), ""), tt.end());
    NumNodes = stoi(tt[2]);
    getline(readnode, tmp);
    tt = split(tmp, ' ');
    NumTerminals = stoi(tt[2]);
    NumMovableNodes = NumNodes - NumTerminals;
    getline(readnode, tmp);//空白行
    vector<NODE_info> res;
    NODE_info node;
    while (getline(readnode, tmp)) {
        NODE_info node;
        tt = split(tmp, ' ');
        tt.erase(remove(tt.begin(), tt.end(), ""), tt.end());
        node.nodeName = tt[0];
        node.width = stoi(tt[1]);
        node.height = stoi(tt[2]);
        if (tt.size() == 4) node.moveType = 1;
        res.push_back(node);
        indexfornode[node.nodeName] = res.size() - 1;
    }
    readnode.close();
    return res;
}
void Readpl(string path, vector<NODE_info>& node_t) {
    ifstream readpl;
    readpl.open(path);
    if (!readpl) cout << "read .pl file error!" << endl;
    string tmp;
    vector<string> tt;
    while (getline(readpl, tmp)) {
        tt = split(tmp, ' ');
        tt.erase(remove(tt.begin(), tt.end(), ""), tt.end());
        node_t[indexfornode[tt[0]]].new_coord.first = node_t[indexfornode[tt[0]]].initial_coord.first = stod(tt[1]);
        node_t[indexfornode[tt[0]]].new_coord.second = node_t[indexfornode[tt[0]]].initial_coord.second = stod(tt[2]);
        if (tt.size() == 6) node_t[indexfornode[tt[0]]].moveType = 1;
    }
    readpl.close();
}
vector<ROW_info> Readscl(string path) {
    ifstream readscl;
    readscl.open(path);
    if (!readscl) cout << "read .scl file error!" << endl;
    string tmp;
    getline(readscl, tmp);
    vector<string> tt = split(tmp, ' ');
    NumRows = stoi(tt[2]);
    getline(readscl, tmp);//空白行
    int round = NumRows;
    vector<ROW_info> res;
    while (round--) {
        ROW_info row_t;
        getline(readscl, tmp);//CoreRow Horizontal
        getline(readscl, tmp);//Coordinate
        tt = split(tmp, ' ');
        tt.erase(remove(tt.begin(), tt.end(), ""), tt.end());
        row_t.y_coord = stoi(tt[2]);
        getline(readscl, tmp);//Height
        tt = split(tmp, ' ');
        tt.erase(remove(tt.begin(), tt.end(), ""), tt.end());
        row_t.height = stoi(tt[2]);
        getline(readscl, tmp);//SiteWidth
        tt = split(tmp, ' ');
        tt.erase(remove(tt.begin(), tt.end(), ""), tt.end());
        row_t.Sitewidth = stoi(tt[2]);
        getline(readscl, tmp);//Numsites
        tt = split(tmp, ' ');
        tt.erase(remove(tt.begin(), tt.end(), ""), tt.end());
        row_t.NumSites = stoi(tt[2]);
        getline(readscl, tmp);//subroworigin
        tt = split(tmp, ' ');
        tt.erase(remove(tt.begin(), tt.end(), ""), tt.end());
        row_t.SubrowOrigin = stoi(tt[2]);
        getline(readscl, tmp);//End
        res.push_back(row_t);
    }
    readscl.close();
    return res;
}
vector<string> Readaux(string path) {
    ifstream readaux;
    readaux.open(path);
    if (!readaux) {
        cout << "read .aux error" << endl;
    }
    string tmp;// , nodefile, plfile, sclfile;
    getline(readaux, tmp);
    vector<string> file = split(tmp, ' ');
    getline(readaux, tmp);
    vector<string> tt = split(tmp, ' ');
    tt.erase(remove(tt.begin(), tt.end(), ""), tt.end());
    MaxDisplacement = stoi(tt[2]);
    readaux.close();
    cout << "MaxDisplacement:" << MaxDisplacement << endl;
    string dest = path.substr(0, path.find_last_of('/'));
    file[2] = dest + '/' + file[2];
    file[3] = dest + '/' + file[3];
    file[4] = dest + '/' + file[4];
    return file;
}
double calculating_cost(NODE_info node) {
    double x = abs(node.new_coord.first*siteWidth - node.initial_coord.first*siteWidth);
    double y = abs(node.new_coord.second - node.initial_coord.second);
    return sqrt(pow(x, 2) + pow(y, 2));
}
double calculating_displacement(vector<NODE_info>& node_t) {
    double total_displacement = 0, max_displacement = 0;
    for (int i = 0; i < node_t.size(); i++) {
        double x = abs(node_t[i].new_coord.first - node_t[i].initial_coord.first);
        double y = abs(node_t[i].new_coord.second - node_t[i].initial_coord.second);
        double displacement = sqrt(pow(x, 2) + pow(y, 2));
        total_displacement += displacement;
        max_displacement = max(max_displacement, displacement);
    }
    cout << "max_displacement(by node_t): " << max_displacement << '\n';
    return total_displacement;
}
void create_subRow(vector<NODE_info>& node_t, vector<ROW_info>& row_t) {
    for (int i = 0; i < NumRows; i++) {
        subRow t;
        t.indextoRow = i;
        t.minX = row_t[i].SubrowOrigin;
        t.width = row_t[i].NumSites;
        t.maxX = t.minX + t.width;
        row_t[i].subrow.push_back(t);
    }
  
    for (int i = NumNodes - NumTerminals; i < NumNodes; i++) {
    //cout << "terminal number: " <<i <<'\n';
        for (int j = 0; j < NumRows; j++) {
            if (row_t[j].y_coord >= node_t[i].initial_coord.second + node_t[i].height || row_t[j].y_coord < node_t[i].initial_coord.second) {
                continue;
            }
            int last_pos = row_t[j].subrow.size() - 1;
            //subRow* last = &row_t[j].subrow[last_pos];
            if (row_t[j].subrow[last_pos].minX < node_t[i].initial_coord.first) {
                if (row_t[j].subrow[last_pos].maxX > node_t[i].initial_coord.first + node_t[i].width) {//加右邊
                    subRow t;
                    t.minX = node_t[i].initial_coord.first + node_t[i].width;
                    t.maxX = row_t[j].subrow[last_pos].maxX;
                    t.width = t.maxX - t.minX;
                    row_t[j].subrow.push_back(t);
                    //last = &row_t[j].subrow[last_pos];
                }
                row_t[j].subrow[last_pos].maxX = node_t[i].initial_coord.first;
                row_t[j].subrow[last_pos].width = row_t[j].subrow[last_pos].maxX - row_t[j].subrow[last_pos].minX;
            }
            else {//有無重疊
                //t->minX = node_t[i].initial_coord.first
                if (row_t[j].subrow[last_pos].maxX > node_t[i].initial_coord.first + node_t[i].width) {
                    row_t[j].subrow[last_pos].minX = node_t[i].initial_coord.first + node_t[i].width;
                    row_t[j].subrow[last_pos].width = row_t[j].subrow[last_pos].maxX - row_t[j].subrow[last_pos].minX;
                }
                else {//t.minX = t.maxX
                    row_t[i].subrow.pop_back();
                }
            }
        }
    }
}
int find_row(vector<NODE_info>& node_t, vector<ROW_info>& row_t, int node_index) {
    //Binary search
 //cout <<"find row start" <<'\n';
    int up = NumRows - 1, down = 0;
    double target = node_t[node_index].initial_coord.second;
    int mid = down + (up - down) / 2;
    while (up >= down) {
        if (target == row_t[mid].y_coord) return mid;
        else if (target > row_t[mid].y_coord)down = mid + 1;// 往上找
        else if (target < row_t[mid].y_coord) up = mid - 1;//往下找
        mid = down + (up - down) / 2;
    }
    if (abs(target - row_t[up].y_coord) < abs(target - row_t[down].y_coord)) {
        if (abs(target - row_t[up].y_coord) < abs(target - row_t[up + 1].y_coord)) {
            return up;
        }
        else return up + 1;
    }
    else {
        if (abs(target - row_t[down].y_coord) < abs(target - row_t[down - 1].y_coord)) {
            return down;
        }
        else return down - 1;
    }
}
int find_subrow(NODE_info& node, ROW_info& row) {
	vector<subRow> subrow_t;
	subrow_t = row.subrow;
	if (subrow_t.size() == 0) return -1;//換別行找
	if (node.initial_coord.first <= subrow_t[0].minX) {
		if (node.width <= subrow_t[0].width)
			return 0;
	}
	for (int i = 0; i < subrow_t.size(); i++) {
		if (node.initial_coord.first >= subrow_t[i].maxX) continue;
		if (node.initial_coord.first >= subrow_t[i].minX) {
			if (node.width <= subrow_t[i].width)
				return i;
		}
		else {//i-1~i之間 //i==0已被檢查過(除非subrow_t[0]放不下
			if (i > 0) {
				if (abs(node.initial_coord.first - (subrow_t[i-1].maxX - node.width)) < abs(node.initial_coord.first - subrow_t[i].minX)) {
					//跟i-1比較近
					if (node.width <= row.subrow[i - 1].width) {
						return i - 1;
					}
					else if (node.width <= row.subrow[i].width) {
						return i;
					}
				}
				else {
					if (node.width <= row.subrow[i].width) {
						return i;
					}
					else if (node.width <= row.subrow[i - 1].width)
						return i - 1;
				}
			}
		}
	}
	return -1;
}
bool cmp(const NODE_info& a, const NODE_info& b) {
    return a.initial_coord.first < b.initial_coord.first;
}
bool cmp2(const NODE_info& a, const NODE_info& b) {
    if (a.new_coord.second == b.new_coord.second) {
        return a.new_coord.first < b.new_coord.first;
    }
    return a.new_coord.second < b.new_coord.second;
}
void PlacceRowTrial(NODE_info& node, ROW_info& row, int best_subrow) {
	subRow current_subrow = row.subrow[best_subrow];
	double x_coord = node.initial_coord.first;
	if (node.initial_coord.first <= row.subrow[best_subrow].minX) {//最左邊
		x_coord = row.subrow[best_subrow].minX;
	}
	if (node.initial_coord.first > row.subrow[best_subrow].maxX - node.width) {//最右邊
		x_coord = row.subrow[best_subrow].maxX - node.width;
	}
	int last = row.subrow[best_subrow].clusters.size() - 1;
	//cluster last_cluster = row.subrow[best_subrow].clusters[last];
	if (last == -1) {//no cluster
		node.new_coord.first = x_coord;
	}
	else if (current_subrow.clusters[last].x_coord + current_subrow.clusters[last].width <= x_coord) {//overlap-free
		node.new_coord.first = x_coord;
	}
	else {//overlap
		cluster last_cluster = current_subrow.clusters[last];//調整last cluster位置
		int weight = last_cluster.weight + node.weight;
		double q = current_subrow.clusters[last].q + node.weight * (x_coord - current_subrow.clusters[last].width);
		int trial_width = node.width + last_cluster.width;
		double cluster_x = 0.0;
		while (1) {
			cluster_x = q / weight;
			if (cluster_x < current_subrow.minX)
				cluster_x = current_subrow.minX;
			if (cluster_x > current_subrow.maxX - last_cluster.width)
				cluster_x = current_subrow.maxX - last_cluster.width;
			if (last > 0) {
				cluster previous = current_subrow.clusters[last - 1];
				if (previous.x_coord + previous.width > cluster_x) {//overlap
					//q = q + current_cluster.q - current_cluster.weight * previous.width;
					q = previous.q + q - weight * previous.width;
					weight = previous.weight + weight;
					trial_width = previous.width + trial_width;
					last--;//往前走
					last_cluster = current_subrow.clusters[last];
				}
				else break;
			}
			else break;
		}
		node.new_coord.first = cluster_x + trial_width - node.width;
	}
	node.new_coord.second = row.y_coord;
}
//put node into row[row_index].subrow[subrow_index]
cluster ADDcell(NODE_info node, int row_index, int subrow_index,double x) {
    cluster new_c;
    new_c.nodes.push_back(node);
    new_c.q = node.weight * x;
    new_c.weight += node.weight;
    new_c.x_coord = x;
    new_c.width += node.width;
    return new_c;
}
void PlaceRowFinal(NODE_info& node, int row_index, int subrow_index) {
	node.new_coord.second = row_t[row_index].y_coord;
	row_t[row_index].subrow[subrow_index].width -= node.width;
	double x_coord = node.initial_coord.first;
	if (node.initial_coord.first < row_t[row_index].subrow[subrow_index].minX) {
		x_coord = row_t[row_index].subrow[subrow_index].minX;
	}
	if (node.initial_coord.first > row_t[row_index].subrow[subrow_index].maxX - node.width) {
		x_coord = row_t[row_index].subrow[subrow_index].maxX - node.width;
	}
	int last = row_t[row_index].subrow[subrow_index].clusters.size() - 1;
	//cluster *last_cluster = &row_t[row_index].subrow[subrow_index].clusters[last];
	if (last == -1) {//new
		cluster new_c = ADDcell(node, row_index, subrow_index, x_coord);
		node.new_coord.first = x_coord;
		row_t[row_index].subrow[subrow_index].clusters.push_back(new_c);
	}
	else {
		cluster* last_cluster = &row_t[row_index].subrow[subrow_index].clusters[last];
		if (last_cluster->x_coord + last_cluster->width <= x_coord) {
			cluster new_c = ADDcell(node, row_index, subrow_index, x_coord);
			node.new_coord.first = x_coord;
			row_t[row_index].subrow[subrow_index].clusters.push_back(new_c);
		}
		else {//overlap
			row_t[row_index].subrow[subrow_index].clusters[last].nodes.push_back(node);
			row_t[row_index].subrow[subrow_index].clusters[last].weight += node.weight;
			row_t[row_index].subrow[subrow_index].clusters[last].q += node.weight * (x_coord - row_t[row_index].subrow[subrow_index].clusters[last].width);
			row_t[row_index].subrow[subrow_index].clusters[last].width += node.width;
			while (1) {
				row_t[row_index].subrow[subrow_index].clusters[last].x_coord = row_t[row_index].subrow[subrow_index].clusters[last].q / row_t[row_index].subrow[subrow_index].clusters[last].weight;
				
				if (row_t[row_index].subrow[subrow_index].clusters[last].x_coord < row_t[row_index].subrow[subrow_index].minX) {
					row_t[row_index].subrow[subrow_index].clusters[last].x_coord = row_t[row_index].subrow[subrow_index].minX;
				}
				if (row_t[row_index].subrow[subrow_index].clusters[last].x_coord > row_t[row_index].subrow[subrow_index].maxX - row_t[row_index].subrow[subrow_index].clusters[last].width) {
					row_t[row_index].subrow[subrow_index].clusters[last].x_coord = row_t[row_index].subrow[subrow_index].maxX - row_t[row_index].subrow[subrow_index].clusters[last].width;
				}
				if (last > 0) {
					int prev = last - 1;
					if (row_t[row_index].subrow[subrow_index].clusters[prev].x_coord + row_t[row_index].subrow[subrow_index].clusters[prev].width > row_t[row_index].subrow[subrow_index].clusters[last].x_coord) {
						int pos = 0;
						while (pos < row_t[row_index].subrow[subrow_index].clusters[last].nodes.size()) {
							row_t[row_index].subrow[subrow_index].clusters[prev].nodes.push_back(row_t[row_index].subrow[subrow_index].clusters[last].nodes[pos]);
							pos++;
						}

						row_t[row_index].subrow[subrow_index].clusters[prev].weight += row_t[row_index].subrow[subrow_index].clusters[last].weight;
						row_t[row_index].subrow[subrow_index].clusters[prev].q += row_t[row_index].subrow[subrow_index].clusters[last].q - row_t[row_index].subrow[subrow_index].clusters[last].weight * row_t[row_index].subrow[subrow_index].clusters[prev].width;
						row_t[row_index].subrow[subrow_index].clusters[prev].width += row_t[row_index].subrow[subrow_index].clusters[last].width;
						//if (row_t[row_index].subrow[subrow_index].clusters[last].nodes.empty()) {//移除該cluster
						row_t[row_index].subrow[subrow_index].clusters.pop_back();
						//}
						last--;
					}
					else break;
				}
				else break;
			}
			node.new_coord.first = row_t[row_index].subrow[subrow_index].clusters[last].x_coord + row_t[row_index].subrow[subrow_index].clusters[last].width - node.width;
		}
	}
}
void Abacus(vector<NODE_info>& node_t, vector<ROW_info>& row_t) {
    int NumMovable = NumNodes - NumTerminals;
    int best_row, best_subrow;//index
    sort(node_t.begin(), node_t.begin() + NumMovable, cmp);
    sort(node_t.begin() + NumMovable, node_t.end(), cmp);
    //set indexfornode
    for (int i = 0; i < node_t.size(); i++) {
        indexfornode[node_t[i].nodeName] = i;
    }
  //cout<<"abacus start"<<endl;
    create_subRow(node_t, row_t);
    //abacus start
    double temp_maxdisplacement = 0;
    for (int i = 0; i < NumMovableNodes; i++) {
    //cout << "abacus node: "<< i <<"start" <<endl;
        double best_cost = INT_MAX;
        best_row = find_row(node_t, row_t, i);
        int best_subrow = find_subrow(node_t[i], row_t[best_row]);
        if (best_subrow != -1) {
            PlacceRowTrial(node_t[i], row_t[best_row], best_subrow);//go current_row and find best_subrow
            bestXfornode = node_t[i].new_coord.first;
            best_cost = calculating_cost(node_t[i]);//if put this, how many displacement
        }
        int start = best_row;
        for (int up = start + 1; up < row_t.size() - 1; up++) {//往上找
            if (abs(node_t[i].initial_coord.second - row_t[up].y_coord) >= best_cost) break;//太遠
            else {      
                int temp_subrow = find_subrow(node_t[i], row_t[up]);
                if (temp_subrow != -1) {
                    PlacceRowTrial(node_t[i], row_t[up], temp_subrow);
                    double cost = calculating_cost(node_t[i]);
                    if (cost < best_cost) {
                        best_cost = cost;
                        best_row = up;
                        best_subrow = temp_subrow;
                    }
                }
            }
        }
        for (int down = start - 1; down >= 0; down--) {//往下找
            if (abs(node_t[i].initial_coord.second - row_t[down].y_coord) >= best_cost) break;//太遠
            else {
                //current_row = row_t[down];
                int temp_subrow = find_subrow(node_t[i], row_t[down]);
                if (temp_subrow != -1) {
                    PlacceRowTrial(node_t[i], row_t[down], temp_subrow);
                    double cost = calculating_cost(node_t[i]);
                    if (cost < best_cost) {
                        best_cost = cost;
                        best_row = down;
                        best_subrow = temp_subrow;
                    }
                }
            }
        }
        if (best_cost <= MaxDisplacement) {
      //cout << "node i:" <<i <<'\n';
            PlaceRowFinal(node_t[i], best_row, best_subrow);
            for (int k = 0; k < row_t[best_row].subrow[best_subrow].clusters.size(); k++) {
                int startX = row_t[best_row].subrow[best_subrow].clusters[k].x_coord;
                for (int m = 0; m < row_t[best_row].subrow[best_subrow].clusters[k].nodes.size(); m++) {
                    row_t[best_row].subrow[best_subrow].clusters[k].nodes[m].new_coord.first = startX;
                    node_t[indexfornode[row_t[best_row].subrow[best_subrow].clusters[k].nodes[m].nodeName]].new_coord.first = startX;
                    startX += row_t[best_row].subrow[best_subrow].clusters[k].nodes[m].width;
                }
            }
        }
        else {
            int start = best_row;
            for (int up = start; up < row_t.size(); up++) {
                if (abs(node_t[i].initial_coord.second - row_t[up].y_coord) >= best_cost) break;
                for (int l = 0; l < row_t[up].subrow.size(); l++) {
                    if (node_t[i].width <= row_t[up].subrow[l].width) {//fitted
                        PlacceRowTrial(node_t[i], row_t[up], l);
                        double cost = calculating_cost(node_t[i]);
                        if (cost < best_cost) {
                            best_cost = cost;
                            best_row = up;
                            best_subrow = l;
                        }
                    }
                }
            }
            for (int down = start - 1; down >= 0; down--) {
                if (abs(node_t[i].initial_coord.second - row_t[down].y_coord) >= best_cost) break;
                for (int l = 0; l < row_t[down].subrow.size(); l++) {
                    if (node_t[i].width <= row_t[down].subrow[l].width) {//fitted
                        PlacceRowTrial(node_t[i], row_t[down], l);
                        double cost = calculating_cost(node_t[i]);
                        if (cost < best_cost) {
                            best_cost = cost;
                            best_row = down;
                            best_subrow = l;
                        }
                    }
                }
            }
            //if (best_cost > MaxDisplacement / siteWidth) cout << "i:" << i << "dissatisfied maxdisplacement." << '\n';
            if (best_cost != INT_MAX) {
                PlaceRowFinal(node_t[i], best_row, best_subrow);
                for (int k = 0; k < row_t[best_row].subrow[best_subrow].clusters.size(); k++) {
                    int startX = row_t[best_row].subrow[best_subrow].clusters[k].x_coord;
                    for (int m = 0; m < row_t[best_row].subrow[best_subrow].clusters[k].nodes.size(); m++) {
                        row_t[best_row].subrow[best_subrow].clusters[k].nodes[m].new_coord.first = startX;
                        node_t[indexfornode[row_t[best_row].subrow[best_subrow].clusters[k].nodes[m].nodeName]].new_coord.first = startX;
                        startX += row_t[best_row].subrow[best_subrow].clusters[k].nodes[m].width;
                    }
                }
            }
            else {
                cout << node_t[i].nodeName << " can't not put in.";
            }
        }
        temp_maxdisplacement = max(temp_maxdisplacement, best_cost);
    }
    sort(node_t.begin(), node_t.end(), cmp2);
    for (int i = 0; i < row_t.size(); i++) {
        row_t[i].SubrowOrigin *= siteWidth;
        for (int j = 0; j < row_t[i].subrow.size(); j++) {
            row_t[i].subrow[j].minX *= siteWidth;
            row_t[i].subrow[j].width *= siteWidth;
            row_t[i].subrow[j].maxX *= siteWidth;
            for (int m = 0; m < row_t[i].subrow[j].clusters.size(); m++) {
                row_t[i].subrow[j].clusters[m].x_coord *= siteWidth;
                row_t[i].subrow[j].clusters[m].width *= siteWidth;
                for (int n = 0; n < row_t[i].subrow[j].clusters[m].nodes.size(); n++) {
                    row_t[i].subrow[j].clusters[m].nodes[n].initial_coord.first *= siteWidth;
                    row_t[i].subrow[j].clusters[m].nodes[n].new_coord.first *= siteWidth;
                    row_t[i].subrow[j].clusters[m].nodes[n].width *= siteWidth;
                }
            }
        }
    }   
    for (int i = 0; i < node_t.size(); i++) {
        node_t[i].initial_coord.first *= siteWidth;
        node_t[i].new_coord.first *= siteWidth;
        node_t[i].width *= siteWidth;
    }
    cout << "temp_maxdisplacement: " << temp_maxdisplacement << '\n';
    
}
void Output(string tmp) {
    ofstream out_res;
  out_res.open(tmp);
    for (int i = 0; i < node_t.size(); i++) {
        int first = node_t[i].new_coord.first;
        int second = node_t[i].new_coord.second;
        out_res << node_t[i].nodeName << " " << first << " " << second << '\n';
    }
    out_res.close();
}
int main(int argc, char* argv[]) {
  time_t total_beg, total_end;
    total_beg = clock();
    time_t read_beg, read_end;
    vector<string> read_file;
    read_beg = clock();
    //read_file = Readaux("HW4//ibm01//ibm01.aux");
    read_file = Readaux(argv[1]);// ../testcase/ibm01/ibm01.aux
    node_t = Readnode(read_file[2]);
    Readpl(read_file[3], node_t);//***new_coord順便給值
    row_t = Readscl(read_file[4]);
    read_end = clock();
    cout << "readfile spend:" << double(read_end - read_beg) / CLOCKS_PER_SEC << "sec" << endl;
    
    
    time_t cpu_beg, cpu_end;
    cpu_beg = clock();
    siteWidth = row_t[0].Sitewidth;
    for (int i = 0; i < node_t.size(); i++) {
        node_t[i].initial_coord.first /= siteWidth;
        node_t[i].new_coord.first = node_t[i].initial_coord.first;
        node_t[i].width /= siteWidth;
    }
    for (int i = 0; i < row_t.size(); i++) {
        row_t[i].SubrowOrigin /= siteWidth;
    }
    
    Abacus(node_t, row_t);
    calculating_displacement(node_t);
    cpu_end = clock();
    cout << "Abacus time: " << (double)(cpu_end - cpu_beg) / CLOCKS_PER_SEC << endl;
    
    
    time_t write_beg, write_end;
    write_beg = clock();
  Output(argv[2]);
    //Output("HW4//ibm01//ibm01.aux");
    write_end = clock();
    cout << "write file:" << (double)(write_end - write_beg) / CLOCKS_PER_SEC << endl;
    total_end = clock();
 cout << "total time:" << (double)(total_end - total_beg) / CLOCKS_PER_SEC << endl;
}