// docs/association-contract.md — FAUI/IUIN merge (oracle #1).
#include "s100/exchange_set.hpp"
#include "s100/types.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

#define CHECK(cond)                                                                         \
  do {                                                                                      \
    if (!(cond)) {                                                                          \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";        \
      std::exit(1);                                                                         \
    }                                                                                       \
  } while (0)

namespace {

s100::AssociationRef make_assoc(std::uint8_t rcnm, std::uint32_t rcid, const char* assoc,
                                const char* role, std::uint8_t insn,
                                const std::vector<s100::Attribute>& attrs = {}) {
  s100::AssociationRef a;
  a.rcnm = rcnm;
  a.rcid = rcid;
  a.association = assoc;
  a.role = role;
  a.instruction = insn;
  a.attributes = attrs;
  return a;
}

s100::Attribute make_attr(const char* code, const char* value) {
  s100::Attribute a;
  a.code = code;
  a.value = value;
  a.instruction = 1;
  return a;
}

bool has_peer(const std::vector<s100::AssociationRef>& list, std::uint32_t rcid) {
  return std::any_of(list.begin(), list.end(),
                     [&](const s100::AssociationRef& a) { return a.rcid == rcid; });
}

}  // namespace

void test_association_faui_merge() {
  std::vector<s100::AssociationRef> list;
  list.push_back(make_assoc(100, 2, "supports", "supportedBy", 0));

  {
    std::vector<s100::AssociationRef> patch;
    patch.push_back(make_assoc(100, 3, "supports", "supportedBy", 1));
    s100::LoadError err;
    CHECK(s100::apply_association_updates(list, patch, &err));
    CHECK(list.size() == 2);
    CHECK(has_peer(list, 2) && has_peer(list, 3));
  }
  {
    std::vector<s100::AssociationRef> patch;
    patch.push_back(make_assoc(100, 2, "supports", "supportedBy", 2));
    s100::LoadError err;
    CHECK(s100::apply_association_updates(list, patch, &err));
    CHECK(list.size() == 1);
    CHECK(has_peer(list, 3) && !has_peer(list, 2));
  }
  {
    std::vector<s100::AssociationRef> patch;
    patch.push_back(make_assoc(100, 3, "supports", "supportedBy", 3, {make_attr("status", "1")}));
    s100::LoadError err;
    CHECK(s100::apply_association_updates(list, patch, &err));
    CHECK(list.size() == 1);
    CHECK(list[0].rcid == 3);
    CHECK(list[0].attributes.size() == 1);
    CHECK(list[0].attributes[0].code == "status" && list[0].attributes[0].value == "1");
  }
  {
    auto before = list;
    std::vector<s100::AssociationRef> patch;
    patch.push_back(make_assoc(100, 9, "supports", "supportedBy", 9));
    s100::LoadError err;
    CHECK(!s100::apply_association_updates(list, patch, &err));
    CHECK(list.size() == before.size());
    CHECK(list[0].rcid == before[0].rcid);
    CHECK(list[0].attributes.size() == before[0].attributes.size());
    CHECK(list[0].attributes.empty() || list[0].attributes[0].value == before[0].attributes[0].value);
  }
  {
    // Modify with no identity match → fail
    auto before = list;
    std::vector<s100::AssociationRef> patch;
    patch.push_back(make_assoc(100, 42, "supports", "supportedBy", 3, {make_attr("status", "2")}));
    s100::LoadError err;
    CHECK(!s100::apply_association_updates(list, patch, &err));
    CHECK(list.size() == before.size());
  }
  {
    std::vector<s100::AssociationRef> patch;
    patch.push_back(make_assoc(100, 7, "supports", "supportedBy", 0));
    s100::LoadError err;
    CHECK(s100::apply_association_updates(list, patch, &err));
    CHECK(list.size() == 1 && list[0].rcid == 7);
  }
  std::cout << "association_faui_merge ok\n";
}
