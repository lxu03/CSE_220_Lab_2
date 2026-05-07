#include "miss_counter.h"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <list>
#include <unordered_map>
#include <unordered_set>

class FACache {
 public:
  explicit FACache(unsigned capacity) : capacity_(capacity) {
    map_.reserve(capacity * 2);
  }

  bool access(uint64_t line_addr) {
    auto it = map_.find(line_addr);
    if (it == map_.end()) return false;
    lru_list_.splice(lru_list_.begin(), lru_list_, it->second);
    return true;
  }

  void insert(uint64_t line_addr) {
    if (map_.size() >= capacity_) {
      uint64_t lru_addr = lru_list_.back();
      lru_list_.pop_back();
      map_.erase(lru_addr);
    }
    lru_list_.push_front(line_addr);
    map_[line_addr] = lru_list_.begin();
  }

 private:
  unsigned capacity_;
  std::list<uint64_t> lru_list_;
  std::unordered_map<uint64_t, std::list<uint64_t>::iterator> map_;
};

struct ProcState {
  bool        initialized = false;
  FACache*    fa_cache    = nullptr;
  std::unordered_set<uint64_t> ever_seen;
  uint64_t    counts[MISS_3C_COUNT];

  ProcState() { memset(counts, 0, sizeof(counts)); }
  ~ProcState() { delete fa_cache; }

  ProcState(const ProcState&)            = delete;
  ProcState& operator=(const ProcState&) = delete;
};

static constexpr unsigned MAX_PROCS = 64;
static ProcState g_states[MAX_PROCS];

extern "C" {

void miss_3c_init(unsigned proc_id, unsigned total_fa_lines) {
  ProcState& s = g_states[proc_id];

  s.fa_cache   = new FACache(total_fa_lines);
  s.initialized = true;
}

int miss_3c_on_access(unsigned proc_id, uint64_t line_addr, int is_miss) {
  ProcState& s = g_states[proc_id];

  bool is_first = s.ever_seen.insert(line_addr).second;

  bool fa_hit = s.fa_cache->access(line_addr);
  if (!fa_hit) {
    s.fa_cache->insert(line_addr);
  }

  if (!is_miss) {
    return -1;
  }

  Miss3CType type;

  if (is_first) {
    type = MISS_3C_COMPULSORY;
  } else if (!fa_hit) {
    type = MISS_3C_CAPACITY;
  } else {
    type = MISS_3C_CONFLICT;
  }

  s.counts[type]++;
  return static_cast<int>(type);
}

}
