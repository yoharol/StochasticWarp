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

#include <sstream>

#include "StWarp/type.h"
#include "StWarp/solver.h"
#include "StWarp/st_deformer.h"

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
  solver.walk_on_sphere(100, 1e-6, 200);

  if (solver.status != MS::kSuccess) {
    MGlobal::displayError("Failed to initialize solver.");
    return MS::kFailure;
  }

  MFnDependencyNode deformerFn;
  MObject deformerObj = deformerFn.create("cageDeformer", &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MPlug usedByPlug = deformerFn.findPlug("usedBy", false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MPlug usedByElementPlug = usedByPlug.elementByLogicalIndex(0, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MPlugArray connectedPlugs;
  usedByElementPlug.connectedTo(connectedPlugs, true, false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  if (connectedPlugs.length() == 0) {
    MGlobal::displayError(
        "Failed to find deformer set connected to the deformer.");
    return MS::kFailure;
  }

  MObject deformerSetObj = connectedPlugs[0].node(&status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MFnSet deformerSetFn(deformerSetObj, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  status = deformerSetFn.addMember(originMeshDagPath);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MDGModifier dgMod;
  MPlug inputPlug = deformerFn.findPlug("input", false, &status)
                        .elementByLogicalIndex(0, &status)
                        .child(0, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MFnDagNode originMeshDagNode(originMeshDagPath, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MPlug outMeshPlug = originMeshDagNode.findPlug("outMesh", false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  status = dgMod.connect(outMeshPlug, inputPlug);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MPlug outputPlug = deformerFn.findPlug("outputGeometry", false, &status)
                         .elementByLogicalIndex(0, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MPlug inMeshPlug = originMeshDagNode.findPlug("inMesh", false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  status = dgMod.connect(outputPlug, inMeshPlug);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  status = dgMod.doIt();
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MFnDagNode cageMeshDagNode(cageMeshDagPath, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MPlug cageMeshPlug = deformerFn.findPlug("cageMesh", false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MPlug cageOutMeshPlug = cageMeshDagNode.findPlug("outMesh", false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  status = dgMod.connect(cageOutMeshPlug, cageMeshPlug);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  status = dgMod.doIt();
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MPlug weightsPlug = deformerFn.findPlug("weights", false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MDoubleArray weightsArray(solver.n_mesh_verts * solver.n_cage_verts);
  for (int i = 0; i < solver.n_mesh_verts; i++) {
    for (int j = 0; j < solver.n_cage_verts; j++) {
      weightsArray[i * solver.n_cage_verts + j] = solver.harmonic_weights(i, j);
    }
  }

  MFnDoubleArrayData weightsDataFn;
  MObject weightsDataObj = weightsDataFn.create(weightsArray, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  status = weightsPlug.setValue(weightsDataObj);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MGlobal::displayInfo("Stochastic deformation applied successfully.");

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
      "StochaticDeformer", StochasticDeformer::id, StochasticDeformer::creator,
      StochasticDeformer::initialize, MPxNode::kDeformerNode);
  CHECK_MSTATUS_AND_RETURN_IT(status);

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

  status = plugin.deregisterNode(StochasticDeformer::id);
  if (!status) {
    status.perror("deregisterNode");
    return status;
  }
  CHECK_MSTATUS_AND_RETURN_IT(status);

  return MS::kSuccess;
}
