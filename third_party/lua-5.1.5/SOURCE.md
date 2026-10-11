# Lua 5.1.5 (vendored, unmodified)

| Field | Value |
|-------|-------|
| Version | Lua 5.1.5 (final 5.1 release, 2012-02-13) |
| Source | https://www.lua.org/ftp/lua-5.1.5.tar.gz |
| Tarball SHA-256 | `2640fc56a795f29d28ef15e13c34a47e223960b0240e8cb0a82d9b0738695333` (matches the checksum published on https://www.lua.org/ftp/, checked 2026-10-11) |
| Files kept | `COPYRIGHT`, `README`, `src/*.c`, `src/*.h`, `etc/lua.hpp` (byte-identical to the tarball; docs, tests, Makefiles dropped) |
| Tree SHA-256 | `062f4c2a4a414834e5eb6172354bf507139a38271bb1efa623cffd22f110b414` (58 files, excluding this SOURCE.md; checked by CTest `third_party_provenance`) |
| License | MIT (see `COPYRIGHT`) |
| Build | `third_party/CMakeLists.txt` → static lib `s100_lua`, sources compiled **as C++** (Lua 5.1 supports this; `lua_error` then unwinds via C++ exceptions so host functions' destructors run). No `lua.c`/`luac.c`/`print.c`. No system package needed. Include `lua.h`/`lauxlib.h`/`lualib.h` directly, not `etc/lua.hpp`. |

## Why 5.1

- S-100 Ed 5.2.0 Part 13, clause 13-2 Conformance: "Scripts conforming to this part shall be
  implemented using version 5.1 of the Lua programming language." Clause 13-7.1 Lua version:
  "The host must provide a scripting engine; a Lua version 5.1 interpreter or virtual machine.
  The reference implementation is available from lua.org ... Embedding the reference
  implementation into the host is recommended."
  (https://iho.int/uploads/user/pubs/standards/s-100/S-100_5.2.0_Final_Clean.pdf, fetched
  2026-10-11, sha256 dd5ab86a3f6b5a26f32f7468a2eb3023e7d87a63423c8567c44657e8d3373732.)
- The S-101 PC 2.0.0 framework says the same in `Rules/main.lua`: "NOTE: S-100 requires a Lua 5.1
  scripting engine ... the scripts are only guaranteed to run as intended on Lua 5.1" (the
  `EqMetaMethodGuarantee` probe there warns on 5.3+ `__eq` semantics).
- 5.1.5 is the last bug-fix release of 5.1.
