#ifndef STWARP_BARYCENTRIC_H_
#define STWARP_BARYCENTRIC_H_

#include "StWarp/type.h"

namespace StWarp {

template <typename T>
inline T clamp(T value, T min, T max) {
  return std::max(min, std::min(max, value));
}

Vec3d computeBarycentricCoordinates(const Vec3d& c, const Vec3d& p0,
                                    const Vec3d& p1, const Vec3d& p2);

Vec4d computeBilinearCoordinates(const Vec3d& c, const Vec3d& p0,
                                 const Vec3d& p1, const Vec3d& p2,
                                 const Vec3d& p3);

}  // namespace StWarp

#endif  // STWARP_BARYCENTRIC_H_
