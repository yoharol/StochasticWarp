#include "StWarp/cage_deformer.h"
#include <maya/MFnMesh.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MItGeometry.h>
#include <maya/MGlobal.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MDataHandle.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MDoubleArray.h>
#include <maya/MIntArray.h>

MTypeId CageDeformer::id(0x8702);  // Unique ID, replace with your own

MObject CageDeformer::aCageMesh;
MObject CageDeformer::aWeights;

CageDeformer::CageDeformer() {}
CageDeformer::~CageDeformer() {}

void* CageDeformer::creator() { return new CageDeformer(); }

MStatus CageDeformer::initialize() {
  MStatus status;

  // Create the cage mesh attribute
  MFnTypedAttribute tAttr;
  aCageMesh = tAttr.create("cageMesh", "cage", MFnData::kMesh);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  tAttr.setStorable(true);
  tAttr.setReadable(false);
  tAttr.setWritable(true);
  tAttr.setDisconnectBehavior(MFnAttribute::kReset);

  // Create the weights attribute (we'll use MFnDoubleArrayData for simplicity)
  MFnTypedAttribute weightsAttr;
  aWeights = weightsAttr.create("weights", "wts", MFnData::kDoubleArray,
                                MObject::kNullObj, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  weightsAttr.setStorable(true);
  weightsAttr.setReadable(false);
  weightsAttr.setWritable(true);

  // Add attributes to the node
  status = addAttribute(aCageMesh);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  status = addAttribute(aWeights);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  // Affect the output geometry when these attributes change
  status = attributeAffects(aCageMesh, outputGeom);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  status = attributeAffects(aWeights, outputGeom);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  return MS::kSuccess;
}

MStatus CageDeformer::deform(MDataBlock& data, MItGeometry& itGeo,
                             const MMatrix& localToWorldMatrix,
                             unsigned int geomIndex) {
  MStatus status;

  // Get the envelope
  float env = data.inputValue(envelope).asFloat();
  if (env == 0.0f) return MS::kSuccess;

  // Get the cage mesh
  MObject cageMeshObj = data.inputValue(aCageMesh, &status).asMesh();
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MFnMesh cageMeshFn(cageMeshObj, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  // Get cage mesh vertex positions
  MPointArray cagePoints;
  status = cageMeshFn.getPoints(cagePoints, MSpace::kWorld);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  // Get the weights matrix
  unsigned int numVertices = itGeo.count(&status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  Eigen::MatrixXd weights = getWeights(data, numVertices);

  if (weights.rows() != numVertices || weights.cols() != cagePoints.length()) {
    MGlobal::displayError("Weights matrix dimensions do not match geometry.");
    return MS::kFailure;
  }

  // Deform each point
  for (; !itGeo.isDone(); itGeo.next()) {
    int idx = itGeo.index();
    MPoint origPoint = itGeo.position(MSpace::kWorld);

    // Compute the new position
    MPoint newPoint(0.0, 0.0, 0.0);
    for (unsigned int j = 0; j < cagePoints.length(); ++j) {
      double w = weights(idx, j);
      newPoint += cagePoints[j] * w;
    }

    // Blend with the envelope
    MPoint finalPoint = origPoint + (newPoint - origPoint) * env;

    // Set the new position
    itGeo.setPosition(finalPoint, MSpace::kWorld);
  }

  return MS::kSuccess;
}

Eigen::MatrixXd CageDeformer::getWeights(MDataBlock& data,
                                         unsigned int numVertices) {
  MStatus status;

  // Get the weights attribute data
  MDataHandle weightsDataHandle = data.inputValue(aWeights, &status);
  CHECK_MSTATUS_AND_RETURN(status, Eigen::MatrixXd());

  MObject weightsDataObj = weightsDataHandle.data();
  MFnDoubleArrayData weightsArrayData(weightsDataObj, &status);
  CHECK_MSTATUS_AND_RETURN(status, Eigen::MatrixXd());

  MDoubleArray weightsArray = weightsArrayData.array(&status);
  CHECK_MSTATUS_AND_RETURN(status, Eigen::MatrixXd());

  // Assume that weights are stored row-major in the array
  // Total number of weights should be numVertices * cageVertexCount
  unsigned int totalWeights = weightsArray.length();
  unsigned int cageVertexCount = totalWeights / numVertices;

  Eigen::MatrixXd weights(numVertices, cageVertexCount);
  for (unsigned int i = 0; i < numVertices; ++i) {
    for (unsigned int j = 0; j < cageVertexCount; ++j) {
      weights(i, j) = weightsArray[i * cageVertexCount + j];
    }
  }

  return weights;
}
