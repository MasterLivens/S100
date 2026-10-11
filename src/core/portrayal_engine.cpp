#include "s100/portrayal_engine.hpp"

#include <cstdio>
#include <map>
#include <memory>
#include <set>

// Lua is built as C++ (third_party/CMakeLists.txt): plain headers, C++ linkage.
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "s100/dataset.hpp"
#include "s100/feature_catalogue.hpp"

namespace s100 {

// ------------------------------------------------------------------ context

PortrayalContext PortrayalContext::defaults(const PortrayalCatalogue& pc) {
  PortrayalContext c;
  for (const auto& p : pc.context_parameters) c.values[p.id] = p.default_value;
  return c;
}

bool PortrayalContext::set(const PortrayalCatalogue& pc, const std::string& id, const std::string& value,
                           std::string* err) {
  if (!pc.find_context_parameter(id)) {
    if (err) *err = "context parameter '" + id + "' not declared by PC " + pc.product + " " + pc.version;
    return false;
  }
  values[id] = value;
  return true;
}

std::string PortrayalContext::hash() const {
  std::uint64_t h = 1469598103934665603ull;  // FNV-1a 64
  for (const auto& [k, v] : values) {
    for (char c : k + "=" + v + ";") {
      h ^= static_cast<unsigned char>(c);
      h *= 1099511628211ull;
    }
  }
  char buf[17];
  std::snprintf(buf, sizeof buf, "%016llx", static_cast<unsigned long long>(h));
  return buf;
}

// ------------------------------------------------------------------ engine

namespace {

struct LuaState {
  lua_State* L{luaL_newstate()};
  ~LuaState() {
    if (L) lua_close(L);
  }
};

void open_lib(lua_State* L, lua_CFunction f, const char* name) {
  lua_pushcfunction(L, f);
  lua_pushstring(L, name);
  lua_call(L, 1, 0);
}

int traceback(lua_State* L) {
  // debug library is not opened (sandbox); keep the message as is.
  if (!lua_isstring(L, 1)) lua_pushliteral(L, "(non-string Lua error)");
  return 1;
}

bool pcall(lua_State* L, int nargs, int nres, std::string* err, const char* what) {
  const int base = lua_gettop(L) - nargs;
  lua_pushcfunction(L, traceback);
  lua_insert(L, base);
  const int rc = lua_pcall(L, nargs, nres, base);
  lua_remove(L, base);
  if (rc != 0) {
    if (err) *err = std::string(what) + ": " + (lua_isstring(L, -1) ? lua_tostring(L, -1) : "?");
    lua_pop(L, 1);
    return false;
  }
  return true;
}

// Unregistered Host* globals fail loudly (contract rule 2). Other unknown globals keep normal
// Lua semantics (nil), which the official framework relies on.
const char* kStrictHost = R"lua(
local product = ...
setmetatable(_G, { __index = function(t, k)
  if type(k) == 'string' and k:sub(1, 4) == 'Host' then
    error("Host function '" .. k .. "' is not implemented by the " .. product .. " host", 2)
  end
end })
)lua";

}  // namespace

