/**
 * @file samurai.cpp
 * @author Kimiyuki Onaka
 * @date Tue. 05, 2016
 */
#include "samurai.hpp"
using namespace std;

int getint(istream & in) {
    string s;
    while (true) {
        in >> s;
        if (s == "#") {
            getline(in, s);
        } else {
            return atoi(s.c_str());
        }
    }
}

istream & operator >> (istream & in, point_t & p) {
    p.x = getint(in);
    p.y = getint(in);
    return in;
}
bool operator == (point_t const & a, point_t const & b) {
    return a.y == b.y and a.x == b.x;
}
bool operator != (point_t const & a, point_t const & b) {
    return not (a == b);
}
point_t & operator += (point_t & a, point_t const & b) {
    a.y += b.y;
    a.x += b.x;
    return a;
}
point_t & operator -= (point_t & a, point_t const & b) {
    a.y -= b.y;
    a.x -= b.x;
    return a;
}
point_t operator + (point_t const & a, point_t const & b) {
    point_t c = a;
    return c += b;
}
point_t operator - (point_t const & a, point_t const & b) {
    point_t c = a;
    return c -= b;
}
point_t rotdeg(point_t const & p, int degree) {
    switch ((degree + 360) % 360) {
        case   0: return {   p.y,   p.x };
        case  90: return { - p.x,   p.y };
        case 180: return { - p.y, - p.x };
        case 270: return {   p.x, - p.y };
        default: assert (false);
    }
}
point_t rotdir(point_t const & p, int direction) {
    assert (0 <= direction and direction < DIRECTION_NUM);
    return rotdeg(p, direction * 90);
}

istream & operator >> (istream & in, game_info_t & ginfo) {
    ginfo.turns  = getint(in);
    ginfo.side   = getint(in);
    ginfo.weapon = getint(in);
    ginfo.width  = getint(in);
    ginfo.height = getint(in);
    ginfo.cure   = getint(in);
    repeat (i,SAMURAI_NUM) {
        in >> ginfo.home[i];
    }
    repeat (i,SAMURAI_NUM) {
        ginfo.rank[i]  = getint(in);
        ginfo.score[i] = getint(in);
    }
    return in;
}

bool is_field_occupied(int f) {
    return F_OCCUPIED <= f and f < F_OCCUPIED + SAMURAI_NUM;
}
bool is_field_friend(int f) {
    return F_OCCUPIED <= f and f < F_OCCUPIED + FRIEND_NUM;
}
bool is_field_enemy(int f) {
    return F_OCCUPIED + FRIEND_NUM <= f and f < F_OCCUPIED + SAMURAI_NUM;
}
bool is_on_field(point_t const & p, game_info_t const & ginfo) {
    return 0 <= p.y and p.y < ginfo.height and 0 <= p.x and p.x < ginfo.width;
}

turn_info_t getturninfo(istream & in, game_info_t & ginfo) {
    turn_info_t tinfo;
    tinfo.turn = getint(in);
    tinfo.cure = getint(in);
    repeat (i,SAMURAI_NUM) {
        cin >> tinfo.pos[i];
        tinfo.state[i] = getint(in);
    }
    tinfo.field.resize(ginfo.height, vector<int>(ginfo.width));
    repeat (y, ginfo.height) {
        repeat (x, ginfo.width) {
            tinfo.field[y][x] = getint(in);
        }
    }
    return tinfo;
}

bool is_action_attack(int a) {
    return A_ATTACK <= a and a < A_ATTACK + DIRECTION_NUM;
}
bool is_action_move(int a) {
    return A_MOVE <= a and a < A_MOVE + DIRECTION_NUM;
}

ostream & operator << (ostream & out, action_plan_t & plan) {
    for (int a : plan.a) out << a << ' ';
    return out << 0;
}

int total_cost(action_plan_t const & plan) {
    int cost = 0;
    for (int a : plan.a) cost += ACTION_COST[a];
    if (cost > 7) return false;
    return cost;
}
point_t total_move(action_plan_t const & plan) {
    point_t p = {};
    for (int a : plan.a) {
        if (is_action_move(a)) {
            p += direction[a - A_MOVE];
        }
    }
    return p;
}

bool is_valid_plan(action_plan_t const & plan) {
    // 10 種の行動から任意のものを
    for (int a : plan.a) if (a < 1 or 10 < a) return false;
    // そのコストの計が 7 以下である範囲で任意数選んで指示することができる
    if (total_cost(plan) > 7) return false;
    return true;
}
bool is_valid_plan(action_plan_t const & plan, game_info_t const & ginfo, turn_info_t const & tinfo) {
    if (plan.a.empty()) return true;
    if (tinfo.cure) return false;
    if (not is_valid_plan(plan)) return false;
    point_t p = tinfo.pos[ginfo.weapon];
    int s = tinfo.state[ginfo.weapon];
    if (s == S_ELIMINATED) return false;
    for (int a : plan.a) {
        if (is_action_attack(a)) {
            // 隠伏している間は占領行動をできない
            if (s == S_HIDDEN) return false;
        } else if (is_action_move(a)) {
            p += direction[a - A_MOVE];
            if (not is_on_field(p, ginfo)) return false;
            if (s == S_APPEARED) {
                repeat (i,SAMURAI_NUM) if (i != ginfo.weapon) {
                    // 姿を隠していない状態で他のサムライがいる区画に移動することはできない
                    if (tinfo.state[i] == S_APPEARED and p == tinfo.pos[i]) return false;
                }
            } else {
                // 姿を隠しながら味方の領地以外の区画に移動することはできない
                if (not is_field_friend(tinfo.field[p.y][p.x])) return false;
            }
            repeat (i,SAMURAI_NUM) if (i != ginfo.weapon) {
                // 他のサムライの居館の区画には移動できない
                if (p == ginfo.home[i]) return false;
            }
        } else if (a == A_HIDE) {
            // 隠伏は味方の領地にいるときしかできない
            if (not is_field_friend(tinfo.field[p.y][p.x])) return false;
            // XXX: 隠伏中に隠伏は可能？
            if (s == S_HIDDEN) return false;
            s = S_HIDDEN;
        } else if (a == A_APPEAR) {
            repeat (i,SAMURAI_NUM) if (i != ginfo.weapon) {
                // 同じ区画に姿を隠していない他のサムライがいる場合には、顕現できない
                if (tinfo.state[i] == S_APPEARED and p == tinfo.pos[i]) return false;
            }
            // XXX: 顕現中に顕現は可能？
            if (s == S_APPEARED) return false;
            s = S_APPEARED;
        }
    }
    return true;
}

void debug_print(point_t const & p, vector<vector<int> > const & f, game_info_t const & ginfo, turn_info_t const & tinfo) {
    repeat (y,ginfo.height) {
        repeat (x,ginfo.width) {
            point_t q = { y, x };
            string s = "\x1b[";
            if (q == p) {
                s += '4';
                s += '1' + ginfo.weapon;
                s += "m@";
                goto next;
            }
            repeat (i,SAMURAI_NUM) {
                if (q == tinfo.pos[i]) {
                    s += '4';
                    s += '1' + i;
                    s += 'm';
                    s += 'A' + i;
                    goto next;
                }
            }
            s += '3';
            if (f[y][x] == F_UNKNOWN) {
                s += '0';
            } else if (f[y][x] == F_FREE) {
                s += '7';
            } else {
                s += '1' + f[y][x];
            }
            s += 'm';
            s += '0' + f[y][x];
next:;
            cerr << s << "\x1b[0m";
        }
        cerr << endl;
    }
}