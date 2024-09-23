#ifndef STWARP_CAGEDEFORMER_H
#define STWARP_CAGEDEFORMER_H

#include <maya/MPxDeformerNode.h>
#include <maya/MTypeId.h>
#include <maya/MObject.h>
#include <maya/MPointArray.h>
#include <Eigen/Dense>

class StochasticDeformer : public MPxDeformerNode {
 public:
  StochasticDeformer();
  virtual ~StochasticDeformer();

  static void* creator();
  static MStatus initialize();

  virtual MStatus deform(MDataBlock& data, MItGeometry& itGeo,
                         const MMatrix& localToWorldMatrix,
                         unsigned int geomIndex);

  static MTypeId id;

  static MObject aCageMesh;
  static MObject aWeights;

 private:
  Eigen::MatrixXd getWeights(MDataBlock& data, unsigned int numVertices);
};

#endif  // STWARP_CAGEDEFORMER_H
