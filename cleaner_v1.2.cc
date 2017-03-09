#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stack>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <sys/time.h>

using namespace std;
int numx, numy;
int TotalThreads = 2;
mutex locker;

unordered_set<int> skips;

inline void printRoom(vector<vector<int> > &room, string &value) {
  for(int i = 0; i< room.size(); ++i) {
    for (int j = 0; j< room[i].size(); ++j) {
      value += to_string(room[i][j]);
    }
  }
}

int cleanRoom(vector<vector<int> > &room, bool clean, int start, int dir, int &last) {
  int x = (start >> 16), y = (start & 0xFFFF);
  // cout << x << ":" << y << ", dir=" << dir << endl;
  // printRoom(room);
  int delx = 0, dely = 0;
  if (dir == 1) delx = 1;  //d
  else if (dir == 2) dely = 1;   //r
  else if (dir == 3) delx = -1;  //u
  else if (dir == 4) dely = -1;  //l

  int num = 0;
  if (clean) {
    while(true) {
      x += delx, y+=dely;
      if (x < 0 || x >= numx || y < 0 || y >= numy) break;
      if (room[x][y] != 0) break;
      last = ((x << 16) | y);
      room[x][y] = 2;
      num += 1;
    }
  } else {
    while (true) {
      x += delx, y+=dely;
      room[x][y] = 0;
      num += 1;
        if (x== (last>>16) && y==(last & 0xFFFF)) break;
    }
  }
  return num;
}

int connectable(vector<vector<int> > room, int invalid, int x, int y) {
  // cout << "check current node:" << x << ":" << y << endl;
  int entrys = 0, rounds = 0;
  for (int i = 0; i < numx; ++i) {
    for (int j = 0; j < numy; ++j) {
      if (room[i][j] != 0) continue;
      if (j==y && (x==i-1 || x==i+1)) continue;
      if (i==x && (y==j-1 || y==j+1)) continue;
      rounds = 0;
      if (i <= 0 || room[i-1][j] != 0) ++rounds;
      if (j <= 0 || room[i][j-1] != 0) ++rounds;
      if (i >= numx-1 || room[i+1][j] != 0) ++rounds;
      if (j >= numy-1 || room[i][j+1] != 0) ++rounds;
      if (rounds >= 3) ++entrys;
      if (entrys >= 2) return false;
    }
  }
  // start from x,y to check if the room is connected
  stack<int> pos, changed;
  int sx = x, sy = y;
  if (x>=1 && room[x-1][y] == 0) --sx;
  else if (y>=1 && room[x][y-1] == 0) --sy;
  else if (x < numx-1 && room[x+1][y] == 0) ++sx;
  else if (y < numy-1 && room[x][y+1] == 0) ++sy;
  else return false;

  pos.push((sx<< 16) | sy);
  int total = 1, value = 10;
  room[sx][sy] = value;
  while (pos.size()) {
    int curpos = pos.top();
    pos.pop();
    int a = (curpos >> 16), b = (curpos & 0xFFFF);
    if (a-1>=0 && room[a-1][b] == 0) {
      room[a-1][b] = value;
      pos.push(((a-1)<<16) | b);
      ++total;
    }
    if (a+1 < numx && room[a+1][b] == 0) {
      room[a+1][b] = value;
      pos.push(((a+1)<< 16) | b);
      ++total;
    }
    if (b -1 >=0 && room[a][b-1]==0) {
      room[a][b-1] = value;
      pos.push((a<<16)| (b-1));
      ++total;
    }
    if (b +1 < numy && room[a][b+1] == 0) {
      room[a][b +1] = value;
      pos.push((a<<16) | (b+1));
      ++total;
    }
  }
  return total == invalid;
}

bool trySearchRoom(vector<vector<int> >room, int invalid, int x, int y, string &results) {
  stack<int> paths;
  stack<int> dirs;  // 1,2,3,4

  // cout << "==================\ntry search room: " << x << ":" << y << endl;
  int curpos = ((x<<16)|y);
  int last_dir;

  paths.push(curpos); dirs.push(1);
  invalid --; room[x][y] = 2;
  if (connectable(room, invalid, x, y) == false) {
    return false;
  }

  bool check = true;
  while (dirs.size()) {
    if (invalid == 0) break;  // all invalid unions has been cleaned.
    last_dir = dirs.top();
    int now = paths.top();
    if (check && last_dir != 5 && 
        connectable(room, invalid, now>>16, now & 0xFFFF) == false) {
      last_dir = 5;
    }
    if (last_dir <= 4) {  // move forward
      int newpos;
      int num = cleanRoom(room, true, paths.top(), last_dir, newpos);
      if (num == 0) {  // invalid directions
        dirs.pop(); 
        if (dirs.size() && dirs.top() == last_dir + 1) {
          dirs.push(last_dir+2);
        } else {
          dirs.push(last_dir + 1);
        }
        check = false;
      } else {  // valid directions
        paths.push(newpos); invalid -= num;
        if (dirs.top() == 1) dirs.push(2); else dirs.push(1);
        check = true;
      }
    } else {  // current pos should not be used;
      if (dirs.size() == 1) return false;  // only one direction, and tryed all failed.
      curpos = paths.top(); paths.pop();
      dirs.pop();
      //insertInvalid(room, curpos, dirs.top(), dirs.size());
      int num = cleanRoom(room, false, paths.top(), dirs.top(), curpos);
      invalid += num;
      last_dir = dirs.top(); dirs.pop();
      if (dirs.size() && dirs.top() == last_dir + 1) dirs.push(last_dir + 2);
      else dirs.push(last_dir+1);
      check = false;
    }
  }

  if (!invalid) {
    results = "";
    dirs.pop();
    while (dirs.size()) {
      char s = 'd';
      if (dirs.top() == 2) s = 'r';
      if (dirs.top() == 3) s = 'u';
      if (dirs.top() == 4) s = 'l';
      results = s + results;
      dirs.pop();
    }
    return true;
  }
  return false;
  
}

