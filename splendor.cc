// Copyright 2016, Krzysztof Duleba
//
//
// Code helping to solve the Koh-i-noor Splendor challenge.
// It estimates the likelihood that a given board results in a win.
//
// It takes as input to STDIN a description of the scenario - the initial cards
// as well as any cards that show up later in the sequence.
//
// Sample input, consisting only of the initial board description:
// 0 2 0 2 0 green 0
// 0 0 3 0 0 black 0
// 1 0 1 1 1 red 0
// 0 0 0 1 2 green 0
// 0 6 0 0 0 red 3
// 6 0 0 0 0 black 3
// 0 2 4 1 0 black 2
// 3 2 0 3 0 red 1
// 6 0 6 8 6 red 10
// 6 8 6 6 0 white 10
//
// If you play the scenario for a while and learn the sequence of the cards,
// you can add them at the end.




#include "splendor_cards.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Twister {
  void init(int seed) {
    tab_[0] = seed;
    for (int i = 1 ; i < N; ++i) {
      tab_[i] = 1812433253UL * (tab_[i - 1] ^ (tab_[i - 1] >> 30)) + i;
    }
    generate_numbers();
  }

  Twister () {
    ptr_ = N;
  }

  float next_float() {
    if (ptr_ >= N) generate_numbers();
    unsigned int a = extract_number(ptr_++) & 0x7FFFFFFF;
    return a * 4.656612873077392578125e-10;
  }

  int next_int() {
    if (ptr_ >= N) generate_numbers();
    return extract_number(ptr_++);
  }

  int next_int(int max) {
    unsigned int used = max - 1;
    used |= used >> 1;
    used |= used >> 2;
    used |= used >> 4;
    used |= used >> 8;
    used |= used >> 16;

    int a;
    do {
      a = next_int() & used;
    } while (a >= max);
    return a;
  }

  private:
  void generate_numbers() {
    ptr_ = 0;
    for (int i = 0; i < N - M; ++i) {
      unsigned int val = tab_[i + M], s1 = tab_[i + 1];
      unsigned int y = (tab_[i] & 0x80000000) | (s1 & 0x7FFFFFFF);
      tab_[i] = val ^ (y >> 1) ^ (-(s1 & 1) & 0x9908B0DF);
    }
    for (int i = N - M; i < N - 1; ++i) {
      unsigned int val = tab_[i + (M - N)], s1 = tab_[i + 1];
      unsigned int y = (tab_[i] & 0x80000000) | (s1 & 0x7FFFFFFF);
      tab_[i] = val ^ (y >> 1) ^ (-(s1 & 1) & 0x9908B0DF);
    }
    {
      int i = N - 1;
      unsigned int val = tab_[M - 1], s1 = tab_[0];
      unsigned int y = (tab_[i] & 0x80000000) | (s1 & 0x7FFFFFFF);
      tab_[i] = val ^ (y >> 1) ^ (-(s1 & 1) & 0x9908B0DF);
    }
  }

  int extract_number(int i) {
    unsigned int y = tab_[i];
    y = y ^ (y >> 11);
    y = y ^ ((y << 7) & 0x9D2C5680);
    y = y ^ ((y << 15) & 0xEFC60000);
    y = y ^ (y >> 18);
    return y;
  }

  static const int N = 624;
  static const int M = 397;
  unsigned int tab_[N];
  int ptr_;
};


#define FATAL { fprintf(stderr, "FATAL error in line %d\n", __LINE__); exit(1); }

const char* color_to_string(int c) {
  if (c == 0) return "black";
  if (c == 1) return "red";
  if (c == 2) return "green";
  if (c == 3) return "blue";
  if (c == 4) return "white";
  FATAL;
}

struct Card {
  char cost[5];
  char type;
  char value;

  Card() {type = -1;}
  Card(const Card &c) {
    cost[0] = c.cost[0];
    cost[1] = c.cost[1];
    cost[2] = c.cost[2];
    cost[3] = c.cost[3];
    cost[4] = c.cost[4];
    type = c.type;
    value = c.value;
  }
  bool operator==(const Card &c) const {
    if (cost[0] != c.cost[0]) return false;
    if (cost[1] != c.cost[1]) return false;
    if (cost[2] != c.cost[2]) return false;
    if (cost[3] != c.cost[3]) return false;
    if (cost[4] != c.cost[4]) return false;
    if (type != c.type) return false;
    return value == c.value;
  }
  bool operator<(const Card &c) const {
    if (cost[0] != c.cost[0]) return cost[0] < c.cost[0];
    if (cost[1] != c.cost[1]) return cost[1] < c.cost[1];
    if (cost[2] != c.cost[2]) return cost[2] < c.cost[2];
    if (cost[3] != c.cost[3]) return cost[3] < c.cost[3];
    if (cost[4] != c.cost[4]) return cost[4] < c.cost[4];
    if (type != c.type) return type < c.type;
    return value < c.value;
  }

  int color_str_to_int(const string &str) const {
    if (str == "black") {
      return 0;
    } else if (str == "red") {
      return 1;
    } else if (str == "green") {
      return 2;
    } else if (str == "blue") {
      return 3;
    } else if (str == "white") {
      return 4;
    } else {
      FATAL;
    }
  }

