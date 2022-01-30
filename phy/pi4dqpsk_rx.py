#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Pi4Dqpsk Rx
# GNU Radio version: 3.8.1.0

from distutils.version import StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

from PyQt5 import Qt
from gnuradio import eng_notation
from gnuradio import qtgui
import sip
from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio.qtgui import Range, RangeWidget
import cmath
import osmosdr
import time
from gnuradio import qtgui

class pi4dqpsk_rx(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Pi4Dqpsk Rx")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Pi4Dqpsk Rx")
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

        self.settings = Qt.QSettings("GNU Radio", "pi4dqpsk_rx")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Variables
        ##################################################
        self.sps = sps = 2
        self.nfilts = nfilts = 32
        self.samp_rate = samp_rate = 2000000
        self.rrc_taps = rrc_taps = firdes.root_raised_cosine(nfilts, nfilts, 1.0/float(sps), 0.35, 11*sps*nfilts)
        self.ppm_corr = ppm_corr = 0
        self.frequency_Mhz = frequency_Mhz = '394.305'
        self.freq_offset_khz = freq_offset_khz = 50
        self.decim = decim = 32
        self.constel = constel = digital.constellation_dqpsk().base()
        self.constel.gen_soft_dec_lut(8)
        self.channel_rate = channel_rate = 36000
        self.arity = arity = 4

        ##################################################
        # Blocks
        ##################################################
        self._ppm_corr_range = Range(-20, 15, 1, 0, 200)
        self._ppm_corr_win = RangeWidget(self._ppm_corr_range, self.set_ppm_corr, 'PPM correction', "counter_slider", int)
        self.top_grid_layout.addWidget(self._ppm_corr_win, 2, 0, 1, 2)
        for r in range(2, 3):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._frequency_Mhz_tool_bar = Qt.QToolBar(self)
        self._frequency_Mhz_tool_bar.addWidget(Qt.QLabel('Frequency [MHz]' + ": "))
        self._frequency_Mhz_line_edit = Qt.QLineEdit(str(self.frequency_Mhz))
        self._frequency_Mhz_tool_bar.addWidget(self._frequency_Mhz_line_edit)
        self._frequency_Mhz_line_edit.returnPressed.connect(
            lambda: self.set_frequency_Mhz(str(str(self._frequency_Mhz_line_edit.text()))))
        self.top_grid_layout.addWidget(self._frequency_Mhz_tool_bar, 1, 0, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._freq_offset_khz_tool_bar = Qt.QToolBar(self)
        self._freq_offset_khz_tool_bar.addWidget(Qt.QLabel('Baseband offset [kHz]' + ": "))
        self._freq_offset_khz_line_edit = Qt.QLineEdit(str(self.freq_offset_khz))
        self._freq_offset_khz_tool_bar.addWidget(self._freq_offset_khz_line_edit)
        self._freq_offset_khz_line_edit.returnPressed.connect(
            lambda: self.set_freq_offset_khz(eng_notation.str_to_num(str(self._freq_offset_khz_line_edit.text()))))
        self.top_grid_layout.addWidget(self._freq_offset_khz_tool_bar, 1, 1, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.rtlsdr_source = osmosdr.source(
            args="numchan=" + str(1) + " " + ''
        )
        self.rtlsdr_source.set_time_unknown_pps(osmosdr.time_spec_t())
        self.rtlsdr_source.set_sample_rate(samp_rate)
        self.rtlsdr_source.set_center_freq(float(frequency_Mhz)*1e6-freq_offset_khz*1e3, 0)
        self.rtlsdr_source.set_freq_corr(ppm_corr, 0)
        self.rtlsdr_source.set_gain(0, 0)
        self.rtlsdr_source.set_if_gain(0, 0)
        self.rtlsdr_source.set_bb_gain(0, 0)
        self.rtlsdr_source.set_antenna('', 0)
        self.rtlsdr_source.set_bandwidth(0, 0)
        self.qtgui_const_sink_x_0_0 = qtgui.const_sink_c(
            1024, #size
            "output", #name
            1 #number of inputs
        )
        self.qtgui_const_sink_x_0_0.set_update_time(0.10)
        self.qtgui_const_sink_x_0_0.set_y_axis(-2, 2)
        self.qtgui_const_sink_x_0_0.set_x_axis(-2, 2)
        self.qtgui_const_sink_x_0_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, "")
        self.qtgui_const_sink_x_0_0.enable_autoscale(False)
        self.qtgui_const_sink_x_0_0.enable_grid(False)
        self.qtgui_const_sink_x_0_0.enable_axis_labels(True)

        self.qtgui_const_sink_x_0_0.disable_legend()

        labels = ['', 'other rx', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "red", "red", "red",
            "red", "red", "red", "red", "red"]
        styles = [0, 0, 0, 0, 0,
            0, 0, 0, 0, 0]
        markers = [0, 0, 0, 0, 0,
            0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_const_sink_x_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_const_sink_x_0_0.set_line_label(i, labels[i])
            self.qtgui_const_sink_x_0_0.set_line_width(i, widths[i])
            self.qtgui_const_sink_x_0_0.set_line_color(i, colors[i])
            self.qtgui_const_sink_x_0_0.set_line_style(i, styles[i])
            self.qtgui_const_sink_x_0_0.set_line_marker(i, markers[i])
            self.qtgui_const_sink_x_0_0.set_line_alpha(i, alphas[i])

        self._qtgui_const_sink_x_0_0_win = sip.wrapinstance(self.qtgui_const_sink_x_0_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_const_sink_x_0_0_win, 4, 0, 1, 2)
        for r in range(4, 5):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.mmse_resampler_xx_0 = filter.mmse_resampler_cc(0, float(samp_rate)/(float(decim)*float(channel_rate)))
        self.freq_xlating_fir_filter_xxx_0 = filter.freq_xlating_fir_filter_ccc(decim, firdes.low_pass(1,samp_rate,12500,12500*0.2), freq_offset_khz*1e3, samp_rate)
        self.digital_pfb_clock_sync_xxx_0 = digital.pfb_clock_sync_ccf(sps, 2*cmath.pi/100.0, rrc_taps, nfilts, nfilts/2, 1.5, sps)
        self.digital_map_bb_0 = digital.map_bb(constel.pre_diff_code())
        self.digital_fll_band_edge_cc_0 = digital.fll_band_edge_cc(sps, 0.35, 45, cmath.pi/100.0)
        self.digital_diff_phasor_cc_0 = digital.diff_phasor_cc()
        self.digital_constellation_decoder_cb_0 = digital.constellation_decoder_cb(constel)
        self.digital_cma_equalizer_cc_0 = digital.cma_equalizer_cc(15, 1, 10e-3, sps)
        self.blocks_unpack_k_bits_bb_0 = blocks.unpack_k_bits_bb(constel.bits_per_symbol())
        self.blocks_udp_sink_0 = blocks.udp_sink(gr.sizeof_char*1, "127.0.0.1", 42000, 1472, False)
        self.analog_feedforward_agc_cc_0 = analog.feedforward_agc_cc(8, 1)



        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_feedforward_agc_cc_0, 0), (self.digital_fll_band_edge_cc_0, 0))
        self.connect((self.blocks_unpack_k_bits_bb_0, 0), (self.blocks_udp_sink_0, 0))
        self.connect((self.digital_cma_equalizer_cc_0, 0), (self.digital_diff_phasor_cc_0, 0))
        self.connect((self.digital_constellation_decoder_cb_0, 0), (self.digital_map_bb_0, 0))
        self.connect((self.digital_diff_phasor_cc_0, 0), (self.digital_constellation_decoder_cb_0, 0))
        self.connect((self.digital_diff_phasor_cc_0, 0), (self.qtgui_const_sink_x_0_0, 0))
        self.connect((self.digital_fll_band_edge_cc_0, 0), (self.digital_pfb_clock_sync_xxx_0, 0))
        self.connect((self.digital_map_bb_0, 0), (self.blocks_unpack_k_bits_bb_0, 0))
        self.connect((self.digital_pfb_clock_sync_xxx_0, 0), (self.digital_cma_equalizer_cc_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.mmse_resampler_xx_0, 0))
        self.connect((self.mmse_resampler_xx_0, 0), (self.analog_feedforward_agc_cc_0, 0))
        self.connect((self.rtlsdr_source, 0), (self.freq_xlating_fir_filter_xxx_0, 0))

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "pi4dqpsk_rx")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_sps(self):
        return self.sps

    def set_sps(self, sps):
        self.sps = sps
        self.set_rrc_taps(firdes.root_raised_cosine(self.nfilts, self.nfilts, 1.0/float(self.sps), 0.35, 11*self.sps*self.nfilts))

    def get_nfilts(self):
        return self.nfilts

    def set_nfilts(self, nfilts):
        self.nfilts = nfilts
        self.set_rrc_taps(firdes.root_raised_cosine(self.nfilts, self.nfilts, 1.0/float(self.sps), 0.35, 11*self.sps*self.nfilts))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.freq_xlating_fir_filter_xxx_0.set_taps(firdes.low_pass(1,self.samp_rate,12500,12500*0.2))
        self.mmse_resampler_xx_0.set_resamp_ratio(float(self.samp_rate)/(float(self.decim)*float(self.channel_rate)))
        self.rtlsdr_source.set_sample_rate(self.samp_rate)

    def get_rrc_taps(self):
        return self.rrc_taps

    def set_rrc_taps(self, rrc_taps):
        self.rrc_taps = rrc_taps
        self.digital_pfb_clock_sync_xxx_0.update_taps(self.rrc_taps)

    def get_ppm_corr(self):
        return self.ppm_corr

    def set_ppm_corr(self, ppm_corr):
        self.ppm_corr = ppm_corr
        self.rtlsdr_source.set_freq_corr(self.ppm_corr, 0)

    def get_frequency_Mhz(self):
        return self.frequency_Mhz

    def set_frequency_Mhz(self, frequency_Mhz):
        self.frequency_Mhz = frequency_Mhz
        Qt.QMetaObject.invokeMethod(self._frequency_Mhz_line_edit, "setText", Qt.Q_ARG("QString", str(self.frequency_Mhz)))
        self.rtlsdr_source.set_center_freq(float(self.frequency_Mhz)*1e6-self.freq_offset_khz*1e3, 0)

    def get_freq_offset_khz(self):
        return self.freq_offset_khz

    def set_freq_offset_khz(self, freq_offset_khz):
        self.freq_offset_khz = freq_offset_khz
        Qt.QMetaObject.invokeMethod(self._freq_offset_khz_line_edit, "setText", Qt.Q_ARG("QString", eng_notation.num_to_str(self.freq_offset_khz)))
        self.freq_xlating_fir_filter_xxx_0.set_center_freq(self.freq_offset_khz*1e3)
        self.rtlsdr_source.set_center_freq(float(self.frequency_Mhz)*1e6-self.freq_offset_khz*1e3, 0)

    def get_decim(self):
        return self.decim

    def set_decim(self, decim):
        self.decim = decim
        self.mmse_resampler_xx_0.set_resamp_ratio(float(self.samp_rate)/(float(self.decim)*float(self.channel_rate)))

    def get_constel(self):
        return self.constel

    def set_constel(self, constel):
        self.constel = constel

    def get_channel_rate(self):
        return self.channel_rate

    def set_channel_rate(self, channel_rate):
        self.channel_rate = channel_rate
        self.mmse_resampler_xx_0.set_resamp_ratio(float(self.samp_rate)/(float(self.decim)*float(self.channel_rate)))

    def get_arity(self):
        return self.arity

    def set_arity(self, arity):
        self.arity = arity



def main(top_block_cls=pi4dqpsk_rx, options=None):

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()
    tb.start()
    tb.show()

    def sig_handler(sig=None, frame=None):
        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    def quitting():
        tb.stop()
        tb.wait()
    qapp.aboutToQuit.connect(quitting)
    qapp.exec_()


if __name__ == '__main__':
    main()