bool PortrayalEngine::run(HostAdapter& host, const PortrayalContext& ctx, DrawingInstructions& out,
                          PortrayalStats* stats_out, std::string* err) const {
  PortrayalStats local;
  PortrayalStats& stats = stats_out ? *stats_out : local;
  stats = PortrayalStats{};
  host.reset_run();
  out.clear();

  LuaState state;
  lua_State* L = state.L;
  if (!L) {
    if (err) *err = "cannot create Lua state";
    return false;
  }
  open_lib(L, luaopen_base, "");
  open_lib(L, luaopen_package, LUA_LOADLIBNAME);
  open_lib(L, luaopen_table, LUA_TABLIBNAME);
  open_lib(L, luaopen_string, LUA_STRLIBNAME);
  open_lib(L, luaopen_math, LUA_MATHLIBNAME);
  // Sandbox: rules are loaded only through require from the PC Rules directory.
  for (const char* g : {"dofile", "loadfile"}) {
    lua_pushnil(L);
    lua_setglobal(L, g);
  }
  lua_getglobal(L, "package");
  const std::string path = (pc_->rules_dir() / "?.lua").string();
  lua_pushstring(L, path.c_str());
  lua_setfield(L, -2, "path");
  lua_pushstring(L, "");
  lua_setfield(L, -2, "cpath");
  lua_pushnil(L);
  lua_setfield(L, -2, "loadlib");
  lua_pop(L, 1);

  // Host functions from the product registry; each gets the adapter as upvalue.
  for (const auto& name : host.registry().names()) {
    lua_pushlightuserdata(L, &host);
    lua_pushcclosure(L, host.registry().function(name), 1);
    lua_setglobal(L, name.c_str());
  }
  if (luaL_loadstring(L, kStrictHost) != 0) {
    if (err) *err = "strict host chunk";
    return false;
  }
  lua_pushstring(L, host.product().c_str());
  if (!pcall(L, 1, 0, err, "strict host")) return false;

  // Top level template (main.lua), unmodified.
  const auto main_file = (pc_->rules_dir() / pc_->top_level_rule()).string();
  if (luaL_loadfile(L, main_file.c_str()) != 0) {
    if (err) *err = std::string("load ") + main_file + ": " + lua_tostring(L, -1);
    return false;
  }
  if (!pcall(L, 0, 0, err, "main.lua")) return false;

  // Part 9a 9a-14.1.2/3: context parameters with PC defaults, then session values.
  lua_newtable(L);
  int n = 0;
  for (const auto& p : pc_->context_parameters) {
    lua_getglobal(L, "PortrayalCreateContextParameter");
    lua_pushstring(L, p.id.c_str());
    lua_pushstring(L, p.type.c_str());
    lua_pushstring(L, p.default_value.c_str());
    if (!pcall(L, 3, 1, err, "PortrayalCreateContextParameter")) return false;
    lua_rawseti(L, -2, ++n);
  }
  lua_getglobal(L, "PortrayalInitializeContextParameters");
  lua_insert(L, -2);
  if (!pcall(L, 1, 0, err, "PortrayalInitializeContextParameters")) return false;
  for (const auto& p : pc_->context_parameters) {
    auto it = ctx.values.find(p.id);
    if (it == ctx.values.end() || it->second == p.default_value) continue;
    lua_getglobal(L, "PortrayalSetContextParameter");
    lua_pushstring(L, p.id.c_str());
    lua_pushstring(L, it->second.c_str());
    if (!pcall(L, 2, 0, err, "PortrayalSetContextParameter")) return false;
  }
  for (const auto& [id, v] : ctx.values)
    if (!pc_->find_context_parameter(id)) {
      if (err) *err = "context parameter '" + id + "' not declared by the PC";
      return false;
    }

  // Part 9a 9a-14.1.1: all features (nil = every feature ID).
  stats.features = host.feature_count();
  lua_getglobal(L, "PortrayalMain");
  lua_pushnil(L);
  if (!pcall(L, 1, 1, err, "PortrayalMain")) {
    ++stats.lua_errors;
    return false;
  }
  const bool ok = lua_toboolean(L, -1) != 0;
  lua_pop(L, 1);

  stats.lua_errors += host.trace_errors.size();
  for (const auto& e : host.trace_errors)
    if (stats.errors.size() < 100) stats.errors.push_back(e);
  stats.first_chance_errors = host.first_chance_errors;
  stats.warnings = host.warnings;
  stats.emitted_features = host.emissions.size();

  std::set<std::uint64_t> with_instructions;
  for (const auto& e : host.emissions) {
    std::uint64_t fid = 0;
    if (!host.feature_id_of(e.feature_id, fid)) {
      if (err) *err = "HostPortrayalEmit for unknown feature ID " + e.feature_id;
      return false;
    }
    const std::size_t before = out.size();
    parse_drawing_instructions(fid, e.instructions, out, &stats);
    if (out.size() > before) with_instructions.insert(fid);
  }
  if (const auto* s1 = dynamic_cast<const S101HostAdapter*>(&host)) {
    for (const auto& f : s1->dataset().features)
      if (!with_instructions.count(f.id)) stats.features_without_instructions.push_back(f.id);
  }
  if (!ok) {
    if (err) *err = "PortrayalMain returned false";
    return false;
  }
  return true;
}

bool portray(const Dataset& ds, const FeatureCatalogue& fc, const PortrayalCatalogue& pc,
             const PortrayalContext& ctx, DrawingInstructions& out, std::string* err,
             PortrayalStats* stats) {
  S101HostAdapter host(ds, fc);
  return PortrayalEngine(pc).run(host, ctx, out, stats, err);
}

bool portray(const CoverageDataset& cov, const FeatureCatalogue& fc, const PortrayalCatalogue& pc,
             const PortrayalContext& ctx, DrawingInstructions& out, std::string* err,
             PortrayalStats* stats) {
  S102HostAdapter host(cov, fc);
  return PortrayalEngine(pc).run(host, ctx, out, stats, err);
}

// ------------------------------------------------------------------ layers

void apply_portrayal_to_layers(Dataset& ds, const DrawingInstructions& di, const PortrayalCatalogue& pc) {
  std::map<std::uint64_t, const DrawingInstruction*> first;
  for (const auto& d : di) first.emplace(d.feature_id, &d);
  struct Tally {
    std::map<std::string, int> vg, plane;
    std::map<int, int> prio;
  };
  std::map<std::string, Tally> tally;
  for (const auto& f : ds.features) {
    auto it = first.find(f.id);
    if (it == first.end()) continue;
    auto& t = tally[f.feature_type];
    ++t.vg[it->second->viewing_group];
    ++t.plane[it->second->display_plane];
    ++t.prio[it->second->drawing_priority];
  }
  auto best = [](const auto& m) {
    auto b = m.begin();
    for (auto i = m.begin(); i != m.end(); ++i)
      if (i->second > b->second) b = i;
    return b->first;
  };
  for (const auto& [code, t] : tally) {
    if (!ds.layers.find(code)) continue;  // layers come from classify_layers only (no new keys)
    LayerInfo& li = ds.layers.upsert(code);
    li.viewing_group = best(t.vg);
    li.display_plane = best(t.plane);
    li.drawing_priority = best(t.prio);
    li.display_category = pc.display_mode_of(li.viewing_group);
  }
}

// ------------------------------------------------------------------ session

std::string PortrayalSession::cache_key(const Dataset& ds) const {
  return ds.path.string() + "|" + pc_->product + " " + pc_->version + "|" + ctx_.hash();
}

const DrawingInstructions* PortrayalSession::portray(Dataset& ds, PortrayalStats* stats, std::string* err) {
  const std::string key = cache_key(ds);
  auto it = cache_.find(key);
  if (it == cache_.end()) {
    Entry e;
    if (!s100::portray(ds, *fc_, *pc_, ctx_, e.instructions, err, &e.stats)) return nullptr;
    ++runs_;
    it = cache_.emplace(key, std::move(e)).first;
  }
  apply_portrayal_to_layers(ds, it->second.instructions, *pc_);
  if (stats) *stats = it->second.stats;
  return &it->second.instructions;
}

}  // namespace s100
