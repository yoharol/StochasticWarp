#include "st_deformer.h"
#include <maya/MFnMesh.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MItGeometry.h>
#include <maya/MGlobal.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MDoubleArray.h>

MTypeId StochasticDeformer::id(0x00122700);  // Replace with your unique ID

MObject StochasticDeformer::aCageMesh;
MObject StochasticDeformer::aWeights;

StochasticDeformer::StochasticDeformer() {}
StochasticDeformer::~StochasticDeformer() {}

void* StochasticDeformer::creator() { return new StochasticDeformer(); }

MStatus StochasticDeformer::initialize() {
  MStatus status;

  MFnTypedAttribute tAttr;
  aCageMesh = tAttr.create("cageMesh", "cage", MFnData::kMesh,
                           MObject::kNullObj, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  tAttr.setStorable(true);
  tAttr.setReadable(false);
  tAttr.setWritable(true);
  tAttr.setDisconnectBehavior(MFnAttribute::kReset);

  aWeights = tAttr.create("weights", "wts", MFnData::kDoubleArray,
                          MObject::kNullObj, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  tAttr.setStorable(true);
  tAttr.setReadable(false);
  tAttr.setWritable(true);

  status = addAttribute(aCageMesh);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  status = addAttribute(aWeights);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  status = attributeAffects(aCageMesh, outputGeom);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  status = attributeAffects(aWeights, outputGeom);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  return MS::kSuccess;
}

MStatus StochasticDeformer::deform(MDataBlock& data, MItGeometry& itGeo,
                                   const MMatrix& localToWorldMatrix,
                                   unsigned int geomIndex) {
  MStatus status;

  float env = data.inputValue(envelope).asFloat();
  if (env == 0.0f) return MS::kSuccess;

  MDataHandle cageMeshDataHandle = data.inputValue(aCageMesh, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MObject cageMeshObj = cageMeshDataHandle.asMesh();
  if (cageMeshObj.isNull()) {
    MGlobal::displayError("Cage mesh is not connected.");
    return MS::kFailure;
  }
  MFnMesh cageMeshFn(cageMeshObj, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MPointArray cagePoints;
  status = cageMeshFn.getPoints(cagePoints, MSpace::kWorld);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  unsigned int numVertices = itGeo.count(&status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  Eigen::MatrixXd weights = getWeights(data, numVertices);

  if (weights.rows() != numVertices || weights.cols() != cagePoints.length()) {
    MGlobal::displayError("Weights matrix dimensions do not match geometry.");
    return MS::kFailure;
  }

  for (; !itGeo.isDone(); itGeo.next()) {
    int idx = itGeo.index();
    MPoint origPoint = itGeo.position(MSpace::kWorld);

    MPoint newPoint(0.0, 0.0, 0.0);
    for (unsigned int j = 0; j < cagePoints.length(); ++j) {
      double w = weights(idx, j);
      newPoint += cagePoints[j] * w;
    }

    MPoint finalPoint = origPoint + (newPoint - origPoint) * env;

    itGeo.setPosition(finalPoint, MSpace::kWorld);
  }

  return MS::kSuccess;
}

Eigen::MatrixXd StochasticDeformer::getWeights(MDataBlock& data,
                                               unsigned int numVertices) {
  MStatus status;

  MDataHandle weightsDataHandle = data.inputValue(aWeights, &status);
  CHECK_MSTATUS_AND_RETURN(status, Eigen::MatrixXd());

  MObject weightsDataObj = weightsDataHandle.data();
  MFnDoubleArrayData weightsArrayData(weightsDataObj, &status);
  CHECK_MSTATUS_AND_RETURN(status, Eigen::MatrixXd());

  MDoubleArray weightsArray = weightsArrayData.array(&status);
  CHECK_MSTATUS_AND_RETURN(status, Eigen::MatrixXd());

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