void thread_run(vector<vector<int> > room, int invalid, int idx) {
  int total = 0;
  string value;
  for (int i = 0; i < numx; ++i) {
    for (int j = 0; j < numy; ++j) {
      if (room[i][j] != 0) continue;
      //if (skips.count((i<<16)|j)) continue;
      total += 1;
      if (total % TotalThreads == idx) {
        if (trySearchRoom(room, invalid, i, j, value)) {
          locker.lock();
          cout << value << endl;
          cout << i+1 << ":"  << j+1 << endl;
          exit(0);
        }
      }
    }
  }
}

inline int getWrappers(vector<vector<int> > &room, int x, int y) {
  int round = 0;
  if (room[x][y] == 1) return 4;
  if (x <= 0 || room[x-1][y] == 1) ++round;
  if (x >= numx - 1 || room[x+1][y] == 1) ++round;
  if (y <= 0 || room[x][y-1] == 1) ++round;
  if (y >= numy - 1 || room[x][y+1] == 1) ++round;
  return round;
}

void checkSkipPoints(vector<vector<int> > room) {
  int valid = 0;
  for (int i = 0; i < numx; ++i) {
    for (int j = 0; j < numy; ++j) {
      string key = "";
      if (skips.count((i<<16)|j) >= 1) continue;
      ++valid;
      if (getWrappers(room, i, j) == 2) {
        stack<int> pts; pts.push((i<<16)|j);
        while (pts.size()) {
          int cur = pts.top(); pts.pop();
          int x = cur>>16, y = cur & 0xFFFF;
          if (x > 0 && getWrappers(room, x-1, y) == 2 && (x-1 != i || y != j)) {
            int key = ((x-1)<<16) | y;
            if (skips.count(key) == 0) {
              pts.push(key); skips.insert(key);
            }
          }
          if (x < numx -1 && getWrappers(room, x+1, y) == 2 && (x+1 != i || y != j)) {
            int key = ((x+1)<<16) | y;
            if (skips.count(key) == 0) {
              pts.push(key); skips.insert(key);
            }
          }
          if (y > 0 && getWrappers(room, x, y-1) == 2 && (x != i || y-1 != j)) {
            int key = ((x)<<16) | (y-1);
            if (skips.count(key) == 0) {
              pts.push(key); skips.insert(key);
            }
          }
          if (y < numy-1 && getWrappers(room, x, y+1)==2 && (x != i || y+1 != j)) {
            int key = ((x)<<16) | (y+1);
            if (skips.count(key) == 0) {
              pts.push(key); skips.insert(key);
            }
          }
        }
      }
    }
  }
  for (auto itr = skips.begin(); itr != skips.end(); ++itr) {
    cout << ((*itr)>>16) << ":" << ((*itr) & 0xFFFF) << endl;
  }
}

int main(int argc, char**argv) {
  string filename = argv[1];
  TotalThreads = atoi(argv[2]);
  ifstream fin(filename.c_str());
  string s;
  int invalid = 0;
  vector<vector<int> > room;
  while (getline(fin, s)) {
    vector<int> curline;
    for (int i = 0; i < s.length(); ++i) {
      if (s[i] == '0') {
        curline.push_back(0);
        invalid ++;
      }
      if (s[i] == '1') curline.push_back(1);
    }
    room.push_back(curline);
  }
  numx = room.size();
  numy = room[0].size();

  //checkSkipPoints(room);
  thread *thrds = new thread[TotalThreads];
  for (int k = 0; k < TotalThreads; ++k) {
    thrds[k] = thread(thread_run, room, invalid, k);
  }
  for (int k =0; k < TotalThreads; ++k) {
    thrds[k].join();
  }
  cout << "done, no success" << endl;
  return -1;
}
