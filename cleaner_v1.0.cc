#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stack>
#include <thread>
#include <unordered_set>
#include <mutex>
using namespace std;
int numx, numy;
int TotalThreads = 2;
struct Point {
  Point() { };
  Point(int a, int b) : x(a), y(b) { }
  int x, y;
};
mutex locker;

inline void printRoom(vector<vector<int> > &room, string &value) {
  for(int i = 0; i< room.size(); ++i) {
    for (int j = 0; j< room[i].size(); ++j) {
      value += to_string(room[i][j]);
    }
  }
}

int cleanRoom(vector<vector<int> > &room, bool clean, Point start, int dir, Point &last) {
  int x = start.x, y = start.y;
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
      last = Point(x,y);
      room[x][y] = 2;
      num += 1;
    }
  } else {
    while (true) {
      x += delx, y+=dely;
      room[x][y] = 0;
      num += 1;
      if (x==last.x && y==last.y) break;
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
  stack<Point > pos, changed;
  int sx = x, sy = y;
  if (x>=1 && room[x-1][y] == 0) --sx;
  else if (y>=1 && room[x][y-1] == 0) --sy;
  else if (x < numx-1 && room[x+1][y] == 0) ++sx;
  else if (y < numy-1 && room[x][y+1] == 0) ++sy;
  else return false;

  pos.push(Point(sx,sy));
  int total = 1, value = 10;
  room[sx][sy] = value;
  while (pos.size()) {
    Point curpos = pos.top();
    pos.pop();
    int &a = curpos.x, &b = curpos.y;
    if (a-1>=0 && room[a-1][b] == 0) {
      room[a-1][b] = value;
      pos.push(Point(a-1, b));
      ++total;
    }
    if (a+1 < numx && room[a+1][b] == 0) {
      room[a+1][b] = value;
      pos.push(Point(a+1, b));
      ++total;
    }
    if (b -1 >=0 && room[a][b-1]==0) {
      room[a][b-1] = value;
      pos.push(Point(a, b-1));
      ++total;
    }
    if (b +1 < numy && room[a][b+1] == 0) {
      room[a][b +1] = value;
      pos.push(Point(a, b+1));
      ++total;
    }
  }
  return total == invalid;
}

bool trySearchRoom(vector<vector<int> >room, int invalid, int x, int y, string &results) {
  stack<Point > paths;
  stack<int> dirs;  // 1,2,3,4

  // cout << "==================\ntry search room: " << x << ":" << y << endl;
  Point curpos(x,y), lastpos;
  int last_dir;

  paths.push(curpos); dirs.push(1);
  invalid --; room[x][y] = 2;
  if (connectable(room, invalid, x, y) == false) {
    return false;
  }

  while (dirs.size()) {
    if (invalid == 0) break;  // all invalid unions has been cleaned.
    last_dir = dirs.top();
    Point now = paths.top();
    if (connectable(room, invalid, now.x, now.y) == false) {
      last_dir = 5;
    }
    if (last_dir <= 4) {  // move forward
      Point newpos;
      int num = cleanRoom(room, true, paths.top(), last_dir, newpos);
      if (num == 0) {  // invalid directions
        dirs.pop(); 
        if (dirs.size() && dirs.top() == last_dir + 1) {
          dirs.push(last_dir+2);
        } else {
          dirs.push(last_dir + 1);
        }
      } else {  // valid directions
        paths.push(newpos); invalid -= num;
        if (dirs.top() == 1) dirs.push(2); else dirs.push(1);
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