  bool read_from_string(const char *str) {
    istringstream iss(str);
    for (int i = 0; i < 5; ++i) {
      int x;
      iss >> x;
      if (!iss) return false;
      cost[i] = x;
    }

    string buff;
    iss >> buff;
    if (!iss) return false;
    type = color_str_to_int(buff);

    int y;
    iss >> y;
    if (!iss) return false;
    value = y;

    return true;
  }

  bool read() {
    char line[100];
    if (fgets(line, sizeof(line), stdin) == NULL) return false;
    if (read_from_string(line)) return true;
    return read();
  }

  int get_cost(const vector<int> &bonuses) const {
    int res = 0;
    for (int i = 0; i < 5; ++i) {
      if (bonuses[i] >= cost[i]) continue;
      if (bonuses[i] + 4 < cost[i]) return -1;
      res += cost[i] - bonuses[i];
    }
    if (res > 12) return -1;
    return res;
  }

  string ToString() const {
    ostringstream oss;
    oss << color_to_string(type) << " (" << (int)value << ") ";
    for (int j = 0; j < 5; ++j) {
      if (cost[j] > 0) oss << color_to_string(j) << " " << (int)cost[j] << ", ";
    }
    return oss.str();
  }
};

set<Card> full_deck_set;
void parse_full_deck() {
  int sz = sizeof(full_deck) / sizeof(full_deck[0]);
  for (int i = 0; i < sz; ++i) {
    Card c;
    c.read_from_string(full_deck[i]);
    full_deck_set.insert(c);
  }
}

struct Deck {
  Card table[4];
  int table_sz;
  Card q[30];
  int q_sz;

  Deck() {
    table_sz = 0;
    q_sz = 0;
  }

  void add_card(const Card &c) {
    if (table_sz < 4) table[table_sz++] = c;
    else q[q_sz++] = c;
  }

  bool can_peak_card(int idx) const {
    if (table_sz < idx) return false;
    if (table[idx].type == -1) return false;
    return true;
  }

  void pop_card(int idx) {
    table[idx].type = -1;
    if (q_sz > 0) {
      table[idx] = q[--q_sz];
    }
  }

  set<Card> to_set() const {
    set<Card> result;
    for (int i = 0; i < table_sz; ++i) result.insert(table[i]);
    for (int i = 0; i < q_sz; ++i) result.insert(q[i]);
    return result;
  }

  bool process_move(int x, int *points, int *tokens_cost, int *rounds, vector<int> *bonus, Card *card_sequence) {
    if (!can_peak_card(x)) return false;

    int cost = table[x].get_cost(*bonus);
    if (cost == -1) return false;
    if (*rounds + 1 + (*tokens_cost + 3 + cost) / 4 > 28) return false;

    *tokens_cost += cost;
    *rounds += 1;
    *points += table[x].value;
    (*bonus)[table[x].type] += 1;
    *card_sequence = table[x];
    pop_card(x);
    return true;
  }

  void fill_up_randomly(int desired_size, vector<Card> v, Twister *setup_twister) {
    while (table_sz < 4 && !v.empty()) {
      int x = setup_twister->next_int(v.size());
      table[table_sz++] = v[x];
      swap(v[x], v.back());
      v.pop_back();
    }

    while (q_sz < desired_size && !v.empty()) {
      int x = setup_twister->next_int(v.size());
      q[q_sz++] = v[x];
      swap(v[x], v.back());
      v.pop_back();
    }
  }
};

struct State {
  char move_sequence[40];
  int move_sequence_sz;
  int points;
  int tokens_cost;
  int rounds;
  Card card_sequence[40];
  int card_sequence_sz;

  State() {
    move_sequence_sz = 0;
    points = 0;
    tokens_cost = 0;
    rounds = 0;
    card_sequence_sz = 0;
  }

  State(const State &st) {
    memcpy(move_sequence, st.move_sequence, sizeof(move_sequence));
    move_sequence_sz = st.move_sequence_sz;
    points = st.points;
    tokens_cost = st.tokens_cost;
    rounds = st.rounds;
  }

  void DeepCopy(const State &st) {
    memcpy(move_sequence, st.move_sequence, sizeof(move_sequence));
    move_sequence_sz = st.move_sequence_sz;
    points = st.points;
    tokens_cost = st.tokens_cost;
    rounds = st.rounds;
    memcpy(card_sequence, st.card_sequence, sizeof(card_sequence));
    card_sequence_sz = st.card_sequence_sz;
  }

  void play_out(Deck d1, Deck d2, Deck d3, const State &cand) {
    vector<int> bonus(5, 0);

    int ptr = 0;
    while (ptr < cand.move_sequence_sz) {
      int move = cand.move_sequence[ptr++];
      int x = move % 16;

      if ((x < 4 && d1.process_move(x, &points, &tokens_cost, &rounds, &bonus, card_sequence + card_sequence_sz)) ||
          (x >= 4 && x < 8 && d2.process_move(x - 4, &points, &tokens_cost, &rounds, &bonus, card_sequence + card_sequence_sz)) ||
          (x >= 8 && d3.process_move(x - 8, &points, &tokens_cost, &rounds, &bonus, card_sequence + card_sequence_sz))) {
        move_sequence[move_sequence_sz++] = x;
        ++card_sequence_sz;
      }
    }
  }

