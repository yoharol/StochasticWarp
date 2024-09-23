#include <stdio.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MPxCommand.h>
#include <maya/MIOStream.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>

#include <sstream>

#include "StWarp/type.h"
#include "StWarp/solver.h"

// MPxCommand: Base class for user defined commands.
class StochasticWarp : public MPxCommand {
 public:
  static const char* kName;
  MSelectionList selectionList;
  // doIt: This method is called when the command is executed.
  MStatus doIt(const MArgList& args);

  // creator: This method creates an instance of the command.
  static void* creator();
};

const char* StochasticWarp::kName = "StochasticWarp";

MStatus StochasticWarp::doIt(const MArgList& args) {
  MStatus status;

  // Get the active selection list
  MGlobal::getActiveSelectionList(selectionList, true);

  // Check if exactly two objects are selected
  if (selectionList.length() != 2) {
    MGlobal::displayError("Please select exactly two mesh objects.");
    return MS::kFailure;
  }

  MDagPath dagPath;
  MObject component;
  selectionList.getDagPath(0, dagPath, component);
  MFnMesh meshFn(dagPath, &status);
  if (status != MS::kSuccess) {
    MString errorMsg = dagPath.fullPathName() + " is not a mesh.";
    MGlobal::displayError(errorMsg);
    return MS::kFailure;
  }
  selectionList.getDagPath(1, dagPath, component);
  MFnMesh cageFn(dagPath, &status);
  if (status != MS::kSuccess) {
    MString errorMsg = dagPath.fullPathName() + " is not a mesh.";
    MGlobal::displayError(errorMsg);
    return MS::kFailure;
  }

  StWarp::StoWarpSolver solver(cageFn, meshFn);

  if (solver.status != MS::kSuccess) {
    MGlobal::displayError("Failed to initialize solver.");
    return MS::kFailure;
  }

  /*for (int i = 0; i < solver.n_mesh_verts; i++) {
    std::stringstream ss;
    ss << "Vertex " << i << ": " << solver.mesh_verts(i, 0) << " "
       << solver.mesh_verts(i, 1) << " " << solver.mesh_verts(i, 2) << ": ";
    MPoint p(solver.mesh_verts(i, 0), solver.mesh_verts(i, 1),
             solver.mesh_verts(i, 2));
    MPoint closestPoint;
    int faceId;
    status = cageFn.getClosestPoint(p, closestPoint, MSpace::kWorld, &faceId);
    ss << "Closest point: " << closestPoint.x << " " << closestPoint.y << " "
       << closestPoint.z << " Face ID: " << faceId;
    meshFn.setPoint(i, closestPoint, MSpace::kWorld);
    MGlobal::displayInfo(ss.str().c_str());
  }*/

  return MS::kSuccess;
}

void* StochasticWarp::creator() { return new StochasticWarp; }

MStatus initializePlugin(MObject obj) {
  // MFnPlugin: Provides methods for registering and deregistering plug-ins.
  MFnPlugin plugin(obj, "Autodesk", "1.0", "Any");
  plugin.registerCommand(StochasticWarp::kName, StochasticWarp::creator);
  return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj) {
  MFnPlugin plugin(obj);
  plugin.deregisterCommand(StochasticWarp::kName);
  return MS::kSuccess;
}
