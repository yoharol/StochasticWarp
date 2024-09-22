#ifndef STWARP_TYPE_H_
#define STWARP_TYPE_H_

#include <Eigen/Core>
#include <Eigen/SparseCore>

namespace StWarp {

using DiagMatxXd = Eigen::DiagonalMatrix<double, Eigen::Dynamic>;
using Matx2f = Eigen::Matrix<float, Eigen::Dynamic, 2, Eigen::RowMajor>;
using Matx3f = Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>;
using MatxXf =
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using Matx2d = Eigen::Matrix<double, Eigen::Dynamic, 2, Eigen::RowMajor>;
using Matx3d = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;
using MatxXd =
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using Matx2i = Eigen::Matrix<int, Eigen::Dynamic, 2, Eigen::RowMajor>;
using Matx3i = Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>;
using Matx4i = Eigen::Matrix<int, Eigen::Dynamic, 4, Eigen::RowMajor>;
using MatxXi =
    Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using Vecxf = Eigen::VectorXf;
using Vec4f = Eigen::Vector4f;
using Vec3f = Eigen::Vector3f;
using Vec2f = Eigen::Vector2f;
using RowVecxf = Eigen::RowVectorXf;
using RowVec3f = Eigen::RowVector3f;
using RowVec2f = Eigen::RowVector2f;
using Vecxd = Eigen::VectorXd;
using Vec4d = Eigen::Vector4d;
using Vec3d = Eigen::Vector3d;
using Vec2d = Eigen::Vector2d;
using RowVecxd = Eigen::RowVectorXd;
using RowVec3d = Eigen::RowVector3d;
using RowVec2d = Eigen::RowVector2d;
using Vecxi = Eigen::VectorXi;
using Vec3i = Eigen::Vector3i;
using Vec4i = Eigen::Vector4i;
using Vec2i = Eigen::Vector2i;
using RowVec3i = Eigen::RowVector3i;
using RowVec2i = Eigen::RowVector2i;
using Mat2f = Eigen::Matrix2f;
using Mat3f = Eigen::Matrix3f;
using Mat4f = Eigen::Matrix4f;
using Mat2d = Eigen::Matrix2d;
using Mat3d = Eigen::Matrix3d;
using Mat4d = Eigen::Matrix4d;
using RGB = Eigen::Vector3i;
using Listx3f = std::vector<Vec3f>;
using Listx2f = std::vector<Vec2f>;
using Listx3d = std::vector<Vec3d>;
using Listx2d = std::vector<Vec2d>;
using SparseMatf = Eigen::SparseMatrix<float>;
using SparseMatd = Eigen::SparseMatrix<double>;
using SparseVecd = Eigen::SparseVector<double>;
using Tripletf = Eigen::Triplet<float>;
using Tripletd = Eigen::Triplet<double>;

}  // namespace StWarp

#endif  // STWARP_TYPE_H_
