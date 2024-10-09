#include <stdio.h>

#include <maya/MFnMesh.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnDagNode.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnGeometryFilter.h>
#include <maya/MFnWeightGeometryFilter.h>
#include <maya/MItGeometry.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MPxCommand.h>
#include <maya/MDagPath.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnSet.h>
#include <maya/MDagModifier.h>
#include <maya/MArgList.h>

#include <sstream>

#include "StWarp/type.h"
#include "StWarp/solver.h"
#include "StWarp/deform_node.h"
// #include "StWarp/st_deformer.h"

class StochasticWarp : public MPxCommand {
 public:
  static const char* kName;
  MSelectionList selectionList;
  MStatus doIt(const MArgList& args);

  static void* creator();
};

const char* StochasticWarp::kName = "StochasticWarp";

MStatus StochasticWarp::doIt(const MArgList& args) {
  MStatus status;

  MGlobal::displayInfo("Starting Stochastic Coordinates computing.");

  MGlobal::getActiveSelectionList(selectionList, true);

  if (selectionList.length() != 2) {
    MGlobal::displayError("Please select exactly two mesh objects.");
    return MS::kFailure;
  }

  MDagPath originMeshDagPath, cageMeshDagPath;
  MObject component;
  selectionList.getDagPath(0, originMeshDagPath, component);
  MFnMesh meshFn(originMeshDagPath, &status);
  if (status != MS::kSuccess) {
    MString errorMsg = originMeshDagPath.fullPathName() + " is not a mesh.";
    MGlobal::displayError(errorMsg);
    return MS::kFailure;
  }
  selectionList.getDagPath(1, cageMeshDagPath, component);
  MFnMesh cageFn(cageMeshDagPath, &status);
  if (status != MS::kSuccess) {
    MString errorMsg = cageMeshDagPath.fullPathName() + " is not a mesh.";
    MGlobal::displayError(errorMsg);
    return MS::kFailure;
  }

  StWarp::StoWarpSolver solver(cageFn, meshFn);

  // 100: number of max steps, 100 should be enough
  // 1e-6: define how close the sample point should be to the cage
  // 200: number of walks, more walks will give better results
  int n_walks = 200;
  if (args.length() > 0) {
    n_walks = args.asInt(0);
    if (status != MS::kSuccess) {
      MGlobal::displayError(
          "Invalid argument for number of walks. Using default value 200.");
      n_walks = 200;
    }
  }

  solver.walk_on_sphere(100, 1e-6, n_walks);

  if (solver.status != MS::kSuccess) {
    MGlobal::displayError("Failed to initialize solver.");
    return MS::kFailure;
  }
  MGlobal::displayInfo("Walk on sphere solver complete.");

  //! bind the mesh
  MString deformerCmd = "deformer -type \"myTypedDeformer\" ";
  MString targetMeshName = originMeshDagPath.fullPathName();
  deformerCmd += "\"" + targetMeshName + "\"";
  MStringArray deformerResult;
  status = MGlobal::executeCommand(deformerCmd, deformerResult);
  if (status != MS::kSuccess || deformerResult.length() == 0) {
    MGlobal::displayError("Failed to create myTypedDeformer node.");
    return status;
  }

  MString deformerNodeName = deformerResult[0];

  MSelectionList deformerSelection;
  deformerSelection.add(deformerNodeName);
  MObject deformerNodeObj;
  deformerSelection.getDependNode(0, deformerNodeObj);

  MFnDependencyNode deformerFn(deformerNodeObj);

  MPlug deformerInputMeshPlug = deformerFn.findPlug("cageMesh", &status);
  if (status != MS::kSuccess) {
    MGlobal::displayError("Failed to find inputMesh plug on deformer.");
    return status;
  }

  MPlug cageMeshWorldMeshPlug =
      cageFn.findPlug("worldMesh", &status).elementByLogicalIndex(0);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MFnDagNode cageMeshFn(cageFn.dagPath(), &status);
  MPlug cageMeshOutMeshPlug = cageMeshFn.findPlug("outMesh", &status);
  if (status != MS::kSuccess) {
    MGlobal::displayError("Failed to find worldMesh plug on cage mesh.");
    return status;
  }

  MDGModifier dgModifier;
  status = dgModifier.connect(cageMeshWorldMeshPlug, deformerInputMeshPlug);
  if (status != MS::kSuccess) {
    MGlobal::displayError("Failed to connect cage mesh to deformer.");
    return status;
  }

  status = dgModifier.doIt();
  if (status != MS::kSuccess) {
    MGlobal::displayError("Failed to execute DG modifier.");
    return status;
  }

  MFnDoubleArrayData weightsDataFn;
  MObject weightsDataObj =
      weightsDataFn.create(solver.harmonic_weights_maya, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MPlug deformerWeightsPlug = deformerFn.findPlug("stweights", &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  // iterate over weightsDataFn and print values
  status = deformerWeightsPlug.setValue(weightsDataObj);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MGlobal::displayInfo("Cage deformer created.");

  return MS::kSuccess;
}

void* StochasticWarp::creator() { return new StochasticWarp; }

MStatus initializePlugin(MObject obj) {
  MStatus status;
  MFnPlugin plugin(obj, "YourName", "1.0", "Any");

  status =
      plugin.registerCommand(StochasticWarp::kName, StochasticWarp::creator);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  status = plugin.registerNode(
      "myTypedDeformer", MyTypedDeformer::id, MyTypedDeformer::creator,
      MyTypedDeformer::initialize, MPxNode::kDeformerNode);
  if (!status) {
    status.perror("registerNode");
  }
  if (!status) {
    status.perror("registerNode");
  }

  return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj) {
  MStatus status;
  MFnPlugin plugin(obj);

  status = plugin.deregisterCommand(StochasticWarp::kName);
  if (!status) {
    status.perror("deregisterCommand");
    return status;
  }
  CHECK_MSTATUS_AND_RETURN_IT(status);

  status = plugin.deregisterNode(MyTypedDeformer::id);
  if (!status) {
    status.perror("deregisterNode");
  }
  CHECK_MSTATUS_AND_RETURN_IT(status);

  // status = plugin.deregisterNode(StochasticDeformer::id);
  // if (!status) {
  //   status.perror("deregisterNode");
  //   return status;
  // }

  return MS::kSuccess;
}
