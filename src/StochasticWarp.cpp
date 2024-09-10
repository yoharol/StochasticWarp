#include <stdio.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>

// MPxCommand: Base class for user defined commands.
class StochasticWarp : public MPxCommand
{
    public:
        static const char* kName;
        // doIt: This method is called when the command is executed.
        MStatus doIt( const MArgList& args );

        // creator: This method creates an instance of the command.
        static void* creator();
};

const char* StochasticWarp::kName = "StochasticWarp";

MStatus StochasticWarp::doIt( const MArgList& args ) {
    MGlobal::displayInfo("Hello World");
    return MS::kSuccess;
}

void* StochasticWarp::creator() {
    return new StochasticWarp;
}

MStatus initializePlugin( MObject obj ) {
    // MFnPlugin: Provides methods for registering and deregistering plug-ins.
    MFnPlugin plugin( obj, "Autodesk", "1.0", "Any" );
    plugin.registerCommand( StochasticWarp::kName, StochasticWarp::creator );
    return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj ) {
    MFnPlugin plugin( obj );
    plugin.deregisterCommand( StochasticWarp::kName );
    return MS::kSuccess;
}
