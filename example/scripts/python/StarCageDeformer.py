import re
import maya.cmds as cmd
import maya.OpenMaya as OpenMaya

def updateCutStarCage_olm(attr, index):
    nodeName = re.split('\.', attr)[0]
    cage = cmd.listConnections(
        nodeName + '.cageMesh',
        source=True,
        destination=False
        )[0]

    # get current edge indices 
    currentEdgeIndices = cmd.getAttr(attr + "[" + str(index) + "]")
    currentEdgeSet = set()
    if currentEdgeIndices:
        currentEdgeSet = set(currentEdgeIndices)

    selectedEdges = cmd.ls(flatten=True, selection=True)
    if selectedEdges:
        edgeSet = set()
        for selectedEdge in selectedEdges:
            splits = re.split('\[|\]|\.', selectedEdge)
            objectName = splits[0]
            if objectName == cage and len(splits) > 2:
                edgeIndex = splits[2]
                edgeSet.add(int(edgeIndex))
        if len(edgeSet) > 0 and edgeSet != currentEdgeSet:
            edgeList = list(edgeSet)
            cmd.setAttr(attr + "[" + str(index) + "]", edgeList, type='Int32Array')

def highlightCutStarCage_olm(attr, index):
    nodeName = re.split('\.', attr)[0]
    cages = cmd.listConnections(
        nodeName + '.cageMesh',
        source=True,
        destination=False,
        shapes=True
        )
    if len(cages) > 0:
        cage = cages[0]
        edgeIndices = cmd.getAttr(attr + "[" + str(index) + "]")
        cmd.select( clear=True )
        if edgeIndices:
            for edgeIndex in edgeIndices:
                cmd.select(cage + '.e[' + str(edgeIndex) + ']', add=True)

def createFromUI(cage, targets, edgeSets, local, split):
    cageBase = ""
    cageName = cage
    results = []
    component_dict = {}
    # create cage base
    if cmd.nodeType(cage) != 'transform':
        if cmd.nodeType(cage) == 'mesh':
            parents = cmd.listRelatives( cage, parent=True )
            parent = parents[0]
            if cmd.nodeType(parent) != 'transform':
                cmd.error("wrong node type of the given cage")
            cageName = parent
            cageBase = cageName + "Base"
        else:
            cmd.error("wrong node type of the given cage")
    else:
        cageBase = cage+"Base"
    bases = cmd.duplicate(cageName, n = cageBase)
    cageBase = bases[0]
    cmd.setAttr(cageBase+".visibility", False)
    results.append(cageBase)
    # find target objects
    for target in targets:
        segs = target.split('.')
        if len(segs)>1:
            if segs[0] not in component_dict:
                component_dict[segs[0]] = []
            component_dict[segs[0]].append(target)
        else:
            if segs[0] not in component_dict:
                component_dict[segs[0]] = segs

    if(split):
        for comp in component_dict.values():
            defms = cmd.deformer(comp, type='olmStarCage2')
            defm = defms[0]
            results.append(defm)
            cmd.setAttr(defm+".localBind", local)
            cmd.connectAttr(cageBase+".worldMesh", defm+".cageBase")
            cmd.connectAttr(cageName+".worldMatrix", defm+".cageWorldMatrix")
            cmd.connectAttr(cageName+".worldMesh", defm+".cageMesh")
    else:
        defms = cmd.deformer(targets, type = 'olmStarCage2')
        defm = defms[0]
        results.append(defm)
        cmd.setAttr(defm+".localBind", local)
        cmd.connectAttr(cageBase+".worldMesh", defm+".cageBase")
        cmd.connectAttr(cageName+".worldMatrix", defm+".cageWorldMatrix")
        cmd.connectAttr(cageName+".worldMesh", defm+".cageMesh")
        for index, edgeSet in enumerate(edgeSets):
            cmd.connectAttr(edgeSet+".message", defm+".edgeSets[" + str(index) + "]")


def createFromSelection():
    selectionList = OpenMaya.MSelectionList()
    OpenMaya.MGlobal.getActiveSelectionList( selectionList )
    # path = OpenMaya.MDagPath()
    # for i in range(selectionList.length()):
    #     selectionList.getDagPath(i, path)
    #     path.extendToShape()
    if selectionList.length() < 2:
        cmd.error("Please select targets first and then finally a cage")
        return

    targets = []
    for i in range(selectionList.length() - 1):
        targetPath = OpenMaya.MDagPath()
        selectionList.getDagPath(i, targetPath)
        targetName = targetPath.fullPathName()
        targets.append(targetName)
    # cmd.makeIdentity(targetName, apply=True, t=1, r=1, s=1, n=2 )

    cagePath = OpenMaya.MDagPath()
    selectionList.getDagPath(selectionList.length() - 1, cagePath)

    cageBase = ""
    cageName = cagePath.fullPathName()
    # cmd.makeIdentity(cageName, apply=True, t=1, r=1, s=1, n=2 )

    # create cage base
    if cmd.nodeType(cageName) != 'transform':
        if cmd.nodeType(cageName) == 'mesh':
            parents = cmd.listRelatives( cageName, parent=True )
            parent = parents[0]
            if cmd.nodeType(parent) != 'transform':
                cmd.error("wrong node type of the given cage")
            cageName = parent
            cageBase = cageName + "Base"
        else:
            cmd.error("wrong node type of the given cage")
    else:
        cageBase = cageName+"Base"
    bases = cmd.duplicate(cageName, n = cageBase)
    cageBase = bases[0]
    cmd.setAttr(cageBase+".visibility", False)

    defms = cmd.deformer(targets, type = 'olmMultiCage')
    defm = defms[0]
    cmd.setAttr(defm+".localBind", True)
    cmd.connectAttr(cageBase+".worldMesh", defm+".cageBase")
    cmd.connectAttr(cageName+".worldMatrix", defm+".cageWorldMatrix")
    cmd.connectAttr(cageName+".worldMesh", defm+".cageMesh")


def create(cage, targets):
    if len(targets) < 1:
        cmd.error("At least one target")
        return

    if not cage:
        cmd.error("Need a cage")
        return

    cageBase = ""
    cageName = cage

    # create cage base
    if cmd.nodeType(cageName) != 'transform':
        if cmd.nodeType(cageName) == 'mesh':
            parents = cmd.listRelatives( cageName, parent=True )
            parent = parents[0]
            if cmd.nodeType(parent) != 'transform':
                cmd.error("wrong node type of the given cage")
            cageName = parent
            cageBase = cageName + "Base"
        else:
            cmd.error("wrong node type of the given cage")
    else:
        cageBase = cageName+"Base"
    bases = cmd.duplicate(cageName, n = cageBase)
    cageBase = bases[0]
    cmd.setAttr(cageBase+".visibility", False)

    defms = cmd.deformer(targets, type = 'olmMultiCage')
    defm = defms[0]
    cmd.setAttr(defm+".localBind", True)
    cmd.connectAttr(cageBase+".worldMesh", defm+".cageBase")
    cmd.connectAttr(cageName+".worldMatrix", defm+".cageWorldMatrix")
    cmd.connectAttr(cageName+".worldMesh", defm+".cageMesh")
    return defm


def forceUpdate(node):
    attr = node + '.dummy'
    val = cmd.getAttr(attr)
    cmd.setAttr(attr, 1-val)