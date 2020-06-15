#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Pi4Dqpsk Rx Wx
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

from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio import wxgui
from gnuradio.eng_option import eng_option
from gnuradio.fft import window
from gnuradio.filter import firdes
from gnuradio.wxgui import constsink_gl
from gnuradio.wxgui import forms
from gnuradio.wxgui import waterfallsink2
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import cmath
import osmosdr
import time
import wx


class pi4dqpsk_rx_wx(grc_wxgui.top_block_gui):

    def __init__(self):
        grc_wxgui.top_block_gui.__init__(self, title="Pi4Dqpsk Rx Wx")

        ##################################################
        # Variables
        ##################################################
        self.sps = sps = 2
        self.nfilts = nfilts = 32
        self.samp_rate = samp_rate = 2000000
        self.rrc_taps = rrc_taps = firdes.root_raised_cosine(nfilts, nfilts, 1.0/float(sps), 0.35, 11*sps*nfilts)
        self.ppm_corr = ppm_corr = 0
        self.frequency_mhz = frequency_mhz = 467.565
        self.freq_offset_khz = freq_offset_khz = 50
        self.decim = decim = 32

        self.constel = constel = digital.constellation_dqpsk().base()

        self.constel.gen_soft_dec_lut(8)
        self.channel_rate = channel_rate = 36000
        self.arity = arity = 4

        ##################################################
        # Blocks
        ##################################################
        _ppm_corr_sizer = wx.BoxSizer(wx.VERTICAL)
        self._ppm_corr_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_ppm_corr_sizer,
        	value=self.ppm_corr,
        	callback=self.set_ppm_corr,
        	label='PPM correction',
        	converter=forms.float_converter(),
        	proportion=0,
        )
        self._ppm_corr_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_ppm_corr_sizer,
        	value=self.ppm_corr,
        	callback=self.set_ppm_corr,
        	minimum=-20,
        	maximum=15,
        	num_steps=35,
        	style=wx.SL_HORIZONTAL,
        	cast=float,
        	proportion=1,
        )
        self.GridAdd(_ppm_corr_sizer, 2, 0, 1, 2)
        self._frequency_mhz_text_box = forms.text_box(
        	parent=self.GetWin(),
        	value=self.frequency_mhz,
        	callback=self.set_frequency_mhz,
        	label='Frequency [MHz]',
        	converter=forms.float_converter(),
        )
        self.GridAdd(self._frequency_mhz_text_box, 1, 0, 1, 1)
        self._freq_offset_khz_text_box = forms.text_box(
        	parent=self.GetWin(),
        	value=self.freq_offset_khz,
        	callback=self.set_freq_offset_khz,
        	label='Baseband offset [kHz]',
        	converter=forms.float_converter(),
        )
        self.GridAdd(self._freq_offset_khz_text_box, 1, 1, 1, 1)
        self.wxgui_waterfallsink2_0 = waterfallsink2.waterfall_sink_c(
        	self.GetWin(),
        	baseband_freq=0,
        	dynamic_range=150,
        	ref_level=-10,
        	ref_scale=2,
        	sample_rate=channel_rate,
        	fft_size=256,
        	fft_rate=10,
        	average=False,
        	avg_alpha=None,
        	title="",
        	win=window.blackmanharris,
        )
        self.GridAdd(self.wxgui_waterfallsink2_0.win, 3, 0, 1, 2)
        self.wxgui_constellationsink2_0 = constsink_gl.const_sink_c(
        	self.GetWin(),
        	title="output",
        	sample_rate=channel_rate,
        	frame_rate=40,
        	const_size=2048,
        	M=4,
        	theta=0,
        	loop_bw=6.28/100.0,
        	fmax=0.06,
        	mu=0.5,
        	gain_mu=0.005,
        	symbol_rate=channel_rate,
        	omega_limit=0.005,
        )
        self.GridAdd(self.wxgui_constellationsink2_0.win, 4, 0, 1, 2)
        self.rtlsdr_source = osmosdr.source( args="numchan=" + str(1) + " " + '' )
        self.rtlsdr_source.set_sample_rate(samp_rate)
        self.rtlsdr_source.set_center_freq(frequency_mhz*1e6-freq_offset_khz*1e3, 0)
        self.rtlsdr_source.set_freq_corr(ppm_corr, 0)
        self.rtlsdr_source.set_dc_offset_mode(0, 0)
        self.rtlsdr_source.set_iq_balance_mode(0, 0)
        self.rtlsdr_source.set_gain_mode(True, 0)
        self.rtlsdr_source.set_gain(0, 0)
        self.rtlsdr_source.set_if_gain(0, 0)
        self.rtlsdr_source.set_bb_gain(0, 0)
        self.rtlsdr_source.set_antenna('', 0)
        self.rtlsdr_source.set_bandwidth(0, 0)

        self.freq_xlating_fir_filter_xxx_0 = filter.freq_xlating_fir_filter_ccc(decim, (firdes.low_pass(1,samp_rate,12500,12500*0.2)), freq_offset_khz*1e3, samp_rate)
        self.fractional_resampler_xx_0 = filter.fractional_resampler_cc(0, float(samp_rate)/(float(decim)*float(channel_rate)))
        self.digital_pfb_clock_sync_xxx_0 = digital.pfb_clock_sync_ccf(sps, 2*cmath.pi/100.0, (rrc_taps), nfilts, nfilts/2, 1.5, sps)
        self.digital_map_bb_0 = digital.map_bb((constel.pre_diff_code()))
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
        self.connect((self.digital_diff_phasor_cc_0, 0), (self.wxgui_constellationsink2_0, 0))
        self.connect((self.digital_fll_band_edge_cc_0, 0), (self.digital_pfb_clock_sync_xxx_0, 0))
        self.connect((self.digital_map_bb_0, 0), (self.blocks_unpack_k_bits_bb_0, 0))
        self.connect((self.digital_pfb_clock_sync_xxx_0, 0), (self.digital_cma_equalizer_cc_0, 0))
        self.connect((self.fractional_resampler_xx_0, 0), (self.analog_feedforward_agc_cc_0, 0))
        self.connect((self.fractional_resampler_xx_0, 0), (self.wxgui_waterfallsink2_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.fractional_resampler_xx_0, 0))
        self.connect((self.rtlsdr_source, 0), (self.freq_xlating_fir_filter_xxx_0, 0))

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
        self.rtlsdr_source.set_sample_rate(self.samp_rate)
        self.freq_xlating_fir_filter_xxx_0.set_taps((firdes.low_pass(1,self.samp_rate,12500,12500*0.2)))
        self.fractional_resampler_xx_0.set_resamp_ratio(float(self.samp_rate)/(float(self.decim)*float(self.channel_rate)))

    def get_rrc_taps(self):
        return self.rrc_taps

    def set_rrc_taps(self, rrc_taps):
        self.rrc_taps = rrc_taps
        self.digital_pfb_clock_sync_xxx_0.update_taps((self.rrc_taps))

    def get_ppm_corr(self):
        return self.ppm_corr

    def set_ppm_corr(self, ppm_corr):
        self.ppm_corr = ppm_corr
        self._ppm_corr_slider.set_value(self.ppm_corr)
        self._ppm_corr_text_box.set_value(self.ppm_corr)
        self.rtlsdr_source.set_freq_corr(self.ppm_corr, 0)

    def get_frequency_mhz(self):
        return self.frequency_mhz

    def set_frequency_mhz(self, frequency_mhz):
        self.frequency_mhz = frequency_mhz
        self._frequency_mhz_text_box.set_value(self.frequency_mhz)
        self.rtlsdr_source.set_center_freq(self.frequency_mhz*1e6-self.freq_offset_khz*1e3, 0)

    def get_freq_offset_khz(self):
        return self.freq_offset_khz

    def set_freq_offset_khz(self, freq_offset_khz):
        self.freq_offset_khz = freq_offset_khz
        self._freq_offset_khz_text_box.set_value(self.freq_offset_khz)
        self.rtlsdr_source.set_center_freq(self.frequency_mhz*1e6-self.freq_offset_khz*1e3, 0)
        self.freq_xlating_fir_filter_xxx_0.set_center_freq(self.freq_offset_khz*1e3)

    def get_decim(self):
        return self.decim

    def set_decim(self, decim):
        self.decim = decim
        self.fractional_resampler_xx_0.set_resamp_ratio(float(self.samp_rate)/(float(self.decim)*float(self.channel_rate)))

    def get_constel(self):
        return self.constel

    def set_constel(self, constel):
        self.constel = constel

    def get_channel_rate(self):
        return self.channel_rate

    def set_channel_rate(self, channel_rate):
        self.channel_rate = channel_rate
        self.wxgui_waterfallsink2_0.set_sample_rate(self.channel_rate)
        self.wxgui_constellationsink2_0.set_sample_rate(self.channel_rate)
        self.fractional_resampler_xx_0.set_resamp_ratio(float(self.samp_rate)/(float(self.decim)*float(self.channel_rate)))

    def get_arity(self):
        return self.arity

    def set_arity(self, arity):
        self.arity = arity


def main(top_block_cls=pi4dqpsk_rx_wx, options=None):

    tb = top_block_cls()
    tb.Start(True)
    tb.Wait()


if __name__ == '__main__':
    main()
