/**
 * @file a.cpp
 * @author Kimiyuki Onaka
 * @date Tue. 05, 2016
 */
#include "samurai.hpp"
using namespace std;

class player {
    game_info_t ginfo;
    vector<turn_info_t> tinfos;
    turn_info_t tinfo;
    vector<vector<int> > field;
    vector<vector<bool> > is_dangerous;
    default_random_engine engine;

    int weapon() const { return ginfo.weapon; }
    int h() const { return ginfo.height; }
    int w() const { return ginfo.width; }
    int state() const { return tinfo.state[ginfo.weapon]; }
    point_t pos() const { return tinfo.pos[ginfo.weapon]; }

private:
    void update();
    int evaluate(action_plan_t const & plan);

public:
    explicit player(game_info_t const & ginfo);
    action_plan_t play(turn_info_t const & tinfo);
};

player::player(game_info_t const & ginfo) : ginfo(ginfo) {
    random_device device;
    engine.seed(device());

    tinfo = {};
    tinfo.turn = -1;
    field.resize(h(), vector<int>(w(), F_UNKNOWN));
}

void player::update() {
    repeat (y,h()) {
        repeat (x,w()) {
            if (tinfo.field[y][x] == F_UNKNOWN) continue;
            field[y][x] = tinfo.field[y][x];
        }
    }

    is_dangerous.clear();
    is_dangerous.resize(h(), vector<bool>(w()));
    repeat (i,ENEMY_NUM) {
        if (tinfo.state[i] == S_ELIMINATED) continue;
        point_t p = tinfo.pos[FRIEND_NUM + i];
        if (not is_on_field(p, ginfo)) continue;
        repeat (j,DIRECTION_NUM + 1) {
            repeat (k,DIRECTION_NUM) {
                repeat (l,ATTACK_AREA_NUM[i]) {
                    point_t q = p + direction[j] + rotdir(ATTACK_AREA[i][l], k);
                    if (not is_on_field(q, ginfo)) continue;
                    is_dangerous[q.y][q.x] = true;
                }
            }
        }
    }
}

action_plan_t player::play(turn_info_t const & a_tinfo) {
    if (tinfo.turn != -1) tinfos.push_back(tinfo);
    tinfo = a_tinfo;
    update();

    debug_print(pos(), field, ginfo, tinfo);

    action_plan_t plan;
    if (tinfo.cure) return plan;
    if (state() == S_ELIMINATED) return plan;

    // greedy
    int highscore = 0;
    action_plan_t t;
    if (state() == S_HIDDEN) {
        t.a.push_back(A_APPEAR);
    }
    repeat (i,DIRECTION_NUM + 1) {
        if (i < DIRECTION_NUM) t.a.push_back(A_MOVE + i);
        if (is_valid_plan(t, ginfo, tinfo)) {
            repeat (j,DIRECTION_NUM) {
                t.a.push_back(A_ATTACK + j);
                int score = evaluate(t);
                if (highscore < score) {
                    highscore = score;
                    plan = t;
                }
                t.a.pop_back();
            }
        }
        if (i < DIRECTION_NUM) t.a.pop_back();
    }
    if (total_cost(plan) < 7 and true) { // TODO: check whether it's possible or not
        plan.a.push_back(A_HIDE);
    }
    return plan;
}

int player::evaluate(action_plan_t const & plan) {
    if (not is_valid_plan(plan, ginfo, tinfo)) return -1;
    int score = 0;
    vector<vector<int> > f = field;
    point_t p = pos();
    for (int a : plan.a) {
        if (is_action_attack(a)) {
            repeat (i, ATTACK_AREA_NUM[weapon()]) {
                point_t q = p + rotdir(ATTACK_AREA[weapon()][i], a - A_ATTACK);
                if (not is_on_field(q, ginfo)) continue;
                repeat (j,ENEMY_NUM) {
                    if (q == tinfo.pos[FRIEND_NUM + j]) {
                        if (q == ginfo.home[FRIEND_NUM + j]) {
                            score += 80;
                        } else {
                            score += 2000;
                        }
                    }
                }
                int fq = f[q.y][q.x];
                if (is_field_enemy(fq)) {
                    score += 100;
                } else if (fq == F_FREE or fq == F_UNKNOWN) {
                    score += 90;
                } if (is_field_friend(fq) and fq == F_OCCUPIED + weapon()) {
                    score += 1;
                }
                f[q.y][q.x] = F_OCCUPIED + weapon();
            }
        } else if (is_action_move(a)) {
            p += direction[a - A_MOVE];
        }
    }
    if (is_dangerous[p.y][p.x]) {
        score -= 1000;
    }
    return score;
}

int main() {
    game_info_t ginfo;
    clog << "# read game info" << endl;
    cin >> ginfo;
    clog << "# done" << endl;
    cout << 0 << endl;
    clog << 0 << endl;
    player p(ginfo);
    while (true) {
        clog << "# read turn info" << endl;
        turn_info_t tinfo = getturninfo(cin, ginfo);
        clog << "# make a decision" << endl;
        action_plan_t plan = p.play(tinfo);
        // assert (is_valid_plan(plan, ginfo, tinfo));
        clog << "# done" << endl;
        cout << plan << endl;
        clog << plan << endl;
    }
}
