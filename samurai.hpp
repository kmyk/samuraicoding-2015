/**
 * @file samurai.cpp
 * @author Kimiyuki Onaka
 * @date Tue. 05, 2016
 */
#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <set>
#include <map>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <cstdio>
#include <cassert>
#define repeat(i,n) for (int i = 0; (i) < (n); ++(i))
#define repeat_from(i,m,n) for (int i = (m); (i) < (n); ++(i))
#define repeat_reverse(i,n) for (int i = (n)-1; (i) >= 0; --(i))
#define repeat_from_reverse(i,m,n) for (int i = (n)-1; (i) >= (m); --(i))
#define dump(x)  cerr << #x << " = " << (x) << endl
#define debug(x) cerr << #x << " = " << (x) << " (L" << __LINE__ << ")" << " " << __FILE__ << endl
typedef long long ll;

int getint(std::istream & in); // without comment

struct point_t {
    int y, x;
};
std::istream & operator >> (std::istream & in, point_t & p);
bool operator == (point_t const & a, point_t const & b);
bool operator != (point_t const & a, point_t const & b);
point_t & operator += (point_t & a, point_t const & b);
point_t & operator -= (point_t & a, point_t const & b);
point_t operator + (point_t const & a, point_t const & b);
point_t operator - (point_t const & a, point_t const & b);
const point_t direction[] = { {1,0}, {0,1}, {-1,0}, {0,-1}, {0,0} };
point_t rotdeg(point_t const & a, int degree);
point_t rotdir(point_t const & a, int direction);
const int DIRECTION_NUM = 4;

const int SAMURAI_NUM = 6;
const int FRIEND_NUM = 3;
const int ENEMY_NUM = 3;

struct game_info_t {
    int turns;
    int side;
    int weapon;
    int width;
    int height;
    int cure;
    point_t home[6];
    int rank[6];
    int score[6];
};
std::istream & operator >> (std::istream & in, game_info_t & ginfo);

// field state
const int F_OCCUPIED = 0;
const int F_FREE = 8;
const int F_UNKNOWN = 9;
bool is_field_occupied(int f);
bool is_field_friend(int f);
bool is_field_enemy(int f);
bool is_on_field(point_t const & p, game_info_t const & ginfo); 

struct turn_info_t {
    int turn;
    int cure;
    point_t pos[6];
    int state[6];
    std::vector<std::vector<int> > field;
};
turn_info_t getturninfo(std::istream & in, game_info_t & ginfo);

// direction
const int D_SOUTH = 0;
const int D_EAST  = 1;
const int D_NORTH = 2;
const int D_WEST  = 3;

// action
const int A_ATTACK = 1; // + direction
const int A_MOVE   = 5; // + direction
const int A_APPEAR = 9;
const int A_HIDE  = 10;
const int ACTION_COST[] = { -1,  4, 4, 4, 4,  2, 2, 2, 2,  1, 1 };
bool is_action_attack(int a);
bool is_action_move(int a);

const point_t ATTACK_AREA_SPEAR[] = { {1,0}, {2,0}, {3,0}, {4,0} };
const point_t ATTACK_AREA_SWORD[] = { {0,2}, {0,1}, {1,1}, {1,0}, {2,0} };
const point_t ATTACK_AREA_AXE[] = { {-1,1}, {0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1} };
const point_t * const ATTACK_AREA[] = { ATTACK_AREA_SPEAR, ATTACK_AREA_SWORD, ATTACK_AREA_AXE };
const int ATTACK_AREA_NUM[] = { 4, 5, 7 };

// state
const int S_APPEARED = 0;
const int S_HIDDEN = 1;
const int S_ELIMINATED = -1;

struct action_plan_t {
    std::vector<int> a;
};
std::ostream & operator << (std::ostream & out, action_plan_t & plan);
int total_cost(action_plan_t const & plan);
point_t total_move(action_plan_t const & plan);
bool is_valid_plan(action_plan_t const & plan, game_info_t const & ginfo, turn_info_t const & tinfo);


void debug_print(point_t const & p, std::vector<std::vector<int> > const & field, game_info_t const & ginfo, turn_info_t const & tinfo);
