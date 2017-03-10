#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stack>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
using namespace std;
int numx, numy;
int TotalThreads = 2;
mutex locker;

class RoomState {
 public:
  vector<vector<int> > room;
  vector<vector<int> > wall;
  int invalid = 0, ends = 0;
  bool adj = false;
};

inline void printRoom(vector<vector<int> > &room, string &value) {
  value = "";
  for(int i = 0; i< room.size(); ++i) {
    for (int j = 0; j< room[i].size(); ++j) {
      value += to_string(room[i][j]);
    }
    value += "\n";
  }
}

inline int cleanRoom(RoomState &st, bool clean, int start, int dir, int &last) {
  int x = (start >> 16), y = (start & 0xFFFF);
  int delx = 0, dely = 0;
  int vx = 0, vy = 1;
  if (dir == 1) delx = 1;  //d
  else if (dir == 2) dely = 1, vx=1, vy=0;   //r
  else if (dir == 3) delx = -1;  //u
  else if (dir == 4) dely = -1, vx=1, vy=0;  //l

  int num = 0;
  if (clean) {
    while (true) {
      x += delx, y+=dely;
      if (x < 0 || x >= numx || y < 0 || y >= numy || st.room[x][y] != 0) break;
      if (st.wall[x][y] == 3) --st.ends;
      int ax = x+vx, ay = y+vy;
      st.adj=0;
      if (ax<numx && ay < numy && st.room[ax][ay]==0) {
        ++st.wall[ax][ay];
        if (st.wall[ax][ay] == 3) ++st.ends;
        if (st.wall[ax][ay] >= 3) st.adj=1;
      }
      ax = x - vx, ay = y - vy;
      if (ax >= 0 && ay >= 0 && st.room[ax][ay]==0) {
        ++st.wall[ax][ay];
        if (st.wall[ax][ay] == 3) ++st.ends;
        if (st.wall[ax][ay] >= 3) st.adj=1;
      }
      st.wall[x][y] = -1, st.room[x][y] = 2, --st.invalid;
      last = ((x << 16) | y);
      ++num;
    }
  } else {
    x = (last>>16), y =(last & 0xFFFF);
    bool first = false, end = true;
    while (!first) {
      if (x-delx== (start>>16) && y-dely==(start & 0xFFFF)) first = true;
      int ax = x + vx, ay = y + vy;
      st.wall[x][y] = 2-first-end, st.room[x][y]=0, ++st.invalid, st.adj=0;
      if (ax<numx && ay < numy && st.room[ax][ay]==0) {
        --st.wall[ax][ay];
        if (st.wall[ax][ay] == 2) --st.ends;
        else if (st.wall[ax][ay] == 3) st.adj = 1;
      } else ++st.wall[x][y];
      ax = x - vx, ay = y - vy;
      if (ax>=0 && ay>=0 && st.room[ax][ay]==0) {
        --st.wall[ax][ay];
        if (st.wall[ax][ay] == 2) --st.ends;
        else if (st.wall[ax][ay] == 3) st.adj = 1;
      } else ++st.wall[x][y];
      if (st.wall[x][y]>=3) ++st.ends, st.adj=1;
      x -= delx, y -= dely, end = false;
      ++num;
    }
  }
  return num;
}

int connectable(RoomState &st, int x, int y) {
  if (st.ends > st.adj + 1) return false;
  // start from x,y to check if the room is connected
  vector<vector<int> > room = st.room;
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
  return total == st.invalid;
}

bool trySearchRoom(RoomState st, int x, int y, string &results) {
  stack<int> paths;
  stack<int> dirs;  // 1,2,3,4

  // cout << "==================\ntry search room: " << x << ":" << y << endl;
  int curpos = ((x<<16)|y);
  int last_dir;

  paths.push(curpos); dirs.push(1);
  jamPosition(st, x , y);

  if (connectable(st, x, y) == false) {
    return false;
  }

  bool check = true;
  while (dirs.size()) {
    if (st.invalid == 0) break;  // all invalid unions has been cleaned.
    last_dir = dirs.top();
    int now = paths.top();
    if (check && last_dir != 5 && 
        connectable(st, now>>16, now & 0xFFFF) == false) {
      last_dir = 5;
    }
    if (last_dir <= 4) {  // move forward
      int newpos;
      int num = cleanRoomV2(st, true, paths.top(), last_dir, newpos);
      if (num == 0) {  // invalid directions
        dirs.pop(); 
        if (dirs.size() && dirs.top() == last_dir + 1) {
          dirs.push(last_dir+2);
        } else {
          dirs.push(last_dir + 1);
        }
        check = false;
      } else {  // valid directions
        paths.push(newpos); 
        if (dirs.top() == 1) dirs.push(2); else dirs.push(1);
        check = true;
      }
    } else {  // current pos should not be used;
      if (dirs.size() == 1) return false;  // only one direction, and tryed all failed.
      curpos = paths.top(); paths.pop();
      dirs.pop();
      int num = cleanRoomV2(st, false, paths.top(), dirs.top(), curpos);
      last_dir = dirs.top(); dirs.pop();
      if (dirs.size() && dirs.top() == last_dir + 1) dirs.push(last_dir + 2);
      else dirs.push(last_dir+1);
      check = false;
    }
  }

  if (!st.invalid) {
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

void thread_run(RoomState st, int idx) {
  int total = 0;
  string value;
  for (int i = 0; i < numx; ++i) {
    for (int j = 0; j < numy; ++j) {
      if (st.room[i][j] != 0) continue;
      total += 1;
      if (total < 140) continue;
      if (total % 5 == 0) cout << total << endl;
      if (total % TotalThreads == idx) {
        if (trySearchRoom(st, i, j, value)) {
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
  if (room[x][y] != 0) return -1;
  if (x <= 0 || room[x-1][y] != 0) ++round;
  if (x >= numx - 1 || room[x+1][y] != 0) ++round;
  if (y <= 0 || room[x][y-1] != 0) ++round;
  if (y >= numy - 1 || room[x][y+1] != 0) ++round;
  return round;
}

int main(int argc, char**argv) {
  string filename = argv[1];
  TotalThreads = atoi(argv[2]);
  ifstream fin(filename.c_str());
  string s;
  RoomState st;
  vector<vector<int> > &room = st.room;
  while (getline(fin, s)) {
    vector<int> curline;
    for (int i = 0; i < s.length(); ++i) {
      if (s[i] == '0') {
        curline.push_back(0);
        st.invalid ++;
      }
      if (s[i] == '1') curline.push_back(1);
    }
    room.push_back(curline);
  }
  numx = room.size();
  numy = room[0].size();
  st.wall = st.room;
  for (int i = 0; i < numx; ++i) {
    for (int j = 0; j < numy; ++j) {
      st.wall[i][j] = getWrappers(st.room, i ,j);
    }
  }

  //checkSkipPoints(room);
  thread *thrds = new thread[TotalThreads];
  for (int k = 0; k < TotalThreads; ++k) {
    thrds[k] = thread(thread_run, st, k);
  }
  for (int k =0; k < TotalThreads; ++k) {
    thrds[k].join();
  }
  cout << "done, no success" << endl;
  return -1;
}
