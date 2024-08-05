import maya.cmds as cmds
import maya.OpenMaya as om
import maya.OpenMayaUI as omui

from PySide2 import QtWidgets
from PySide2.QtWidgets import *
from shiboken2 import wrapInstance

import json

def get_main_window():
    """this returns the maya main window for parenting"""
    window = omui.MQtUtil.mainWindow()
    return wrapInstance(int(window), QDialog)

class AttributeSelectionDialog(QtWidgets.QDialog):

    def __init__(self, parent=get_main_window()):
        super().__init__(parent)

        self.selected_objects_list = QListWidget()
        self.selected_objects_list.itemClicked.connect(self.update_attributes)

        self.selected_objects_layout = QVBoxLayout()
        self.selected_objects_layout.addWidget(QLabel("Selected objects:"))
        self.selected_objects_layout.addWidget(self.selected_objects_list)

        self.attributes_list = QListWidget()
        self.attributes_list.setSelectionMode(QListWidget.MultiSelection)

        self.user_defined_checkbox = QCheckBox("User defined attributes only")
        self.user_defined_checkbox.setChecked(True)
        self.create_usd_attr_button = QPushButton("Create")
        self.create_usd_attr_button.clicked.connect(self.create_usd_attributes)

        self.attributes_layout = QVBoxLayout()
        self.attributes_layout.addWidget(QLabel("Available attributes:"))
        self.attributes_layout.addWidget(self.user_defined_checkbox)
        self.attributes_layout.addWidget(self.attributes_list)
        self.attributes_layout.addWidget(self.create_usd_attr_button)

        self.selection_callback = om.MEventMessage.addEventCallback("SelectionChanged", self.update_selected_objects)
        self.update_selected_objects()

        layout = QHBoxLayout()
        layout.addLayout(self.selected_objects_layout)
        layout.addLayout(self.attributes_layout)
        self.setLayout(layout)


    def update_selected_objects(self, *args):
        self.selected_objects_list.clear()
        selected_objects = cmds.ls(selection=True)
        if selected_objects:
            for obj in selected_objects:
                self.selected_objects_list.addItem(obj)

    def update_attributes(self, item):
        self.attributes_list.clear()
        user_defined = self.user_defined_checkbox.isChecked()
        attributes = cmds.listAttr(item.text(), userDefined=user_defined)

        if attributes:
            for attr in attributes:
                if attr == "USD_UserExportedAttributesJson":
                    continue
                self.attributes_list.addItem(attr)

    def create_usd_attributes(self):
        selected_object = self.selected_objects_list.selectedItems()[0].text()        
        selected_attributes = [item.text() for item in self.attributes_list.selectedItems()]

        attr_json = {}
        usd_attr = "USD_UserExportedAttributesJson"
        if cmds.attributeQuery(usd_attr, node=selected_object, exists=True):
            cmds.deleteAttr(selected_object, attribute=usd_attr)

        for attr in selected_attributes:
            attr_value = cmds.getAttr(f"{selected_object}.{attr}")                
            attr_type = cmds.attributeQuery(attr, node=selected_object, attributeType=True)                

            attr_json[attr] = {"usdAttrName": attr, "usdAttrType": attr_type, "usdAttrValue" : attr_value}

        cmds.addAttr(selected_object, longName=usd_attr, dataType="string")
        cmds.setAttr(f"{selected_object}.{usd_attr}", json.dumps(attr_json), type="string")



if __name__== "__main__":
    try:
        attribute_selection_dialog.close()
        attribute_selection_dialog.deleteLater()
    except:
        pass

    attribute_selection_dialog = AttributeSelectionDialog()
    attribute_selection_dialog.show()