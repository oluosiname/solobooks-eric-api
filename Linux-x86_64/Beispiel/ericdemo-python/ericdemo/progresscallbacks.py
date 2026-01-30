from math import floor
from ericapi.fortschrittids import *


class ProgressCallbacks:
    _mapId = {
        ERIC_FORTSCHRITTCALLBACK_ID_EINLESEN: 'XML einlesen',
        ERIC_FORTSCHRITTCALLBACK_ID_VORBEREITEN: 'Versand vorbereiten',
        ERIC_FORTSCHRITTCALLBACK_ID_VALIDIEREN: 'Validieren',
        ERIC_FORTSCHRITTCALLBACK_ID_SENDEN: 'Versenden',
        ERIC_FORTSCHRITTCALLBACK_ID_DRUCKEN: 'Drucken'
    }
    _BAR_LENGTH = 78

    def __init__(self):
        self._current_pos = -1
        self._current_bar = 0
        self._current_id = None

    @staticmethod
    def global_progress(cbid, cbpos, cbmax):
        print('{}/{}: {}'.format(cbpos, cbmax, ProgressCallbacks._mapId[cbid]))

    def progress(self, cbid, cbpos, cbmax):
        if self._current_id != cbid:
            print('[', end='')
            self._current_id = cbid

        total_bar = floor(ProgressCallbacks._BAR_LENGTH * cbpos / cbmax)
        if total_bar != self._current_bar:
            print('#' * (total_bar - self._current_bar), end='')
            self._current_bar = total_bar

        if cbpos == cbmax:
            print(']')
            self._current_bar = 0
