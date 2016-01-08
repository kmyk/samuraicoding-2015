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
    array<vector<point_t>,ENEMY_NUM> eposs; // estimated positions of enemies
    array<vector<vector<set<int> > >,ENEMY_NUM> is_dangerous; // is_dangerous[enemy id][y][x] -> { eposs ixs }
    array<int,ENEMY_NUM> eturns;
    default_random_engine engine;

    int weapon() const { return ginfo.weapon; }
    int h() const { return ginfo.height; }
    int w() const { return ginfo.width; }
    int state() const { return tinfo.state[ginfo.weapon]; }
    point_t pos() const { return tinfo.pos[ginfo.weapon]; }

private:
    void update();
    void update_estimated_positions();
    void update_is_dangerous();
    double evaluate(action_plan_t const & plan);

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

void player::update_estimated_positions() {
    repeat (i,ENEMY_NUM) eposs[i].clear();
    array<bool,ENEMY_NUM> is_hidden;
    repeat (i,ENEMY_NUM) {
        is_hidden[i] = not is_on_field(tinfo.pos[FRIEND_NUM + i], ginfo);
        if (not is_hidden[i]) {
            eposs[i].push_back(tinfo.pos[FRIEND_NUM + i]);
        }
    }
    if (not tinfos.empty()) {
        // gather difference
        array<vector<point_t>,ENEMY_NUM> attacked;
        repeat (y,h()) {
            repeat (x,w()) {
                int cur = tinfo.field[y][x];
                int prv = tinfos.back().field[y][x];
                if (cur == prv) continue;
                if (cur == F_UNKNOWN) continue;
                if (prv == F_UNKNOWN) continue;
                if (not is_field_enemy(cur)) continue;
                attacked[cur - F_OCCUPIED - FRIEND_NUM].push_back((point_t){ y, x });
            }
        }
        repeat (i,ENEMY_NUM) {
            sort(attacked[i].begin(), attacked[i].end()); // sort to compare
        }
        // gather aposs
        array<vector<point_t>,ENEMY_NUM> aposs = {}; // positions where enemy might attack from
        repeat (y,h()) {
            repeat (x,w()) {
                point_t p = { y, x };
                repeat (i,ENEMY_NUM) if (is_hidden[i] and not attacked[i].empty()) {
                    repeat (j,DIRECTION_NUM) {
                        vector<point_t> poss;
                        repeat (k, ATTACK_AREA_NUM[i]) {
                            point_t q = p + rotdir(ATTACK_AREA[i][k], j);
                            if (not is_on_field(q, ginfo)) continue;
                            if (tinfo.field[q.y][q.x] == F_UNKNOWN) continue;
                            if (tinfos.back().field[q.y][q.x] == F_UNKNOWN) continue;
                            if (tinfos.back().field[q.y][q.x] == F_OCCUPIED + FRIEND_NUM + i) continue;
                            poss.push_back(q);
                        }
                        sort(poss.begin(), poss.end());
                        if (poss == attacked[i]) {
                            aposs[i].push_back(p);
                            break;
                        }
                    }
                }
            }
        }
        // construct eposs
        repeat (i,ENEMY_NUM) if (is_hidden[i] and not attacked[i].empty()) {
            point_t prv = tinfos.back().pos[FRIEND_NUM + i];
            if (not is_on_field(prv, ginfo)) {
                // from hidden, appear -> attack -> hide
                for (point_t apos : aposs[i]) {
                    if (is_field_enemy(tinfo.field[apos.y][apos.x]) and
                            is_field_enemy(tinfos.back().field[apos.y][apos.x])) {
                        eposs[i].push_back(apos);
                    }
                }
            } else {
                // from appearing, (attack & move) -> hide
                for (point_t apos : aposs[i]) {
                    if (apos == prv) {
                        // attack -> move
                        repeat (j,DIRECTION_NUM+1) {
                            point_t q = prv + direction[j];
                            if (not is_on_field(q, ginfo)) continue;
                            if (not is_field_enemy(tinfo.field[q.y][q.x])) continue;
                            eposs[i].push_back(q);
                        }
                    } else {
                        // move -> attack
                        if (manhattan_distance(prv, apos) > 1) continue;
                        if (not is_field_enemy(tinfo.field[apos.y][apos.x])) continue;
                        eposs[i].push_back(apos);
                    }
                }
            }
        }

vector<vector<char> > f(h(), vector<char>(w()));
repeat (y,h()) {
    repeat (x,w()) {
        if (tinfo.field[y][x] == F_UNKNOWN) {
            f[y][x] = '#';
        } else {
            f[y][x] = '.';
        }
    }
}
repeat (i,ENEMY_NUM) {
    for (point_t p : aposs[i]) {
        f[p.y][p.x] = 'd' + i;
    }
}
repeat (i,ENEMY_NUM) {
    for (point_t p : eposs[i]) {
        f[p.y][p.x] = 'D' + i;
    }
}
repeat (y,h()) {
    repeat (x,w()) {
        cerr << f[y][x];
    }
    cerr << endl;
}

    }
}

