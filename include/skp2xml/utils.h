// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#ifndef SKPTOXML_COMMON_UTILS_H
#define SKPTOXML_COMMON_UTILS_H

#include <SketchUpAPI/import_export/pluginprogresscallback.h>

// Common miscellaneous utilities

// Returns true if import/export has been cancelled by the user.
bool IsCancelled(SketchUpPluginProgressCallback* callback) {
  return callback != NULL && callback->HasBeenCancelled();
}

// Calls an SU* API function, checks for successful return value. If not
// successful, throws an exception to be caught by the top level handler.
// Should only be used if function is expected to return SU_ERROR_NONE.
#define SU_CALL(func) if ((func) != SU_ERROR_NONE) throw std::exception()

// Set progress percent and message, if progress callback is available.
void HandleProgress(SketchUpPluginProgressCallback* callback,
                    double percent_done, const char* message) {
  if (callback != NULL) {
    if (callback->HasBeenCancelled()) {
      // Throw an exception to be caught by the top-level handler.
      throw std::exception();
    } else {
      callback->SetPercentDone(percent_done);
      callback->SetProgressMessage(message);
    }
  }
}

#endif // SKPTOXML_COMMON_UTILS_H
