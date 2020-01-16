#!/usr/bin/env python
# encoding: utf-8

__auther__ = "kevin tsui"

try:
    from Katana import os, QtGui, QtCore, QT4Widgets, QT4FormWidgets, QT4Color, Utils
    from Katana import NodegraphAPI
    import UI4
except:
    import os
    from PyQt4 import QtGui,QtCore

class kLineTextLabel(QtGui.QWidget):
    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)
        layout = QtGui.QHBoxLayout(self)
        self.label = QtGui.QLabel()
        self.label.setFixedWidth(150)
        self.label.setAlignment(QtCore.Qt.AlignRight)
        self.edited = QtGui.QLineEdit()
        layout.addWidget(self.label)
        layout.addWidget(self.edited)
    def setLabel(self, text):
        self.label.setText(text)
    def setText(self, text):
        self.edited.setText(text)
    def label(self):
        return self.label.text()
    def text(self):
        return self.edited.text()
_iconsDir = os.path.join(os.path.dirname(__file__), 'icons', "no-image.svg")
_workDir = os.path.join("lgt","task","nuke")
def shotInit():
    current_dir = NodegraphAPI.NodegraphGlobals.GetProjectDir()
    # current_dir = "/mnt/work/projects/yzc/shot/u60/u60480/lgt/task/katana"
    shot_folder = os.path.join(current_dir,"..", "..", "..", "..")
    if os.path.isdir(current_dir) and os.path.isdir(shot_folder):
        shot = current_dir.split("/")[-4]
        proj = current_dir.split("/")[-7]
        sequence = shot[:3]
        shot_list = sorted(os.listdir(shot_folder))
        return proj,sequence,shot,shot_list
    else:
        return None, None, None, []

def getOutputDir(proj, sequence, shot):
    output_dir = "/output/projects/%s/shot/%s/%s/lgt/output/nuke"%(
        proj, sequence, shot)
    return output_dir

def getImageList(shot):
    proj, sequence,curr_shot,shot_list = shotInit()
    output_dir = getOutputDir(proj, sequence, shot)

    def getImages(shot, quality):
        aa_dir = os.path.join(output_dir, quality)
        last_version = sorted(os.listdir(aa_dir))[-1]
        image_dir = os.path.join(aa_dir, last_version, "jpg_L")
        if os.path.isdir(image_dir):        
            image_list = sorted(os.listdir(image_dir))
            return [os.path.join(image_dir, f) for f in image_list]
        else:
            return []

    if os.path.isdir(output_dir):
        quality_list = os.listdir(output_dir)
    else:
        return []
    if "R1" in quality_list:
        image_list = getImages(shot, "R1")
    elif "R2" in quality_list:
        image_list = getImages(shot, "R2")
    elif "Rough" in quality_list:
        image_list = getImages(shot, "Rough")
    else:
        image_list = []

    return image_list

def getImageRange(shot):
    shot_images = getImageList(shot)
    if shot_images:
        first_image = shot_images[0]
        last_image = shot_images[-1]
        first_frame = first_image.split(".")[-2]
        last_frame = last_image.split(".")[-2]
        return (first_frame,last_frame)
    else:
        return (1001, 1001)

def doseImageExist(shot):
    image_list = getImageList(shot)

    if image_list is None:
        return False
    if not image_list:
        return False
    return True

def init(ptr):
    current_proj,current_sequence,current_shot,shot_list = shotInit()    
    if current_shot in shot_list:
        index = shot_list.index(current_shot)
        pre_index = post_index = index
        JUMP_A = JUMP_B = False
        MIN_INDEX = 0
        MAX_INDEX = len(shot_list) -1
        pre_shot = post_shot = None
        while True:
            if not JUMP_A and pre_index > MIN_INDEX:
                pre_index -= 1
            if not JUMP_B and post_index < MAX_INDEX:
                post_index += 1

            pre_shot = shot_list[pre_index]
            post_shot = shot_list[post_index]

            if doseImageExist(pre_shot):
                JUMP_A = True
            if doseImageExist(post_shot):
                JUMP_B = True
            
            if JUMP_A and JUMP_B:
                break
            if pre_index < MIN_INDEX and post_index > MAX_INDEX:
                pre_shot=post_shot=None;break
            if JUMP_A and post_index == MAX_INDEX:
                post_shot=None;break
            if pre_index == MIN_INDEX and JUMP_B:
                pre_index=None;break
    else:
       pre_shot = post_shot = None

    if pre_shot is not None:
        pre_shot_label = pre_shot
        pre_shot_image = getImageList(pre_shot)[0]
    else:
        pre_shot_label = None
        pre_shot_image = _iconsDir
    if post_shot is not None:
        post_shot_label = post_shot
        post_shot_image = getImageList(post_shot)[0]
    else:
        post_shot_label = None
        post_shot_image = _iconsDir

    ptr.pre_shot_label.setText(
        "<font color='gray'>Pre Shot: %s</font>"%pre_shot_label)
    pre_shot_pixmap = QtGui.QPixmap(pre_shot_image)
    ptr.pre_shot_image.setPixmap(pre_shot_pixmap)
    ptr.post_shot_label.setText(
        "<font color='gray'>Post Shot: %s</font>"%post_shot_label)
    post_shot_pixmap = QtGui.QPixmap(post_shot_image)
    ptr.post_shot_image.setPixmap(post_shot_pixmap)

    ptr._pre_shot_name = pre_shot_label
    ptr._post_shot_name = post_shot_label
    if pre_shot_label is not None:
        ptr._pre_shot_frame_range = getImageRange(pre_shot_label)
        ptr._pre_shot_frame = ptr._pre_shot_frame_range[0]
    if post_shot_label is not None:
        ptr._post_shot_frame_range = getImageRange(post_shot_label)
        ptr._post_shot_frame = ptr._post_shot_frame_range[0]

    ptr.update();ptr.hide();ptr.show()

