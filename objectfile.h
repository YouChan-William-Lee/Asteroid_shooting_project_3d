#pragma once
#include "Unifiedheader.h"

// vec2 float
class C_Point_2f {
public:
	vector <float> d{ 0, 0 };
};

// vec3 float
class C_Point_3f {
public:
	vector <float> d{ 0, 0, 0 };
};

// vec3 int
class C_Point_3i {
public:
	vector <int> d{ 0, 0, 0 };
};

// faces
class C_Face {
public:
	vector <C_Point_3i> v_pairs;
};

// object
class C_Obj {
public:
	string name;
	vector <C_Point_3f> v;
	vector <C_Point_2f> vt;
	vector <C_Point_3f> vn;
	vector <C_Face> f;
};

// model
class C_Model {
public:
	vector <C_Obj> objs;

	// float tokens
	vector<float> my_strtok_f(char* str, char* delimeter) {
		vector <float> v;
		char* context;
		char* tok = strtok_s(str, delimeter, &context);

		while (tok != NULL) {
			v.push_back(atof(tok));
			tok = strtok_s(context, delimeter, &context);
		}
		return v;
	}

	// string tokens
	vector<string> my_strtok_s(char* str, char* delimeter) {
		vector <string> v;
		char* context;
		char* tok = strtok_s(str, delimeter, &context);

		while (tok != NULL) {
			v.push_back(tok);
			tok = strtok_s(context, delimeter, &context);
		}
		return v;
	}

	// int tokens
	vector<int> my_strtok_i(char* str, char* delimeter) {
		vector <int> v;
		char* context;
		char* tok = strtok_s(str, delimeter, &context);

		while (tok != NULL) {
			v.push_back(atoi(tok));
			tok = strtok_s(context, delimeter, &context);
		}
		return v;
	}

	// load the .obj file
	void loadOjb(ifstream& file) {
		string line;
		C_Obj obj_tmp;
		int cnt = 0;

		while (getline(file, line)) {
			int len = line.length();
			vector<float> vf;
			vector<string> s;
			vector<int> vi;
			C_Point_3f p3;
			C_Point_2f p2;
			C_Point_3i p3i;

			// check 'o '
			if (line[0] == 'o' && line[1] == ' ') {
				obj_tmp.name = line.substr(2, len - 2);
				objs.push_back(obj_tmp);
				cnt += 1;
			}

			// check 'v '
			if (line[0] == 'v' && line[1] == ' ') {
				vf = my_strtok_f((char*)line.substr(2, len - 2).c_str(), (char*)" ");
				p3.d = { vf[0], vf[1], vf[2] };
				objs[cnt - 1].v.push_back(p3);
			}
			// check 'vt'
			else if (line[0] == 'v' && line[1] == 't') {
				vf = my_strtok_f((char*)line.substr(3, len - 3).c_str(), (char*)" ");
				p2.d = { vf[0], vf[1] };
				objs[cnt - 1].vt.push_back(p2);
			}
			// check 'vn'
			else if (line[0] == 'v' && line[1] == 'n') {
				vf = my_strtok_f((char*)line.substr(3, len - 3).c_str(), (char*)" ");
				p3.d = { vf[0], vf[1], vf[2] };
				objs[cnt - 1].vn.push_back(p3);
			}
			//check 'f '
			else if (line[0] == 'f' && line[1] == ' ') {
				s = my_strtok_s((char*)line.substr(2, len - 2).c_str(), (char*)" ");
				int nVertexes = s.size();
				C_Face face_tmp;
				for (int i = 0; i < nVertexes; i++) {
					vi = my_strtok_i((char*)s[i].c_str(), (char*)"/");
					p3i.d = { vi[0], vi[1], vi[2] };
					face_tmp.v_pairs.push_back(p3i);
				}
				objs[cnt - 1].f.push_back(face_tmp);
			}
		}
	}
};