#include "Config.h"
#include <cstdlib>
#include <fstream>
#include <cassert>

using namespace speechsvr;
using namespace std;

string trim(string s) {
  // trim back
  size_t pos = s.find_last_not_of(" \t\r\n");
  if (pos != string::npos)
    s = s.substr(0, pos+1);
  else
    s = "";
  // trim front
  pos = s.find_first_not_of(" \t\r\n");
  if (pos != string::npos)
    s = s.substr(pos);
  return s;
}

Config::Config(string filename) {
  ifstream ifs(filename.c_str(), ifstream::in);
  while (ifs.good()) {
    string line;
    getline(ifs, line);
    line = trim(line);
    if (line.length() > 0 && line[0] != '#') {
      size_t pos = line.find('=');
      assert(pos != string::npos);
      string key = trim(line.substr(0, pos));
      string value = trim(line.substr(pos+1));
      map_[key] = value;
    }
  }
  ifs.close();
}

Config::~Config() {}

string  Config::GetString(string key) { return map_[key]; }
int     Config::GetInt(string key)    { return atoi(map_[key].c_str()); }
float   Config::GetFloat(string key)  { return atof(map_[key].c_str()); }
