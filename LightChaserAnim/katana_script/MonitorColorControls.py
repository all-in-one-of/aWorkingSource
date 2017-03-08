#!/usr/bin/env python
# encoding: utf-8

from Katana import os, QtGui, QtCore, QT4Widgets, QT4FormWidgets, QT4Color, Utils
import UI4

Common2D = UI4.FormMaster.NodeHints.Common2D
__all__ = ['MonitorCCPanel']


class MonitorColorControl(UI4.Tabs.BaseTab):
    def __init__(self, parent):
        UI4.Tabs.BaseTab.__init__(self, parent)
        QtGui.QVBoxLayout(self)
        self.layout().setContentsMargins(4, 4, 4, 4)
        self._MonitorColorControl__monitorGammaWidget = QT4Color.MonitorGammaWidget(self)
        self.layout().addWidget(self._MonitorColorControl__monitorGammaWidget, 0)
        self.connect(self._MonitorColorControl__monitorGammaWidget, QtCore.SIGNAL('valueChanged'),
                     self._MonitorColorControl__monitorChanged_CB)
        self._MonitorColorControl__colorGradeWidget = QT4Color.ColorGradeWidget(self, title='Primary Color Grade',
                                                                                createChildrenInScroll=True)
        self.layout().addWidget(self._MonitorColorControl__colorGradeWidget, 10)
        self.connect(self._MonitorColorControl__colorGradeWidget, QtCore.SIGNAL('valueChanged'),
                     self._MonitorColorControl__gradeChanged_CB)
        self._MonitorColorControl__updateTimer = QtCore.QTimer(self)
        self.connect(self._MonitorColorControl__updateTimer, QtCore.SIGNAL('timeout()'),
                     self._MonitorColorControl__timer_CB)
        Utils.EventModule.RegisterCollapsedHandler(self._MonitorColorControl__monitor_drawStateUpdated_CB,
                                                   'monitor_drawStateUpdated', None, True)

    def _MonitorColorControl__monitor_drawStateUpdated_CB(self, *args, **kwargs):
        self._MonitorColorControl__updateTimer.start(100)

    def _MonitorColorControl__timer_CB(self):
        self._MonitorColorControl__update()

    def _MonitorColorControl__update(self):
        drawState = self.getPrimaryMonitorDrawState()
        if drawState is None:
            return None
        try:
            None._MonitorColorControl__colorGradeWidget.setCC(drawState.getCC())
        except AttributeError:
            pass
        monitorValueDict = {}
        monitorValueDict['fstop'] = drawState.getFStopOffset()
        monitorValueDict['gamma'] = max(drawState.getViewGamma())
        monitorValueDict['blackPoint'] = min(drawState.getViewMin())
        monitorValueDict['whitePoint'] = max(drawState.getViewMax())
        monitorValueDict['mute'] = drawState.getViewAdjustmentsMuted()
        self._MonitorColorControl__monitorGammaWidget.setValueDict(monitorValueDict)

    def _MonitorColorControl__monitorChanged_CB(self, valueDict, isFinal):
        for panel in UI4.App.Tabs.GetTabsByType('Monitor'):
            widget = panel.getMonitorWidget()
            for drawState in widget.getDrawStates():
                if 'fstop' in valueDict:
                    drawState.setFStopOffset(valueDict['fstop'])
                if 'gamma' in valueDict:
                    drawState.setViewGamma(valueDict['gamma'])
                if 'blackPoint' in valueDict:
                    drawState.setViewMin(valueDict['blackPoint'])
                if 'whitePoint' in valueDict:
                    drawState.setViewMax(valueDict['whitePoint'])
                if 'mute' in valueDict:
                    drawState.setViewAdjustmentsMuted(valueDict['mute'])
                    continue
                    continue

    def _MonitorColorControl__gradeChanged_CB(self, cc, isFinal):
        for panel in UI4.App.Tabs.GetTabsByType('Monitor'):
            widget = panel.getMonitorWidget()
            for drawState in widget.getDrawStates():
                drawState.setCC(cc)

    def getPrimaryMonitorDrawState(self):
        for panel in UI4.App.Tabs.GetTabsByType('Monitor'):
            widget = panel.getMonitorWidget()
            for drawState in widget.getDrawStates():
                return drawState


PluginRegistry = [
    ('KatanaPanel', 2, 'Monitor Color Control', MonitorColorControl)]