  bool mutate(Twister *twister) {
    int mod = twister->next_int(3);
    if (mod == 0) {
      // CHANGE
      if (move_sequence_sz == 0) return false;
      int pos = twister->next_int(move_sequence_sz);
      int old = move_sequence[pos];

      while (true) {
        int next = twister->next_int(10);
        if (next == old) continue;
        move_sequence[pos] = next;
        break;
      }
    } else if (mod == 1) {
      // INSERT
      if (move_sequence_sz > 30) return false;
      int pos = twister->next_int(move_sequence_sz+1);
      for (int i = move_sequence_sz; i > pos; --i) move_sequence[i] = move_sequence[i - 1];
      move_sequence[pos] = twister->next_int(10);
      ++move_sequence_sz;
    } else if (mod == 2) {
      // SWAP
      if (move_sequence_sz < 3) return false;
      int px = twister->next_int(move_sequence_sz);
      int py = twister->next_int(move_sequence_sz);
      if (px == py) return false;
      if (move_sequence[px] == move_sequence[py]) return false;
      swap(move_sequence[px], move_sequence[py]);
    }
    return true;
  }

  void print() {
    if (card_sequence_sz != move_sequence_sz) FATAL;
    printf("\npoints: %d, rounds: %d, tokens_cost: %d, cc %d\n", points, rounds, tokens_cost, rounds + (tokens_cost + 3) / 4);
    for (int i = 0; i < move_sequence_sz; ++i) {
      printf("%d: %s\n", move_sequence[i], card_sequence[i].ToString().c_str());
    }
    printf("\n\n");
    fflush(stdout);
  }
};


State best_state;

void play_randomly(Deck d1, Deck d2, Deck d3, Twister *annealing_twister) {
  State st;

  double start_temp = 2;
  double final_temp = 0.1;

  double temp = start_temp;

  double temp_cooldown = pow(final_temp / start_temp, 1.0 / 200000);
  while (temp > final_temp) {
    State cand = st;
    cand.card_sequence_sz = 0;
    while (!cand.mutate(annealing_twister)) {}

    State refined;
    refined.play_out(d1, d2, d3, cand);

    if (refined.points > best_state.points) {
      if (refined.points >= 31) refined.print();
      best_state.DeepCopy(refined);
    }

    if (exp((refined.points - st.points) / temp) > annealing_twister->next_float()) {
      st = refined;
    }
    temp *= temp_cooldown;
  }
}

void play_single_setting(Deck d1, Deck d2, Deck d3, Twister *annealing_twister) {
  reverse(d1.q, d1.q + d1.q_sz);
  reverse(d2.q, d2.q + d2.q_sz);

  for (int i = 0; i < 10; ++i) {
    play_randomly(d1, d2, d3, annealing_twister);
  }
}

void play_randomized_deck(Deck d1, Deck d2, Deck d3, Twister *setup_twister, Twister *annealing_twister) {
  set<Card> d1s = d1.to_set();
  set<Card> d2s = d2.to_set();

  vector<Card> remaining1;
  vector<Card> remaining2;
  for (set<Card>::iterator it = full_deck_set.begin(); it != full_deck_set.end(); ++it) {
    if (it->value == 0) {
      if (d1s.find(*it) == d1s.end()) remaining1.push_back(*it);
    } else if (it->value == 10) {
      // This deck is set, don't fill it up.
    } else {
      if (d2s.find(*it) == d2s.end()) remaining2.push_back(*it);
    }
  }

  d1.fill_up_randomly(25, remaining1, setup_twister);
  d2.fill_up_randomly(25, remaining2, setup_twister);

  best_state = State();

  play_single_setting(d1, d2, d3, annealing_twister);
}

int main() {
  Twister setup_twister;
  setup_twister.init(23590421);
  Twister annealing_twister;
  annealing_twister.init(549120939);

  parse_full_deck();

  Deck d1, d2, d3;
  while (true) {
    Card c;
    if (!c.read()) break;
    if (!full_deck_set.count(c)) {
      fprintf(stderr, "unrecognized card %s\n", c.ToString().c_str());
      exit(1);
    }

    if (c.value == 0) {
      d1.add_card(c);
    } else if (c.value == 10) {
      d3.add_card(c);
    } else {
      d2.add_card(c);
    }
  }

  int at_31 = 0;
  int mx = 0;
  for (int i = 0; i < 50; ++i) {
    play_randomized_deck(d1, d2, d3, &setup_twister, &annealing_twister);
    if (best_state.points >= 31) ++at_31;
    if (best_state.points > mx) mx = best_state.points;
    double ratio = 100.0 * at_31 / static_cast<double>(i + 1);
    printf("\rIter %d, Maximum: %d, Solvability likelihood: %.2lf %%, lift vs random board %.2lf", i+1, mx, ratio, ratio / 3.7);
    fflush(stdout);
  }
  printf("\n");
}
