#ifndef STWARP_DEFORM_NODE_H
#define STWARP_DEFORM_NODE_H

#include <maya/MPxDeformerNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MPointArray.h>
#include <maya/MTypeId.h>

class MyTypedDeformer : public MPxDeformerNode {
 public:
  MyTypedDeformer() {}
  virtual ~MyTypedDeformer() override {}

  static void* creator();
  static MStatus initialize();

  virtual MStatus deform(MDataBlock& dataBlock, MItGeometry& iter,
                         const MMatrix& localToWorldMatrix,
                         unsigned int multiIndex) override;

  static MTypeId id;          // Unique Node ID
  static MObject aCageMesh;   // Mesh attribute for cage
  static MObject aStWeights;  // Weights attribute for cage
};

#endif  // STWARP_DEFORM_NODE_H
