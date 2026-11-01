from math import floor
from ericapi import fortschrittids


class ProgressCallbacks:
    _MAP_ID = {
        fortschrittids.ERIC_FORTSCHRITTCALLBACK_ID_EINLESEN: 'XML einlesen',
        fortschrittids.ERIC_FORTSCHRITTCALLBACK_ID_VORBEREITEN: 'Versand vorbereiten',
        fortschrittids.ERIC_FORTSCHRITTCALLBACK_ID_VALIDIEREN: 'Validieren',
        fortschrittids.ERIC_FORTSCHRITTCALLBACK_ID_SENDEN: 'Versenden',
        fortschrittids.ERIC_FORTSCHRITTCALLBACK_ID_DRUCKEN: 'Drucken'
    }
    _BAR_LENGTH = 78

    def __init__(self):
        self._current_pos = -1
        self._current_bar = 0
        self._current_id = None

    @staticmethod
    def global_progress(cbid: int, cbpos: int, cbmax: int):
        print('{}/{}: {}'.format(cbpos, cbmax, ProgressCallbacks._MAP_ID[cbid]))

    def progress(self, cbid: int, cbpos: int, cbmax: int):
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
