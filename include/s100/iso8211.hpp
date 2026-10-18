#pragma once
#include "s100/dataset.hpp"
#include <cstdint>
#include <vector>

namespace s100 {

struct Iso8211Field {
  std::string tag;
  std::vector<std::uint8_t> data;
};

struct Iso8211Record {
  std::size_t offset{0};
  std::uint32_t length{0};
  char leader_id{'?'};  // 'L' DDR, 'D' data, etc.
  std::vector<Iso8211Field> fields;
  const Iso8211Field* find(const char* tag) const;
};

/** Low-level ISO/IEC 8211 record iterator over an S-100 Part 10a cell. */
bool parse_iso8211_records(const std::vector<std::uint8_t>& bytes,
                           std::vector<Iso8211Record>& out, LoadError* err);

/** Decode a DSID field body (as stored in Iso8211Field::data); `ok` false if DSED unparseable. */
DatasetIdentification parse_s101_dsid(const std::vector<std::uint8_t>& dsid_field);

}  // namespace s100
