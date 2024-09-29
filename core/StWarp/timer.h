#ifndef STWARP_TIMER_H
#define STWARP_TIMER_H

#include <maya/MGlobal.h>
#include <chrono>
#include <sstream>

class ScopedTimer {
 public:
  ScopedTimer(const MString& message)
      : message_(message), startTime_(std::chrono::steady_clock::now()) {
    std::stringstream ss;
    ss << "Starting " << message_.asChar();
    MGlobal::displayInfo(ss.str().c_str());
  }

  void print() {
    auto endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsedTime =
        endTime - startTime_;
    std::stringstream ss;
    ss << message_.asChar() << " took " << elapsedTime.count() << " ms";
    MGlobal::displayInfo(ss.str().c_str());
  }

 private:
  MString message_;
  std::chrono::time_point<std::chrono::steady_clock> startTime_;
};

#endif  // STWARP_TIMER_H