def update(ptr):
    pre_shot = ptr._pre_shot_name
    post_shot = ptr._post_shot_name

    if pre_shot is not None:
        pre_shot_label = pre_shot
        pre_shot_image = getImageList(pre_shot)[0]
    else:
        pre_shot_label = None
        pre_shot_image = _iconsDir
    if post_shot is not None:
        post_shot_label = post_shot
        post_shot_image = getImageList(post_shot)[0]
    else:
        post_shot_label = None
        post_shot_image = _iconsDir


class KDialog(QtGui.QDialog):
    def __init__(self, parent=None):
        QtGui.QDialog.__init__(self, parent)
        self.setWindowTitle("Color Reference Setting")
        main_layout = QtGui.QVBoxLayout(self)

        group_layout = QtGui.QHBoxLayout()
        pre_group = QtGui.QGroupBox()
        pre_group.setTitle("Pre Shot")
        post_group = QtGui.QGroupBox()
        post_group.setTitle("Post Shot")
        group_layout.addWidget(pre_group)
        group_layout.addWidget(post_group)

        self.pre_shot_widget = kLineTextLabel()
        self.pre_shot_widget.setLabel("shot name")
        self.pre_frame_widget = kLineTextLabel()
        self.pre_frame_widget.setLabel("shot frame")
        pre_group_layout = QtGui.QVBoxLayout(pre_group)
        pre_group_layout.setContentsMargins(5,5,5,5)
        pre_group_layout.addWidget(self.pre_shot_widget)
        pre_group_layout.addWidget(self.pre_frame_widget)

        self.post_shot_widget = kLineTextLabel()
        self.post_shot_widget.setLabel("shot name")
        self.post_frame_widget = kLineTextLabel()
        self.post_frame_widget.setLabel("shot frame")
        post_group_layout = QtGui.QVBoxLayout(post_group)
        post_group_layout.setContentsMargins(5,5,5,5)
        post_group_layout.addWidget(self.post_shot_widget)
        post_group_layout.addWidget(self.post_frame_widget)

        if parent is not None:
            if parent._pre_shot_name is not None:
                self.pre_shot_widget.setText(parent._pre_shot_name)
            if parent._pre_shot_frame is not None:
                self.pre_frame_widget.setText(parent._pre_shot_frame)
            if parent._post_shot_name is not None:
                self.post_shot_widget.setText(parent._post_shot_name)
            if parent._post_shot_frame is not None:
                self.post_frame_widget.setText(parent._post_shot_frame)

        ok_button = QtGui.QPushButton("ok")
        ok_button.clicked.connect(
            lambda arg=False, args=parent:self.clicked_ok(args))
        cancel_button = QtGui.QPushButton("cancel")
        cancel_button.clicked.connect(self.close)
        button_layout = QtGui.QHBoxLayout()
        button_layout.addWidget(ok_button)
        button_layout.addWidget(cancel_button)

        main_layout.addLayout(group_layout)
        main_layout.addSpacing(25)
        main_layout.addLayout(button_layout)

    def clicked_ok(self, ptr):
        pre_image = post_image = ""
        if self.pre_shot_widget.text():
            image_list = getImageList(self.pre_shot_widget.text())
            if self.pre_frame_widget.text():
                for image in image_list:
                    _temp = image[:-len(image.split(".")[-1])-1] 
                    if _temp.endswith(self.pre_frame_widget.text()):
                        pre_image = image
                        if os.path.isfile(pre_image):
                            ptr._pre_shot_name =  self.pre_shot_widget.text()
                            ptr._pre_shot_frame = self.pre_frame_widget.text()
                        break
        if self.post_shot_widget.text():
            image_list = getImageList(self.post_shot_widget.text())
            if self.post_frame_widget.text():
                for image in image_list:
                    _temp = image[:-len(image.split(".")[-1])-1] 
                    if _temp.endswith(self.post_frame_widget.text()):
                        post_image = image
                        if os.path.isfile(post_image):
                            ptr._post_shot_name = self.post_shot_widget.text()
                            ptr._post_shot_frame = self.post_frame_widget.text()
                        break

        if os.path.isfile(pre_image):
            ptr.pre_shot_label.setText(
                "<font color='gray'>Pre Shot: %s</font>"%ptr._pre_shot_name)
            pre_shot_pixmap = QtGui.QPixmap(pre_image)
            ptr.pre_shot_image.setPixmap(pre_shot_pixmap)
        if os.path.isfile(post_image):
            ptr.post_shot_label.setText(
                "<font color='gray'>Post Shot: %s</font>"%ptr._post_shot_name)
            post_shot_pixmap = QtGui.QPixmap(post_image)
            ptr.post_shot_image.setPixmap(post_shot_pixmap)

        ptr.update();ptr.hide();ptr.show()

        self.close()


def widget(ptr):
    widget = KDialog(parent=ptr)
    return widget

def main(ptr):
    window = widget(ptr)
    window.resize(600,200)
    window.show()
if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    window = widget(None)
    window.resize(300,200)
    window.show()
    sys.exit(app.exec_())
