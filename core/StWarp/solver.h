#ifndef STWARP_SOLVER_H_
#define STWARP_SOLVER_H_

#include "StWarp/type.h"
#include <maya/MFnMesh.h>
#include <maya/MDoubleArray.h>

namespace StWarp {

struct StoWarpSolver {
  int n_mesh_verts;
  int n_cage_verts;
  int n_cage_faces;
  int n_tri_faces;
  int n_quad_faces;
  MatxXd mesh_verts;
  MatxXd cage_verts;
  Matx3i tri_faces;
  Matx4i quad_faces;
  MFnMesh& cageFn;
  MFnMesh& meshFn;
  std::vector<Mat4d> M;
  std::vector<Vec4d> m;
  MatxXd harmonic_weights;

  MDoubleArray harmonic_weights_maya;

  MStatus status;

  // index in tri_faces and quad_faces
  Vecxi face_idx;

  // 0: triangle, 1: quad
  Vecxi face_type;

  StoWarpSolver(MFnMesh& cageFn, MFnMesh& meshFn);

  double closest_point_on_cage(const Vec3d& input_p, Vec3d& closest_point,
                               int& face_idx);

  Vec3d get_tri_barycentric(const Vec3d& p, const int face_idx);
  Vec3d tri_interpolate(const Vec3d& w, const int face_idx);
  Vec4d get_quad_barycentric(const Vec3d& p, const int face_idx);
  Vec3d quad_interpolate(const Vec4d& w, const int face_idx);

  void walk_on_sphere_single_step(int maxSteps, double eps);
  void walk_on_sphere(int maxSteps, double eps, int n_walks);
};

}  // namespace StWarp

#endif  // STWARP_SOLVER_H_
