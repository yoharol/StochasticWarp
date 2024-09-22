#include "StWarp/barycentric.h"

#include <Eigen/Dense>

namespace StWarp {

Vec3d computeBarycentricCoordinates(const Vec3d& c, const Vec3d& p0,
                                    const Vec3d& p1, const Vec3d& p2) {
  Vec3d v0 = p1 - p0;
  Vec3d v1 = p2 - p0;
  Vec3d v2 = c - p0;

  double d00 = v0.dot(v0);
  double d01 = v0.dot(v1);
  double d11 = v1.dot(v1);
  double d20 = v2.dot(v0);
  double d21 = v2.dot(v1);

  double denom = d00 * d11 - d01 * d01;

  double w0, w1, w2;

  if (denom == 0.0) {
    w0 = w1 = w2 = 0.0;
  } else {
    w1 = (d11 * d20 - d01 * d21) / denom;
    w2 = (d00 * d21 - d01 * d20) / denom;
    w0 = 1.0 - w1 - w2;
  }
  Vec3d w;
  w << w0, w1, w2;
  return w;
}

Vec4d computeBilinearCoordinates(const Vec3d& c, const Vec3d& p0,
                                 const Vec3d& p1, const Vec3d& p2,
                                 const Vec3d& p3) {
  const double tol = 1e-6;
  const int maxIter = 50;
  double u = 0.5;
  double v = 0.5;

  for (int iter = 0; iter < maxIter; ++iter) {
    Vec3d c_estimated = (1 - u) * (1 - v) * p0 + u * (1 - v) * p1 + u * v * p2 +
                        (1 - u) * v * p3;
    Vec3d F = c_estimated - c;
    if (F.norm() < tol) {
      break;
    }
    Vec3d Fu = -(1 - v) * p0 + (1 - v) * p1 + v * p2 - v * p3;
    Vec3d Fv = -(1 - u) * p0 - u * p1 + u * p2 + (1 - u) * p3;

    Mat2d J;
    J(0, 0) = Fu.dot(Fu);
    J(0, 1) = Fu.dot(Fv);
    J(1, 0) = Fv.dot(Fu);
    J(1, 1) = Fv.dot(Fv);

    Vec2d rhs;
    rhs(0) = -F.dot(Fu);
    rhs(1) = -F.dot(Fv);

    Vec2d delta = J.ldlt().solve(rhs);

    u += delta(0);
    v += delta(1);

    u = clamp(u, 0.0, 1.0);
    v = clamp(v, 0.0, 1.0);

    if (delta.norm() < tol) {
      break;
    }
  }

  Vec4d w;
  w << (1 - u) * (1 - v), u * (1 - v), u * v, (1 - u) * v;
  return w;
}

}  // namespace StWarp