void player::update_is_dangerous() {
    repeat (i,ENEMY_NUM) {
        is_dangerous[i].clear();
        is_dangerous[i].resize(h(), vector<set<int> >(w()));
        if (not eturns[i]) continue;
        repeat (pi, eposs[i].size()) {
            point_t p = eposs[i][pi];
            int r = eturns[i]*2+1;
            repeat (j,r*r) {
                point_t dp = { j / r - eturns[i], j % r - eturns[i] };
                repeat (k,DIRECTION_NUM) {
                    repeat (l,ATTACK_AREA_NUM[i]) {
                        point_t q = p + dp + rotdir(ATTACK_AREA[i][l], k);
                        if (not is_on_field(q, ginfo)) continue;
                        is_dangerous[i][q.y][q.x].insert(pi);
                    }
                }
            }
        }
    }
}

void player::update() {
    eturns = turns_to_next(tinfo);
    repeat (y,h()) {
        repeat (x,w()) {
            if (tinfo.field[y][x] == F_UNKNOWN) continue;
            field[y][x] = tinfo.field[y][x];
        }
    }
    update_estimated_positions();
    update_is_dangerous();
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
    double highscore = 0;
    action_plan_t t;
    if (state() == S_HIDDEN) {
        t.a.push_back(A_APPEAR);
    }
    repeat (i,DIRECTION_NUM + 1) {
        if (i < DIRECTION_NUM) t.a.push_back(A_MOVE + i);
        if (is_valid_plan(t, ginfo, tinfo)) {
            repeat (j,DIRECTION_NUM) {
                t.a.push_back(A_ATTACK + j);
                double score = evaluate(t);
                if (highscore < score) {
                    highscore = score;
                    plan = t;
                }
                t.a.pop_back();
            }
        }
        if (i < DIRECTION_NUM) t.a.pop_back();
    }
    cerr << "score: " << highscore << endl;
    if (highscore < 170) {
        // there are no enough space, goto center (heuristic)
        double score[DIRECTION_NUM] = {};
        repeat_from (dy,-7,7+1) repeat_from (dx,-7,7+1) {
            point_t p = pos() + (point_t){ dy, dx };
            if (not is_on_field(p, ginfo)) continue;
            if (not is_field_friend(field[p.y][p.x])) {
                if (dy < 0) score[D_SOUTH] += 1;
                if (dy > 0) score[D_NORTH] += 1;
                if (dx < 0) score[D_WEST] += 1;
                if (dx > 0) score[D_EAST] += 1;
            }
        }
        highscore = -1; // shadowing
        int j = -1;
        repeat (i,DIRECTION_NUM) {
            if (highscore < score[i]) {
                highscore = score[i];
                j = i;
            }
        }
        plan.a.clear();
        plan.a.push_back(A_MOVE + j);
        plan.a.push_back(A_MOVE + j);
        plan.a.push_back(A_MOVE + j);
        if (state() == S_HIDDEN) {
            if (not is_valid_plan(plan, ginfo, tinfo)) {
                plan.a.clear();
                plan.a.push_back(A_APPEAR);
                plan.a.push_back(A_MOVE + j);
                plan.a.push_back(A_MOVE + j);
                plan.a.push_back(A_MOVE + j);
            }
        }
        while (not is_valid_plan(plan, ginfo, tinfo)) {
            plan.a.pop_back();
        }
    }
    plan.a.push_back(A_HIDE);
    while (not is_valid_plan(plan, ginfo, tinfo)) {
        plan.a.pop_back();
    }
    return plan;
}

double player::evaluate(action_plan_t const & plan) {
    if (not is_valid_plan(plan, ginfo, tinfo)) return -1;
    double score = 0;
    vector<vector<int> > f = field;
    array<set<int>,ENEMY_NUM> killed; // eposs ids
    point_t p = pos();
    for (int a : plan.a) {
        if (is_action_attack(a)) {
            repeat (i, ATTACK_AREA_NUM[weapon()]) {
                point_t q = p + rotdir(ATTACK_AREA[weapon()][i], a - A_ATTACK);
                if (not is_on_field(q, ginfo)) continue;
                repeat (j,ENEMY_NUM) {
                    repeat (k,eposs[j].size()) {
                        if (q == eposs[j][k]) {
                            killed[j].insert(k);
                            if (q == ginfo.home[FRIEND_NUM + j]) {
                                score += 80 / eposs[j].size(); // may be curing
                            } else {
                                score += 100000 / eposs[j].size();
                            }
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
    if (total_cost(plan) < 7 and is_field_friend(f[p.y][p.x])) {
        score += 30;
    }
    repeat (i,ENEMY_NUM) {
        for (int j : is_dangerous[i][p.y][p.x]) {
            if (not killed[i].count(j)) {
                score -= 150000 / eposs[i].size();
            }
        }
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
        if (not cin) break;
        clog << "# make a decision" << endl;
        action_plan_t plan = p.play(tinfo);
        assert (is_valid_plan(plan, ginfo, tinfo));
        clog << "# done" << endl;
        cout << plan << endl;
        clog << plan << endl;
    }
}
