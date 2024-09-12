from __future__ import print_function
import maya.mel as mel
import maya.cmds as cmds
import olm.ui
from StarCageDeformer2 import createFromUI

def createStarCageDeformer(cage, targets, edgeSets, local, split):
	createFromUI(cage, targets, edgeSets, local, split)


class StarCageDeformerUI():
	def __init__(self):
		self.name = "OLMStarCageDeformer"
		self.title = "OLM Star Cage Deformer"
		self.iconName = self.title
		self.defaultWidth= 380
		self.defaultHeight= 300

		# Begin creating the UI
		if (cmds.window(self.name, q=1, exists=1)): cmds.deleteUI(self.name)
		self.window = cmds.window(self.name,
						title=self.title,
						iconName= self.iconName,
						width = self.defaultWidth,
						height= self.defaultHeight,
						resizeToFitChildren=True,
						maximizeButton=False,
						sizeable=True)
		self.columnLayout = cmds.columnLayout(parent=self.window, adjustableColumn=True)

		self.frameLayout = cmds.frameLayout(label = "Object, Sets and Cage", width= 350)
		cmds.columnLayout()
		self.cageTextField = cmds.textFieldButtonGrp(label="Cage", buttonLabel="Set", columnWidth3=[50, 200, 100], buttonCommand=olm.ui.Callback(self.olm_cb_cageDeformer_setCage))
		
		cmds.rowLayout(numberOfColumns=2)
		cmds.text(label="Objects", width=50, align="right")
		self.objectListSL = cmds.textScrollList(width=200, allowMultiSelection=True)
		cmds.setParent("..")
		cmds.rowLayout(numberOfColumns=3)
		cmds.separator(width=50, style="none")
		cmds.button(label="Add", command=olm.ui.Callback(self.olm_cb_cageDeformer_addObjects))
		cmds.button(label="Remove", command=olm.ui.Callback(self.olm_cb_cageDeformer_removeObjects))
		cmds.setParent("..")
		cmds.setParent("..")

		cmds.rowLayout(numberOfColumns=2)
		cmds.text(label="Sets", width=50, align="right")
		self.setListSL = cmds.textScrollList(width=200, allowMultiSelection=True)
		cmds.setParent("..")
		cmds.rowLayout(numberOfColumns=3)
		cmds.separator(width=50, style="none")
		cmds.button(label="Add", command=olm.ui.Callback(self.olm_cb_cageDeformer_addSets))
		cmds.button(label="Remove", command=olm.ui.Callback(self.olm_cb_cageDeformer_removeSets))
		cmds.setParent("..")
		cmds.setParent("..")

		cmds.rowLayout(numberOfColumns=3)
		cmds.separator(width=50, style="none")
		self.localBindCB = cmds.checkBox(label="Local bind")
		self.oneDeformerPerShapeCB = cmds.checkBox(label="One deformer per shape")
		cmds.setParent("..")
		cmds.rowLayout(numberOfColumns=3)
		cmds.button(label="Create and close", width=100, command=olm.ui.Callback(self.olm_cb_cageDerformer_ok))
		cmds.button(label="Create", width=75, command=olm.ui.Callback(self.olm_cb_cageDerformer_create))
		cmds.button(label="Close", width=75, command=olm.ui.Callback(self.olm_cb_cageDeformer_close))
		cmds.setParent("..")
		cmds.setParent("..")
		cmds.showWindow(self.window)

	def olm_cb_cageDeformer_setCage(self):
		objs = cmds.ls(selection=True)
		if len(objs)>0:
			cmds.textFieldButtonGrp(self.cageTextField, edit=True, text=objs[0])

	def olm_cb_cageDeformer_addObjects(self):
		objs = cmds.ls(selection=True)
		currentList = cmds.textScrollList(self.objectListSL, query=True, allItems=True)
		currentCage = cmds.textFieldButtonGrp(self.cageTextField, query=True, text=True)
		if currentList==None:
			currentList=[]
		for o in objs:
			if not o in currentList and o != currentCage:
				cmds.textScrollList(self.objectListSL,edit=True,append=o)

	def olm_cb_cageDeformer_removeObjects(self):
		currentList = cmds.textScrollList(self.objectListSL,query=True, selectItem=True)
		if currentList==None:
			currentList=[]
		for c in currentList:
			cmds.textScrollList(self.objectListSL,edit=True,removeItem=c)

	def performCageDeformer(self):
		targets = cmds.textScrollList(self.objectListSL, query=True, allItems=True)
		if targets==None:
			targets=[]
		realTargets = [t for t in targets if cmds.objExists(t)==True]

		cage = cmds.textFieldButtonGrp(self.cageTextField, query=True, text=True)

		edgeSets = cmds.textScrollList(self.setListSL, query=True, allItems=True)
		if edgeSets==None:
			edgeSets=[]
		realSets = [t for t in edgeSets if cmds.objExists(t)==True]

		localBind = cmds.checkBox(self.localBindCB, query=True, value=True)
		oneDeformerPerShape = cmds.checkBox(self.oneDeformerPerShapeCB, query=True, value=True)

		if len(realTargets)>0 and cmds.objExists(cage):
			createStarCageDeformer(cage, realTargets, realSets, localBind, oneDeformerPerShape)
		else:
			print("no valid cage or targets")

	def olm_cb_cageDeformer_addSets(self):
		objs = cmds.ls(selection=True)
		currentList = cmds.textScrollList(self.setListSL, query=True, allItems=True)
		currentCage = cmds.textFieldButtonGrp(self.cageTextField, query=True, text=True)
		if currentList==None:
			currentList=[]
		for o in objs:
			if not o in currentList and o != currentCage:
				cmds.textScrollList(self.setListSL,edit=True,append=o)

	def olm_cb_cageDeformer_removeSets(self):
		currentList = cmds.textScrollList(self.setListSL,query=True, selectItem=True)
		if currentList==None:
			currentList=[]
		for c in currentList:
			cmds.textScrollList(self.setListSL,edit=True,removeItem=c)

	def olm_cb_cageDerformer_ok(self):
		self.performCageDeformer()
		cmds.deleteUI(self.window)
	def olm_cb_cageDerformer_create(self):
		self.performCageDeformer()
	def olm_cb_cageDeformer_close(self):
		cmds.deleteUI(self.window)

#MVCCageDeformerUI()
