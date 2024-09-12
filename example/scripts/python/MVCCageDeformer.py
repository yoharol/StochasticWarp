import maya.cmds as cmd
    
def olmMVCDeformer(cage, targets, local = 0, split = 0):
    '''
    Create mean value coordinates deformer(s)
    Arguments:
        cage    - [string]  cage mesh (must be closed)
        targets - [list]    list of target geometries (could be components)
        local   - [boolean] whether local bind or not (not by default)
        split   - [boolean] whether create deformers for each geometry (not by default)
    '''
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
    # create the deformer node(s)
    if(split):
        for comp in component_dict.values():
            defms = cmd.deformer(comp, type='olmMVC')
            defm = defms[0]
            results.append(defm)
            cmd.setAttr(defm+".localBind", local)
            cmd.connectAttr(cageBase+".worldMesh", defm+".cageBase")
            cmd.connectAttr(cageName+".worldMatrix", defm+".cageWorldMatrix")
            cmd.connectAttr(cageName+".worldMesh", defm+".cageMesh")
    else:
        defms = cmd.deformer(targets, type = 'olmMVC')
        defm = defms[0]
        results.append(defm)
        cmd.setAttr(defm+".localBind", local)
        cmd.connectAttr(cageBase+".worldMesh", defm+".cageBase")
        cmd.connectAttr(cageName+".worldMatrix", defm+".cageWorldMatrix")
        cmd.connectAttr(cageName+".worldMesh", defm+".cageMesh")
    return results
