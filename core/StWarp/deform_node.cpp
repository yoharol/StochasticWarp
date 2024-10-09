#include "StWarp/deform_node.h"

#include <maya/MFnMesh.h>
#include <maya/MItGeometry.h>
#include <maya/MPlug.h>
#include <maya/MDataHandle.h>
#include <maya/MMatrix.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MDoubleArray.h>
#include <maya/MGlobal.h>

#include <sstream>

MTypeId MyTypedDeformer::id(0x0011FFAC);  // Replace with a unique ID
MObject MyTypedDeformer::aCageMesh;
MObject MyTypedDeformer::aStWeights;

void* MyTypedDeformer::creator() { return new MyTypedDeformer(); }

MStatus MyTypedDeformer::initialize() {
  MFnTypedAttribute tAttr;

  // Create a typed attribute to accept a mesh input
  aCageMesh = tAttr.create("cageMesh", "cMesh", MFnData::kMesh);
  tAttr.setStorable(true);
  tAttr.setReadable(true);
  tAttr.setWritable(true);

  // Add the attribute to the node
  addAttribute(aCageMesh);

  // Make the deformer affect geometry when inputMesh is modified
  attributeAffects(aCageMesh, outputGeom);

  MFnTypedAttribute wtAttr;

  aStWeights = wtAttr.create("stweights", "stws", MFnData::kDoubleArray);
  wtAttr.setStorable(true);
  wtAttr.setReadable(true);
  wtAttr.setWritable(true);

  addAttribute(aStWeights);

  return MS::kSuccess;
}

MStatus MyTypedDeformer::deform(MDataBlock& dataBlock, MItGeometry& iter,
                                const MMatrix& localToWorldMatrix,
                                unsigned int multiIndex) {
  MStatus status = MS::kSuccess;

  MDataHandle inputCageHandle = dataBlock.inputValue(aCageMesh);
  MObject inputCage = inputCageHandle.asMesh();
  if (inputCage.isNull()) {
    return MS::kFailure;
  }
  MFnMesh fnMesh(inputCage);
  MPointArray cagePoints;
  fnMesh.getPoints(cagePoints, MSpace::kWorld);
  MDataHandle weightsDataHandle = dataBlock.inputValue(aStWeights);
  MObject weightsObj = weightsDataHandle.data();
  MFnDoubleArrayData weightsArrayData(weightsObj);
  MDoubleArray weightsArray = weightsArrayData.array();

  unsigned int cage_points_count = cagePoints.length();

  MDataHandle envData = dataBlock.inputValue(envelope, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  float env = envData.asFloat();

  for (; !iter.isDone(); iter.next()) {
    MPoint pt = iter.position();

    int idx = iter.index();
    MPoint orinPoint = iter.position(MSpace::kWorld);

    MPoint interp(0, 0, 0);

    for (int i = 0; i < cage_points_count; i++) {
      MPoint cagePoint = cagePoints[i];
      double weight = weightsArray[idx * cage_points_count + i];
      interp += cagePoint * weight;
    }

    MPoint final_point = orinPoint + (interp - orinPoint) * env;
    iter.setPosition(final_point);
  }

  return MS::kSuccess;
}