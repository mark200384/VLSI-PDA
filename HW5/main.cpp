#include<iostream>
#include<string>
#include<cmath>
#include<vector>
#include<fstream>
using namespace std;

class Die {
public:
	Die(string design_name, int die_x1, int die_y1, int die_x2, int die_y2)
		: x1(die_x1), y1(die_y1),x2(die_x2),y2(die_y2)
	{}
	string get_design_name() { return design_name; }
	int get_x1() { return x1; }
	int get_x2() { return x2; }
	int get_y1() { return y1; }
	int get_y2() { return y2; }
private:
	string design_name = design_name;
	int x1;
	int x2;
	int y1;
	int y2;
};
class Component {
public:
	Component(string lib_name,string inst_name,int x,int y)
		:lib_name(lib_name), inst_name(inst_name),x(x),y(y)
	{}
	Component() {}
	string get_lib_name() { return lib_name;}
	string get_inst_name() { return inst_name; }
	int get_x() { return x; }
	int get_y() { return y; }
private:
	string lib_name;
	string inst_name;
	int x;
	int y;
};
class SpecialNet {
public:
	SpecialNet(string inst_name, string layer, int x1, int y1, int x2, int y2)
		:inst_name(inst_name), layer(layer), x1(x1), y1(y1), x2(x2), y2(y2)
	{}
	SpecialNet() {}
	string get_layer() { return layer; }
	string get_inst_name() { return inst_name; }
	int get_x1() { return x1; }
	int get_x2() { return x2; }
	int get_y1() { return y1; }
	int get_y2() { return y2; }
private:
	string inst_name;
	string layer;
	int x1;
	int x2;
	int y1;
	int y2;
};
int CS_WIDTH = 7100;
int CS_HEIGHT = 6600;
int M3_WIDTH = 440;
int M3_SPACING = 310;
int M4_WIDTH = 1000;
int M4_SPACING = 490;
int CS_X1_TO_DRAIN = 1260;
int CS_Y1_TO_DRAIN = 4100;
string CS_LIB_NAME = "MSBCS";
string VIA34_LIB_NAME = "Via34";
void write_def(string file_name, Die die, vector<Component> component_list, vector<SpecialNet> specialnet_list,string design_name) {
	if (file_name == "") {
		cout << "[Error] Please provide a def file name.\n";
		return;
	}
	ofstream out;
	out.open(file_name);
	out << "VERSION 5.6 ;\n";
	out << "DIVIDERCHAR \"/\" ;\n";
	out << "BUSBITCHARS \"[]\" ;\n";

	out << "DESIGN "<< design_name <<" ;\n\n";
	out << "UNITS DISTANCE MICRONS 1000 ;\n\n";
	out << "PROPERTYDEFINITIONS\n";
	out << "  COMPONENTPIN text STRING ;\n";
	out << "END PROPERTYDEFINITIONS\n\n";
	out << "DIEAREA ( " << die.get_x1() << " " << die.get_y1() << " ) ( " << die.get_x2() << " " << die.get_y2() << " ) ;\n\n";

	if (component_list.size() != 0) {
		out << "COMPONENTS " << component_list.size() << " ;\n";
		for (auto& component : component_list) {
			out << "- " << component.get_inst_name() << " " << component.get_lib_name() << "\n";
			out << "  + PLACED ( " << component.get_x() << " " << component.get_y() << " ) N ;\n";
		}
		out << "END COMPONENTS\n\n";
	}
	if (specialnet_list.size() != 0) {
		out << "SPECIALNETS " << specialnet_list.size() << " ;\n";
		for (int i = 0; i < specialnet_list.size(); i++) {
			if (specialnet_list[i].get_layer() == "ME3") {
				string name = specialnet_list[i].get_inst_name();
				int x = (specialnet_list[i].get_x1() + specialnet_list[i].get_x2()) / 2;
				int y1 = specialnet_list[i].get_y1();
				int y2 = specialnet_list[i].get_y2();
				out << "- " << name << "\n";
				out << "  + ROUTED ME3 440 ( " << x <<" "<< y1 << ") ( * "<< y2<<" ) ;\n";
			}
			else if (specialnet_list[i].get_layer() == "ME4") {
				string name = specialnet_list[i].get_inst_name();
				int x1 = specialnet_list[i].get_x1();
				int x2 = specialnet_list[i].get_x2();
				int y = (specialnet_list[i].get_y1() + specialnet_list[i].get_y2()) / 2;
				out << "- " << name << "\n";
				out << "  + ROUTED ME4 1000 ( " << x1 << y << ") (" << x2 << " * ) ;\n";
			}
			else {
				out << "[Error] Wrong layer definition of " << i << "-th specialnet.\n";
			}
		}out << "END SPECIALNETS\n\n";
		out << "END DESIGN\n";
	}
}
int main() {
	int k = 100;
	int n = sqrt(k * 4);
	int NumOfM3 = sqrt(k);
	int NumOfM4 = sqrt(k) / 2;

	/*---------step1 create die boundary--------*/
	string design_name = "CS_APR";
	int die_x1 = 0;
	int die_y1 = 0;
	int die_x2 = CS_WIDTH * n + M3_SPACING * ((NumOfM3 + 1) * n - 1) + M3_WIDTH * NumOfM3 * n;
	int	die_y2 = CS_HEIGHT * n + M4_SPACING * ((NumOfM4 + 1) * n - 1) + M4_WIDTH * NumOfM4 * n;
	Die die(design_name, die_x1, die_y1, die_x2, die_y2);
	/*------------step2 create CS array---------------*/
	vector<vector<Component>> cs_array(n, vector<Component>(n));
	int off_y = (M4_SPACING + M4_WIDTH) * NumOfM4;
	int Dy = CS_HEIGHT + M4_SPACING * (NumOfM4 + 1) + M4_WIDTH * (NumOfM4) * 1;
	int	Dx = CS_WIDTH + M3_SPACING * (NumOfM3 + 1) + M3_WIDTH * (NumOfM3);
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			string cs_lib_name = CS_LIB_NAME;
			string cs_instance_name = "Transistor" + to_string(i * n + j);
			int x = i * Dx;
			int	y = j * Dy + off_y;
			cs_array[i][j] = Component(cs_lib_name, cs_instance_name, x, y);
		}
	}
	/*------------Step 3: create vertical ME3------------------*/
	vector<vector<SpecialNet>> ME3_specialnet(n, vector<SpecialNet>(n / 2));
	Dx = CS_WIDTH + M3_SPACING;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n / 2; j++) {
			string inst_name = "Metal3_" + to_string(i * (int(n / 2)) + j);
			string layer = "ME3";
			int	x1 = cs_array[i][0].get_x() + Dx + j * (M3_SPACING + M3_WIDTH);
			int	x2 = x1 + M3_WIDTH;
			int y1 = 0;
			int y2 = die_y2;
			ME3_specialnet[i][j] = SpecialNet(inst_name, layer, x1, y1, x2, y2);
		}
	}
	/*------Step 4: create ME4 drain -------*/
	vector<vector<SpecialNet>> ME4_specialnet_drain(n, vector<SpecialNet>(n));
	for (int i = 0; i < n / 2; i++) {
		for (int j = 0; j < n / 2; j++) {
			string layer = "ME4";
			// left bottom corner units
			string inst_name = "Metal4_drain_" + to_string(i * (int(n / 2)) + j + 0 * k);
			int x1 = cs_array[i][j].get_x() + CS_X1_TO_DRAIN;
			int x2 = ME3_specialnet[i][j].get_x2();
			int y1 = cs_array[i][j].get_y() + CS_Y1_TO_DRAIN;
			int	y2 = y1 + M4_WIDTH;
			ME4_specialnet_drain[i][j] = SpecialNet(inst_name, layer, x1, y1, x2, y2);
			// right bottom corner units
			inst_name = "Metal4_drain_" + to_string(i * (int(n / 2)) + j + 1 * k);
			x1 = cs_array[n - 1 - i][j].get_x() + CS_X1_TO_DRAIN;
			x2 = ME3_specialnet[n - 1 - i][j].get_x2();
			y1 = cs_array[n - 1 - i][j].get_y() + CS_Y1_TO_DRAIN;
			y2 = y1 + M4_WIDTH;
			ME4_specialnet_drain[n - 1 - i][j] = SpecialNet(inst_name, layer, x1, y1, x2, y2);
			// left top corner units
			inst_name = "Metal4_drain_" + to_string(i * (int(n / 2)) + j + 2 * k);
			x1 = cs_array[i][n - 1 - j].get_x() + CS_X1_TO_DRAIN;
			x2 = ME3_specialnet[i][j].get_x2();
			y1 = cs_array[i][n - 1 - j].get_y() + CS_Y1_TO_DRAIN;
			y2 = y1 + M4_WIDTH;
			ME4_specialnet_drain[i][n - 1 - j] = SpecialNet(inst_name, layer, x1, y1, x2, y2);
			// right top corner units
			inst_name = "Metal4_drain_" + to_string(i * (int(n / 2)) + j + 3 * k);
			x1 = cs_array[n - 1 - i][n - 1 - j].get_x() + CS_X1_TO_DRAIN;
			x2 = ME3_specialnet[n - 1 - i][j].get_x2();
			y1 = cs_array[n - 1 - i][n - 1 - j].get_y() + CS_Y1_TO_DRAIN;
			y2 = y1 + M4_WIDTH;
			ME4_specialnet_drain[n - 1 - i][n - 1 - j] = SpecialNet(inst_name, layer, x1, y1, x2, y2);
		}
	}
	/*----------------Step 5: create ME4 port---------------*/
	vector<SpecialNet> ME4_specialnet_port(n * NumOfM4);
	for (int i = 0; i < n; i++) {
		int x1 = 0;
		int x2 = die_x2;
		int y1 = i * Dy;
		for (int j = 0; j < NumOfM4; j++) {
			string inst_name = "Metal4_port_" + to_string(i * NumOfM4 + j);
			string layer = "ME4";
			if (j == 0) y1 = y1;
			else y1 = y1 + (M4_SPACING + M4_WIDTH);
			int y2 = y1 + M4_WIDTH;
			ME4_specialnet_port[i * NumOfM4 + j] = SpecialNet(inst_name, layer, x1, y1, x2, y2);
		}
	}
	/*-------------Step 6: create Via34 from ME4 drain ----------------*/
	vector<vector<Component>> Via34_drain2ME3(n, vector < Component>(n));
	for (int i = 0; i < n / 2; i++) {
		for (int j = 0; j < n / 2; j++) {
			string lib_name = VIA34_LIB_NAME;
			// left bottom corner units
			string inst_name = "Via34_drain2ME3_" + to_string(i * (int(n / 2)) + j + 0 * k);
			int x = ME3_specialnet[i][j].get_x1();
			int y = cs_array[i][j].get_y() + CS_Y1_TO_DRAIN;
			Via34_drain2ME3[i][j] = Component(lib_name, inst_name, x, y);
			// right bottom corner units
			inst_name = "Via34_drain2ME3_" + to_string(i * (int(n / 2)) + j + 1 * k);
			x = ME3_specialnet[n - 1 - i][j].get_x1();
			y = cs_array[n - 1 - i][j].get_y() + CS_Y1_TO_DRAIN;
			Via34_drain2ME3[n - 1 - i][j] = Component(lib_name, inst_name, x, y);
			//left top corner units
			inst_name = "Via34_drain2ME3_" + to_string(i * (int(n / 2)) + j + 2 * k);
			x = ME3_specialnet[i][j].get_x1();
			y = cs_array[i][n - 1 - j].get_y() + CS_Y1_TO_DRAIN;
			Via34_drain2ME3[i][n - 1 - j] = Component(lib_name, inst_name, x, y);
			// right top corner units
			inst_name = "Via34_drain2ME3_" + to_string(i * (int(n / 2)) + j + 3 * k);
			x = ME3_specialnet[n - 1 - i][j].get_x1();
			y = cs_array[n - 1 - i][n - 1 - j].get_y() + CS_Y1_TO_DRAIN;
			Via34_drain2ME3[n - 1 - i][n - 1 - j] = Component(lib_name, inst_name, x, y);
		}
	}
	/*##### Step 7: create Via34 to ME4 port #####*/
	vector<vector<Component>> Via34_port2ME3(n, vector <Component>(n / 2));
	for (int i = 0; i < n / 2; i++) {
		for (int j = 0; j < n / 2; j++) {
			string lib_name = VIA34_LIB_NAME;
			// left units
			string inst_name = "Via34_port2ME3_" + to_string(i * (int(n / 2)) + j);
			int x = ME3_specialnet[i][j].get_x1();
			int y = ME4_specialnet_port[i * NumOfM3 + j].get_y1();

			Via34_port2ME3[i][j] = Component(lib_name, inst_name, x, y);
			// right units
			inst_name = "Via34_port2ME3_" + to_string(i * (int(n / 2)) + j + k);
			x = ME3_specialnet[n - 1 - i][j].get_x1();
			y = y;
			Via34_port2ME3[n - 1 - i][j] = Component(lib_name, inst_name, x, y);
		}
	}
	vector<Component> component_list;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			component_list.push_back(cs_array[i][j]);
		}
	}

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n/2; j++) {
			component_list.push_back(Via34_port2ME3[i][j]);
		}
	}
	vector<SpecialNet> specialnet_list;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n / 2; j++) {
			specialnet_list.push_back(ME3_specialnet[i][j]);
		}
	}
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			specialnet_list.push_back(ME4_specialnet_drain[i][j]);
		}
	}
	for (int i = 0; i < n * NumOfM4; i++) {
		specialnet_list.push_back(ME4_specialnet_port[i]);
	}
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			component_list.push_back(Via34_drain2ME3[i][j]);
		}
	}
	string file_name = "CS_" + to_string(100) + ".def";
	write_def(file_name, die, component_list, specialnet_list,design_name);
}

