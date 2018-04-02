# Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import (
    QtGui,
    QtCore,
    UI4,
    Utils,
    FnGeolib,
    NodegraphAPI,
    Nodes3DAPI,
    WorkingSetManager,
    WorkingSet,
    ScenegraphManager,
    FnAttribute
)

# These values match the ones defined in the Qt header. We are redefining them
# just in case Qt changes the values at some point.
# These values should match the ones defined in FnEventWrapper.h.
kNoButton = 0
kLeftButton = 1
kRightButton = 2
kMidButton = 4
kMiddleButton = kMidButton
kXButton1 = 8
kXButton2 = 16

class MouseEventTranslator(object):
    """
    Utility class for converting specific types of QEvent into attributes
    """
    # The types of events that can be translated
    EventTypes = [QtCore.QEvent.MouseButtonPress,
                  QtCore.QEvent.MouseButtonRelease,
                  QtCore.QEvent.MouseButtonDblClick,
                  QtCore.QEvent.MouseMove]

    @staticmethod
    def Translate(eventObject):
        """
        Converts a QEvent object into a GroupAttribute
        """
        gb = FnAttribute.GroupBuilder()
        if eventObject.type() == QtCore.QEvent.MouseMove:
            eventType = "MouseMove"
        elif eventObject.type() == QtCore.QEvent.MouseButtonPress:
            eventType = "MouseButtonPress"
        elif eventObject.type() == QtCore.QEvent.MouseButtonRelease:
            eventType = "MouseButtonRelease"
        elif eventObject.type() == QtCore.QEvent.MouseButtonDblClick:
            eventType = "MouseButtonDblClick"

        gb.set("type", FnAttribute.StringAttribute(eventType))
        gb.set("typeid", FnAttribute.IntAttribute(eventObject.type()))
        gb.set("data.x", FnAttribute.IntAttribute(eventObject.x()))
        gb.set("data.y", FnAttribute.IntAttribute(eventObject.y()))
        gb.set("data.globalX", FnAttribute.IntAttribute(eventObject.globalX()))
        gb.set("data.globalY", FnAttribute.IntAttribute(eventObject.globalY()))

        eventButton = int(eventObject.button())
        button = kNoButton
        if eventButton == QtCore.Qt.LeftButton:
            button = kLeftButton
        elif eventButton == QtCore.Qt.RightButton:
            button = kRightButton
        elif eventButton == QtCore.Qt.MidButton:
            button = kMidButton
        elif eventButton == QtCore.Qt.XButton1:
            button = kXButton1
        elif eventButton == QtCore.Qt.XButton2:
            button = kXButton2
        gb.set("data.button", FnAttribute.IntAttribute(button))

        buttons = int(eventObject.buttons())
        gb.set("data.buttons", FnAttribute.IntAttribute(buttons))
        gb.set("data.LeftButton", FnAttribute.IntAttribute(
            int(buttons & QtCore.Qt.LeftButton != 0)))
        gb.set("data.MidButton", FnAttribute.IntAttribute(
            int(buttons & QtCore.Qt.MidButton != 0)))
        gb.set("data.RightButton", FnAttribute.IntAttribute(
            int(buttons & QtCore.Qt.RightButton != 0)))
        gb.set("data.XButton1", FnAttribute.IntAttribute(
            int(buttons & QtCore.Qt.XButton1 != 0)))
        gb.set("data.XButton2", FnAttribute.IntAttribute(
            int(buttons & QtCore.Qt.XButton2 != 0)))

        modifiers = int(eventObject.modifiers())
        gb.set("data.modifiers", FnAttribute.IntAttribute(int(modifiers)))
        gb.set("data.ShiftModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.ShiftModifier != 0)))
        gb.set("data.ControlModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.ControlModifier != 0)))
        gb.set("data.AltModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.AltModifier != 0)))
        gb.set("data.MetaModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.MetaModifier != 0)))
        gb.set("data.KeypadModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.KeypadModifier != 0)))
        gb.set("data.GroupSwitchModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.GroupSwitchModifier != 0)))

        return gb.build()

class MouseWheelTranslator(object):
    """
    Utility class for converting specific types of QEvent into attributes
    """
    # The types of events that can be translated
    EventTypes = QtCore.QEvent.Wheel

    @staticmethod
    def Translate(eventObject):
        """
        Converts a QEvent object into a GroupAttribute
        """
        gb = FnAttribute.GroupBuilder()

        gb.set("type", FnAttribute.StringAttribute("Wheel"))
        gb.set("typeid", FnAttribute.IntAttribute(eventObject.type()))
        gb.set("data.x", FnAttribute.IntAttribute(eventObject.x()))
        gb.set("data.y", FnAttribute.IntAttribute(eventObject.y()))
        gb.set("data.globalX", FnAttribute.IntAttribute(eventObject.globalX()))
        gb.set("data.globalY", FnAttribute.IntAttribute(eventObject.globalY()))

        buttons = eventObject.buttons()
        gb.set("data.buttons", FnAttribute.IntAttribute(int(buttons)))
        gb.set("data.LeftButton", FnAttribute.IntAttribute(
                  int(buttons & QtCore.Qt.LeftButton != 0)))
        gb.set("data.MidButton", FnAttribute.IntAttribute(
                  int(buttons & QtCore.Qt.MidButton != 0)))
        gb.set("data.RightButton", FnAttribute.IntAttribute(
                  int(buttons & QtCore.Qt.RightButton != 0)))
        gb.set("data.XButton1", FnAttribute.IntAttribute(
                  int(buttons & QtCore.Qt.XButton1 != 0)))
        gb.set("data.XButton2", FnAttribute.IntAttribute(
                  int(buttons & QtCore.Qt.XButton2 != 0)))

        gb.set("data.delta", FnAttribute.IntAttribute(eventObject.delta()))
        gb.set("data.orientation", FnAttribute.IntAttribute(eventObject.orientation()))

        modifiers = int(eventObject.modifiers())
        gb.set("data.modifiers", FnAttribute.IntAttribute(int(modifiers)))
        gb.set("data.ShiftModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.ShiftModifier != 0)))
        gb.set("data.ControlModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.ControlModifier != 0)))
        gb.set("data.AltModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.AltModifier != 0)))
        gb.set("data.MetaModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.MetaModifier != 0)))
        gb.set("data.KeypadModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.KeypadModifier != 0)))
        gb.set("data.GroupSwitchModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.GroupSwitchModifier != 0)))
        return gb.build()

class EnterLeaveEventTranslator(object):
    """
    Utility class for converting specific types of QEvent into attributes
    """
    # The types of events that can be translated
    EventTypes = [QtCore.QEvent.Enter,
                  QtCore.QEvent.Leave]

    @staticmethod
    def Translate(eventObject):
        """
        Converts a QEvent object into a GroupAttribute
        """
        gb = FnAttribute.GroupBuilder()
        if eventObject.type() == QtCore.QEvent.Enter:
            eventType = "Enter"
        elif eventObject.type() == QtCore.QEvent.Leave:
            eventType = "Leave"

        gb.set("type", FnAttribute.StringAttribute(eventType))
        gb.set("typeid", FnAttribute.IntAttribute(eventObject.type()))

        return gb.build()

class KeyEventTranslator(object):
    """
    Utility class for converting specific types of QEvent into attributes
    """
    # The types of events that can be translated
    EventTypes = [QtCore.QEvent.KeyPress,
                  QtCore.QEvent.KeyRelease]

    @staticmethod
    def Translate(eventObject):
        """
        Converts a QEvent object into a GroupAttribute
        """
        gb = FnAttribute.GroupBuilder()

        if eventObject.type() == QtCore.QEvent.KeyPress:
            eventType = "KeyPress"
        elif eventObject.type() == QtCore.QEvent.KeyRelease:
            eventType = "KeyRelease"

        gb.set("type", FnAttribute.StringAttribute(eventType))
        gb.set("typeid", FnAttribute.IntAttribute(eventObject.type()))

        gb.set("data.isAutoRepeat",
               FnAttribute.IntAttribute(int(eventObject.isAutoRepeat())))
        gb.set("data.key", FnAttribute.IntAttribute(eventObject.key()))
        gb.set("data.nativeModifiers",
               FnAttribute.IntAttribute(eventObject.nativeModifiers()))
        gb.set("data.nativeScanCode",
               FnAttribute.IntAttribute(eventObject.nativeScanCode()))
        gb.set("data.nativeVirtualKey",
               FnAttribute.IntAttribute(eventObject.nativeVirtualKey()))

        # Pass the text version of the QKeySequence, which can be handy
        # when detecting key combinations in text (for example: "Ctrl+X")
        keySequence = QtGui.QKeySequence(
            int(eventObject.modifiers()) + eventObject.key())
        gb.set("data.keySequence",
               FnAttribute.StringAttribute(str(keySequence.toString())))

        modifiers = int(eventObject.modifiers())
        gb.set("data.modifiers", FnAttribute.IntAttribute(int(modifiers)))
        gb.set("data.ShiftModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.ShiftModifier != 0)))
        gb.set("data.ControlModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.ControlModifier != 0)))
        gb.set("data.AltModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.AltModifier != 0)))
        gb.set("data.MetaModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.MetaModifier != 0)))
        gb.set("data.KeypadModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.KeypadModifier != 0)))
        gb.set("data.GroupSwitchModifier", FnAttribute.IntAttribute(
                  int(modifiers & QtCore.Qt.GroupSwitchModifier != 0)))

        return gb.build()
