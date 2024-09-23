#include "StWarp/solver.h"

#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MGlobal.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MTypeId.h>
#include <maya/MObject.h>
#include <maya/MIntArray.h>
#include <sstream>

#include "Eigen/Dense"
#include "StWarp/type.h"
#include "StWarp/barycentric.h"

namespace StWarp {

inline float random(float rMin, float rMax) {
  const float rRandMax = 1. / (float)RAND_MAX;
  float u = rRandMax * (float)rand();
  return u * (rMax - rMin) + rMin;
}

Vec3d generateRandomDirection() {
  double zValue = random(-1.0, 1.0);
  double phi = random(0.0, 2.0 * M_PI);
  double xyScale = std::sqrt(1.0 - zValue * zValue);

  double x = xyScale * std::cos(phi);
  double y = xyScale * std::sin(phi);
  double z = zValue;
  Vec3d dir;
  dir << x, y, z;
  return dir;
}

StoWarpSolver::StoWarpSolver(MFnMesh& cageFn, MFnMesh& meshFn)
    : cageFn(cageFn), meshFn(meshFn) {
  MPointArray meshPoints;
  status = meshFn.getPoints(meshPoints, MSpace::kWorld);
  if (status != MS::kSuccess) {
    MGlobal::displayError("Failed to get mesh points.");
    return;
  }
  MPointArray cagePoints;
  status = cageFn.getPoints(cagePoints, MSpace::kWorld);
  if (status != MS::kSuccess) {
    MGlobal::displayError("Failed to get cage points.");
    return;
  }

  n_cage_verts = cagePoints.length();
  n_mesh_verts = meshPoints.length();
  M.resize(n_mesh_verts);
  m.resize(n_mesh_verts * n_cage_verts);
  harmonic_weights.resize(n_mesh_verts, n_cage_verts);
  for (auto& Mi : M) Mi.setZero();
  for (auto& mi : m) mi.setZero();
  cage_verts.resize(n_cage_verts, 3);
  mesh_verts.resize(n_mesh_verts, 3);

  for (int i = 0; i < n_cage_verts; i++) {
    cage_verts(i, 0) = cagePoints[i].x;
    cage_verts(i, 1) = cagePoints[i].y;
    cage_verts(i, 2) = cagePoints[i].z;
  }

  for (int i = 0; i < n_mesh_verts; i++) {
    mesh_verts(i, 0) = meshPoints[i].x;
    mesh_verts(i, 1) = meshPoints[i].y;
    mesh_verts(i, 2) = meshPoints[i].z;
  }

  MIntArray faceCounts;
  MIntArray faceConnects;
  status = cageFn.getVertices(faceCounts, faceConnects);
  if (status != MS::kSuccess) {
    MGlobal::displayError("Failed to get vertices.");
    return;
  }

  n_tri_faces = 0;
  n_quad_faces = 0;
  n_cage_faces = faceCounts.length();
  face_idx.resize(n_cage_faces);
  face_type.resize(n_cage_faces);
  for (int i = 0; i < n_cage_faces; i++) {
    if (faceCounts[i] == 3) {
      face_idx[i] = n_tri_faces;
      face_type[i] = 0;
      n_tri_faces++;
    } else if (faceCounts[i] == 4) {
      face_idx[i] = n_quad_faces;
      face_type[i] = 1;
      n_quad_faces++;
    } else {
      MGlobal::displayError("Only triangles and quads are supported.");
      status = MS::kFailure;
      return;
    }
  }
  tri_faces.resize(n_tri_faces, 3);
  quad_faces.resize(n_quad_faces, 4);

  int idx = 0;
  for (int i = 0; i < n_cage_faces; i++) {
    int k = face_idx[i];
    if (faceCounts[i] == 3) {
      tri_faces(face_idx[k], 0) = faceConnects[idx++];
      tri_faces(face_idx[k], 1) = faceConnects[idx++];
      tri_faces(face_idx[k], 2) = faceConnects[idx++];
    } else if (faceCounts[i] == 4) {
      quad_faces(face_idx[k], 0) = faceConnects[idx++];
      quad_faces(face_idx[k], 1) = faceConnects[idx++];
      quad_faces(face_idx[k], 2) = faceConnects[idx++];
      quad_faces(face_idx[k], 3) = faceConnects[idx++];
    }
  }

  // !
  // walk_on_sphere(100, 1e-6, 200);
  /*walk_on_sphere(100, 1e-6, 200);
  MatxXd new_verts = harmonic_weights * cage_verts;
  {
    std::stringstream ss;
    ss << harmonic_weights << std::endl;
    MGlobal::displayInfo(ss.str().c_str());
  }*/

  // for (int i = 0; i < n_mesh_verts; i++) {
  /*for (int i = 0; i < 1; i++) {
    meshFn.setPoint(
        i, MPoint(mesh_verts(i, 0), mesh_verts(i, 1), mesh_verts(i, 2)),
        MSpace::kWorld);
  }*/
  status = MS::kSuccess;
}

double StoWarpSolver::closest_point_on_cage(const Vec3d& input_p,
                                            Vec3d& closest_point, int& fi) {
  MPoint p(input_p(0), input_p(1), input_p(2));
  MPoint cp;
  status = cageFn.getClosestPoint(p, cp, MSpace::kWorld, &fi);
  if (status != MS::kSuccess) {
    MGlobal::displayError("Failed to get closest point.");
    return -1;
  }
  closest_point(0) = cp.x;
  closest_point(1) = cp.y;
  closest_point(2) = cp.z;
  return (input_p - closest_point).norm();
}

Vec3d StoWarpSolver::get_tri_barycentric(const Vec3d& p, const int fi) {
  int j = face_idx(fi);
  Vec3d p0 = cage_verts.row(tri_faces(j, 0)).transpose();
  Vec3d p1 = cage_verts.row(tri_faces(j, 1)).transpose();
  Vec3d p2 = cage_verts.row(tri_faces(j, 2)).transpose();
  return computeBarycentricCoordinates(p, p0, p1, p2);
}

Vec3d StoWarpSolver::tri_interpolate(const Vec3d& w, const int fi) {
  int j = face_idx(fi);
  return (w(0) * cage_verts.row(tri_faces(j, 0)) +
          w(1) * cage_verts.row(tri_faces(j, 1)) +
          w(2) * cage_verts.row(tri_faces(j, 2)))
      .transpose();
}

Vec4d StoWarpSolver::get_quad_barycentric(const Vec3d& p, const int fi) {
  int j = face_idx(fi);
  Vec3d p0 = cage_verts.row(quad_faces(j, 0)).transpose();
  Vec3d p1 = cage_verts.row(quad_faces(j, 1)).transpose();
  Vec3d p2 = cage_verts.row(quad_faces(j, 2)).transpose();
  Vec3d p3 = cage_verts.row(quad_faces(j, 3)).transpose();
  return computeBilinearCoordinates(p, p0, p1, p2, p3);
}

Vec3d StoWarpSolver::quad_interpolate(const Vec4d& w, const int fi) {
  int j = face_idx(fi);
  return (w(0) * cage_verts.row(quad_faces(j, 0)) +
          w(1) * cage_verts.row(quad_faces(j, 1)) +
          w(2) * cage_verts.row(quad_faces(j, 2)) +
          w(3) * cage_verts.row(quad_faces(j, 3)))
      .transpose();
}

void StoWarpSolver::walk_on_sphere_single_step(int maxSteps, double eps) {
  for (int i = 0; i < n_mesh_verts; i++) {
    Vec3d mp = mesh_verts.row(i).transpose();
    Vec3d cp;
    Vec4d sample_p;

    double R = 1e10;
    int steps = 0;
    int fi = -1;
    while (R > eps && steps < maxSteps) {
      sample_p << mp(0), mp(1), mp(2), 1.;
      double distance = closest_point_on_cage(mp, cp, fi);
      R = distance;
      Vec3d dir = generateRandomDirection();
      mp = mp + dir * R;
      steps++;
    }
    M[i] += sample_p * sample_p.transpose();
    if (face_type[fi] == 0) {
      Vec3d bary = get_tri_barycentric(cp, fi);
      for (int j = 0; j < 3; j++) {
        int idx = tri_faces(face_idx[fi], j);
        m[i * n_cage_verts + idx] += bary(j) * sample_p;
      }
    } else {
      Vec4d bary = get_quad_barycentric(cp, fi);
      Vec4d testv;
      Vec4d testcv;
      testv.setZero();
      for (int j = 0; j < 4; j++) {
        int idx = quad_faces(face_idx[fi], j);
        m[i * n_cage_verts + idx] += bary(j) * sample_p;

        testcv << cage_verts(idx, 0), cage_verts(idx, 1), cage_verts(idx, 2),
            1.;
        testv += bary(j) * testcv;
      }
    }
  }
}

void StoWarpSolver::walk_on_sphere(int maxSteps, double eps, int n_walks) {
  for (int i = 0; i < n_walks; i++) {
    walk_on_sphere_single_step(maxSteps, eps);
  }

  // for (auto& Mi : M) Mi = Mi / n_walks;
  // for (auto& mi : m) mi = mi / n_walks;

  for (int i = 0; i < n_mesh_verts; i++) {
    Mat4d invM = M[i].inverse();
    Vec4d p;
    p << mesh_verts(i, 0), mesh_verts(i, 1), mesh_verts(i, 2), 1.;
    for (int j = 0; j < n_cage_verts; j++) {
      harmonic_weights(i, j) = p.transpose() * invM * m[i * n_cage_verts + j];
    }
  }
}

}  // namespace StWarp
