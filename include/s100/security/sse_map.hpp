#pragma once

#include "s100/security/types.hpp"

namespace s100::security {

/** D5: THE single mapping from SigRejectCode to S-98 SSE integers / Table B-1 text.
 *  No other translation unit may hard-code Part 15 SSE integers. */
int sse_code(SigRejectCode c);
const char* sse_message(SigRejectCode c);

}  // namespace s100::security
