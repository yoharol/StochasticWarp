#ifndef STWARP_CAGEDEFORMER_H
#define STWARP_CAGEDEFORMER_H

#include <maya/MPxDeformerNode.h>
#include <maya/MTypeId.h>
#include <maya/MObject.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <Eigen/Dense>

class CageDeformer : public MPxDeformerNode {
 public:
  CageDeformer();
  virtual ~CageDeformer();

  static void* creator();
  static MStatus initialize();

  // The main deformation method
  virtual MStatus deform(MDataBlock& data, MItGeometry& itGeo,
                         const MMatrix& localToWorldMatrix,
                         unsigned int geomIndex);

  // Unique ID for the node
  static MTypeId id;

  // Attribute for the cage mesh
  static MObject aCageMesh;

  // Attribute for the weights
  static MObject aWeights;

 private:
  // Helper function to retrieve the weights matrix
  Eigen::MatrixXd getWeights(MDataBlock& data, unsigned int numVertices);
};

#endif  // STWARP_CAGEDEFORMER_H
