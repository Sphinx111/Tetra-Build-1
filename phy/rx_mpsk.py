#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Rx Mpsk
# GNU Radio version: 3.7.14.0
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

from PyQt4 import Qt
from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import digital;import cmath
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio import qtgui
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.qtgui import Range, RangeWidget
from optparse import OptionParser
import osmosdr
import sip
import sys
import time
from gnuradio import qtgui


class rx_mpsk(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Rx Mpsk")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Rx Mpsk")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "rx_mpsk")
        self.restoreGeometry(self.settings.value("geometry").toByteArray())


        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 2560000

        self.rrc_taps_data = rrc_taps_data = firdes.root_raised_cosine(1.0, 2, 1, 0.35, 22)

        self.rf_gain = rf_gain = 44
        self.ppm_corr = ppm_corr = 0
        self.if_gain = if_gain = 26
        self.frequency = frequency = 467.5625e6
        self.freq_offset = freq_offset = 200e3
        self.decim = decim = 32
        self.channel_rate = channel_rate = 36000
        self.bb_gain = bb_gain = 20

        ##################################################
        # Blocks
        ##################################################
        self._rf_gain_range = Range(0, 44, 1, 44, 50)
        self._rf_gain_win = RangeWidget(self._rf_gain_range, self.set_rf_gain, "rf_gain", "counter_slider", int)
        self.top_grid_layout.addWidget(self._rf_gain_win, 1, 2, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(2, 3):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._ppm_corr_range = Range(-15, 15, 1, 0, 50)
        self._ppm_corr_win = RangeWidget(self._ppm_corr_range, self.set_ppm_corr, "ppm_corr", "counter_slider", int)
        self.top_grid_layout.addWidget(self._ppm_corr_win, 1, 1, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._if_gain_range = Range(0, 44, 1, 26, 50)
        self._if_gain_win = RangeWidget(self._if_gain_range, self.set_if_gain, "if_gain", "counter_slider", int)
        self.top_grid_layout.addWidget(self._if_gain_win, 2, 1, 1, 1)
        for r in range(2, 3):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._bb_gain_range = Range(0, 32, 1, 20, 50)
        self._bb_gain_win = RangeWidget(self._bb_gain_range, self.set_bb_gain, "bb_gain", "counter_slider", int)
        self.top_grid_layout.addWidget(self._bb_gain_win, 2, 2, 1, 1)
        for r in range(2, 3):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(2, 3):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.rtlsdr_source_0 = osmosdr.source( args="numchan=" + str(1) + " " + '' )
        self.rtlsdr_source_0.set_sample_rate(samp_rate)
        self.rtlsdr_source_0.set_center_freq(frequency-freq_offset, 0)
        self.rtlsdr_source_0.set_freq_corr(ppm_corr, 0)
        self.rtlsdr_source_0.set_dc_offset_mode(2, 0)
        self.rtlsdr_source_0.set_iq_balance_mode(2, 0)
        self.rtlsdr_source_0.set_gain_mode(True, 0)
        self.rtlsdr_source_0.set_gain(rf_gain, 0)
        self.rtlsdr_source_0.set_if_gain(if_gain, 0)
        self.rtlsdr_source_0.set_bb_gain(bb_gain, 0)
        self.rtlsdr_source_0.set_antenna('', 0)
        self.rtlsdr_source_0.set_bandwidth(0, 0)

        self.root_raised_cosine_filter_0 = filter.fir_filter_ccf(1, firdes.root_raised_cosine(
        	1, 2, 1, 0.35, 11*2))
        self.qtgui_sink_x_0_0 = qtgui.sink_c(
        	1024, #fftsize
        	firdes.WIN_BLACKMAN_hARRIS, #wintype
        	0, #fc
        	channel_rate, #bw
        	"", #name
        	False, #plotfreq
        	False, #plotwaterfall
        	False, #plottime
        	True, #plotconst
        )
        self.qtgui_sink_x_0_0.set_update_time(1.0/10)
        self._qtgui_sink_x_0_0_win = sip.wrapinstance(self.qtgui_sink_x_0_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_sink_x_0_0_win, 3, 1, 6, 2)
        for r in range(3, 9):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 3):
            self.top_grid_layout.setColumnStretch(c, 1)


        self.qtgui_sink_x_0_0.enable_rf_freq(False)



        self.freq_xlating_fir_filter_xxx_0 = filter.freq_xlating_fir_filter_ccc(decim, (firdes.low_pass(1,samp_rate,12500,12500*0.2)), freq_offset, samp_rate)
        self.fractional_resampler_xx_0 = filter.fractional_resampler_cc(0, float(samp_rate)/float(decim)/float(channel_rate))
        self.digital_mpsk_receiver_cc_0 = digital.mpsk_receiver_cc(4, cmath.pi/4.0, cmath.pi/180.0, -0.5, 0.5, 0.25, 0.001, 2, 0.001, 0.001)
        self.digital_map_bb_0 = digital.map_bb(([3,2,0,1,3]))
        self.digital_diff_phasor_cc_0 = digital.diff_phasor_cc()
        self.blocks_unpack_k_bits_bb_0 = blocks.unpack_k_bits_bb(2)
        self.blocks_udp_sink_0 = blocks.udp_sink(gr.sizeof_char*1, "127.0.0.1", 42000, 1472, False)
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_vff((2/cmath.pi, ))
        self.blocks_float_to_uchar_0 = blocks.float_to_uchar()
        self.blocks_complex_to_arg_0 = blocks.complex_to_arg(1)
        self.blocks_add_const_vxx_0 = blocks.add_const_vff((1.5, ))
        self.analog_feedforward_agc_cc_0 = analog.feedforward_agc_cc(8, 1)



        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_feedforward_agc_cc_0, 0), (self.fractional_resampler_xx_0, 0))
        self.connect((self.blocks_add_const_vxx_0, 0), (self.blocks_float_to_uchar_0, 0))
        self.connect((self.blocks_complex_to_arg_0, 0), (self.blocks_multiply_const_vxx_0, 0))
        self.connect((self.blocks_float_to_uchar_0, 0), (self.digital_map_bb_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.blocks_add_const_vxx_0, 0))
        self.connect((self.blocks_unpack_k_bits_bb_0, 0), (self.blocks_udp_sink_0, 0))
        self.connect((self.digital_diff_phasor_cc_0, 0), (self.blocks_complex_to_arg_0, 0))
        self.connect((self.digital_diff_phasor_cc_0, 0), (self.qtgui_sink_x_0_0, 0))
        self.connect((self.digital_map_bb_0, 0), (self.blocks_unpack_k_bits_bb_0, 0))
        self.connect((self.digital_mpsk_receiver_cc_0, 0), (self.digital_diff_phasor_cc_0, 0))
        self.connect((self.fractional_resampler_xx_0, 0), (self.root_raised_cosine_filter_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.analog_feedforward_agc_cc_0, 0))
        self.connect((self.root_raised_cosine_filter_0, 0), (self.digital_mpsk_receiver_cc_0, 0))
        self.connect((self.rtlsdr_source_0, 0), (self.freq_xlating_fir_filter_xxx_0, 0))

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "rx_mpsk")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.rtlsdr_source_0.set_sample_rate(self.samp_rate)
        self.freq_xlating_fir_filter_xxx_0.set_taps((firdes.low_pass(1,self.samp_rate,12500,12500*0.2)))
        self.fractional_resampler_xx_0.set_resamp_ratio(float(self.samp_rate)/float(self.decim)/float(self.channel_rate))

    def get_rrc_taps_data(self):
        return self.rrc_taps_data

    def set_rrc_taps_data(self, rrc_taps_data):
        self.rrc_taps_data = rrc_taps_data

    def get_rf_gain(self):
        return self.rf_gain

    def set_rf_gain(self, rf_gain):
        self.rf_gain = rf_gain
        self.rtlsdr_source_0.set_gain(self.rf_gain, 0)

    def get_ppm_corr(self):
        return self.ppm_corr

    def set_ppm_corr(self, ppm_corr):
        self.ppm_corr = ppm_corr
        self.rtlsdr_source_0.set_freq_corr(self.ppm_corr, 0)

    def get_if_gain(self):
        return self.if_gain

    def set_if_gain(self, if_gain):
        self.if_gain = if_gain
        self.rtlsdr_source_0.set_if_gain(self.if_gain, 0)

    def get_frequency(self):
        return self.frequency

    def set_frequency(self, frequency):
        self.frequency = frequency
        self.rtlsdr_source_0.set_center_freq(self.frequency-self.freq_offset, 0)

    def get_freq_offset(self):
        return self.freq_offset

    def set_freq_offset(self, freq_offset):
        self.freq_offset = freq_offset
        self.rtlsdr_source_0.set_center_freq(self.frequency-self.freq_offset, 0)
        self.freq_xlating_fir_filter_xxx_0.set_center_freq(self.freq_offset)

    def get_decim(self):
        return self.decim

    def set_decim(self, decim):
        self.decim = decim
        self.fractional_resampler_xx_0.set_resamp_ratio(float(self.samp_rate)/float(self.decim)/float(self.channel_rate))

    def get_channel_rate(self):
        return self.channel_rate

    def set_channel_rate(self, channel_rate):
        self.channel_rate = channel_rate
        self.qtgui_sink_x_0_0.set_frequency_range(0, self.channel_rate)
        self.fractional_resampler_xx_0.set_resamp_ratio(float(self.samp_rate)/float(self.decim)/float(self.channel_rate))

    def get_bb_gain(self):
        return self.bb_gain

    def set_bb_gain(self, bb_gain):
        self.bb_gain = bb_gain
        self.rtlsdr_source_0.set_bb_gain(self.bb_gain, 0)


def main(top_block_cls=rx_mpsk, options=None):

    from distutils.version import StrictVersion
    if StrictVersion(Qt.qVersion()) >= StrictVersion("4.5.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()
    tb.start()
    tb.show()

    def quitting():
        tb.stop()
        tb.wait()
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)
    qapp.exec_()


if __name__ == '__main__':
    main()
