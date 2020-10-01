#include <stdio.h>
#include "ad9361_api.h"
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

AD9361_InitParam default_init_param = {
    /* Device selection */
    ID_AD9364,  // dev_sel
    /* Identification number */
    0,      //id_no
    /* Reference Clock */
    10000000UL, //reference_clk_rate
    /* Base Configuration */
    0,      //two_rx_two_tx_mode_enable *** adi,2rx-2tx-mode-enable // (AD9364 must clear this)
    0,      //one_rx_one_tx_mode_use_rx_num *** adi,1rx-1tx-mode-use-rx-num // does'nt exist in ad9364
    0,      //one_rx_one_tx_mode_use_tx_num *** adi,1rx-1tx-mode-use-tx-num // does'nt exist in ad9364
    1,      //frequency_division_duplex_mode_enable *** adi,frequency-division-duplex-mode-enable // our system use FDD
    0,      //frequency_division_duplex_independent_mode_enable *** adi,frequency-division-duplex-independent-mode-enable  // ?
    0,      //tdd_use_dual_synth_mode_enable *** adi,tdd-use-dual-synth-mode-enable // don't care
    0,      //tdd_skip_vco_cal_enable *** adi,tdd-skip-vco-cal-enable // don't care
    0,      //tx_fastlock_delay_ns *** adi,tx-fastlock-delay-ns // ?
    0,      //rx_fastlock_delay_ns *** adi,rx-fastlock-delay-ns // ?
    0,      //rx_fastlock_pincontrol_enable *** adi,rx-fastlock-pincontrol-enable // ?
    0,      //tx_fastlock_pincontrol_enable *** adi,tx-fastlock-pincontrol-enable // ?
    0,      //external_rx_lo_enable *** adi,external-rx-lo-enable  // our system does'nt have external LO
    0,      //external_tx_lo_enable *** adi,external-tx-lo-enable // our system does'nt have external LO
    5,      //dc_offset_tracking_update_event_mask *** adi,dc-offset-tracking-update-event-mask
    6,      //dc_offset_attenuation_high_range *** adi,dc-offset-attenuation-high-range
    5,      //dc_offset_attenuation_low_range *** adi,dc-offset-attenuation-low-range
    0x28,   //dc_offset_count_high_range *** adi,dc-offset-count-high-range
    0x32,   //dc_offset_count_low_range *** adi,dc-offset-count-low-range
    0,      //split_gain_table_mode_enable *** adi,split-gain-table-mode-enable
    MAX_SYNTH_FREF, //trx_synthesizer_target_fref_overwrite_hz *** adi,trx-synthesizer-target-fref-overwrite-hz
    0,      // qec_tracking_slow_mode_enable *** adi,qec-tracking-slow-mode-enable
    /* ENSM Control */
    0,      //ensm_enable_pin_pulse_mode_enable *** adi,ensm-enable-pin-pulse-mode-enable
    0,      //ensm_enable_txnrx_control_enable *** adi,ensm-enable-txnrx-control-enable
    /* LO Control */
    1590000000UL,   //rx_synthesizer_frequency_hz *** adi,rx-synthesizer-frequency-hz
    1590000000UL,   //tx_synthesizer_frequency_hz *** adi,tx-synthesizer-frequency-hz
    1,              //tx_lo_powerdown_managed_enable *** adi,tx-lo-powerdown-managed-enable
    /* Rate & BW Control */
    {1280000000, 160000000, 80000000, 40000000, 40000000, 40000000},// rx_path_clock_frequencies[6] *** adi,rx-path-clock-frequencies // Generated with AD9361 Filter Design Wizard 16.1.3
    {1280000000, 160000000, 80000000, 40000000, 40000000, 40000000},// tx_path_clock_frequencies[6] *** adi,tx-path-clock-frequencies Generated with AD9361 Filter Design Wizard 16.1.3
    20172411,//rf_rx_bandwidth_hz *** adi,rf-rx-bandwidth-hz
    56000000,//rf_tx_bandwidth_hz *** adi,rf-tx-bandwidth-hz
    /* RF Port Control */
    1,      //rx_rf_port_input_select *** adi,rx-rf-port-input-select //(RX1B_N & RX1B_P)
    0,      //tx_rf_port_input_select *** adi,tx-rf-port-input-select
    /* TX Attenuation Control */
    40000,  //tx_attenuation_mdB *** adi,tx-attenuation-mdB
    0,      //update_tx_gain_in_alert_enable *** adi,update-tx-gain-in-alert-enable
    /* Reference Clock Control */
    1,      //xo_disable_use_ext_refclk_enable *** adi,xo-disable-use-ext-refclk-enable
    {8, 5920},  //dcxo_coarse_and_fine_tune[2] *** adi,dcxo-coarse-and-fine-tune
    CLKOUT_DISABLE, //clk_output_mode_select *** adi,clk-output-mode-select // turn off cause it is NC
    /* Gain Control */
    2,      //gc_rx1_mode *** adi,gc-rx1-mode
    2,      //gc_rx2_mode *** adi,gc-rx2-mode
    58,     //gc_adc_large_overload_thresh *** adi,gc-adc-large-overload-thresh
    4,      //gc_adc_ovr_sample_size *** adi,gc-adc-ovr-sample-size
    47,     //gc_adc_small_overload_thresh *** adi,gc-adc-small-overload-thresh
    8192,   //gc_dec_pow_measurement_duration *** adi,gc-dec-pow-measurement-duration
    0,      //gc_dig_gain_enable *** adi,gc-dig-gain-enable
    800,    //gc_lmt_overload_high_thresh *** adi,gc-lmt-overload-high-thresh
    704,    //gc_lmt_overload_low_thresh *** adi,gc-lmt-overload-low-thresh
    24,     //gc_low_power_thresh *** adi,gc-low-power-thresh
    15,     //gc_max_dig_gain *** adi,gc-max-dig-gain
    /* Gain MGC Control */
    2,      //mgc_dec_gain_step *** adi,mgc-dec-gain-step
    2,      //mgc_inc_gain_step *** adi,mgc-inc-gain-step
    0,      //mgc_rx1_ctrl_inp_enable *** adi,mgc-rx1-ctrl-inp-enable
    0,      //mgc_rx2_ctrl_inp_enable *** adi,mgc-rx2-ctrl-inp-enable
    0,      //mgc_split_table_ctrl_inp_gain_mode *** adi,mgc-split-table-ctrl-inp-gain-mode
    /* Gain AGC Control */
    10,     //agc_adc_large_overload_exceed_counter *** adi,agc-adc-large-overload-exceed-counter
    2,      //agc_adc_large_overload_inc_steps *** adi,agc-adc-large-overload-inc-steps
    0,      //agc_adc_lmt_small_overload_prevent_gain_inc_enable *** adi,agc-adc-lmt-small-overload-prevent-gain-inc-enable
    10,     //agc_adc_small_overload_exceed_counter *** adi,agc-adc-small-overload-exceed-counter
    4,      //agc_dig_gain_step_size *** adi,agc-dig-gain-step-size
    3,      //agc_dig_saturation_exceed_counter *** adi,agc-dig-saturation-exceed-counter
    1000,   // agc_gain_update_interval_us *** adi,agc-gain-update-interval-us
    0,      //agc_immed_gain_change_if_large_adc_overload_enable *** adi,agc-immed-gain-change-if-large-adc-overload-enable
    0,      //agc_immed_gain_change_if_large_lmt_overload_enable *** adi,agc-immed-gain-change-if-large-lmt-overload-enable
    10,     //agc_inner_thresh_high *** adi,agc-inner-thresh-high
    1,      //agc_inner_thresh_high_dec_steps *** adi,agc-inner-thresh-high-dec-steps
    12,     //agc_inner_thresh_low *** adi,agc-inner-thresh-low
    1,      //agc_inner_thresh_low_inc_steps *** adi,agc-inner-thresh-low-inc-steps
    10,     //agc_lmt_overload_large_exceed_counter *** adi,agc-lmt-overload-large-exceed-counter
    2,      //agc_lmt_overload_large_inc_steps *** adi,agc-lmt-overload-large-inc-steps
    10,     //agc_lmt_overload_small_exceed_counter *** adi,agc-lmt-overload-small-exceed-counter
    5,      //agc_outer_thresh_high *** adi,agc-outer-thresh-high
    2,      //agc_outer_thresh_high_dec_steps *** adi,agc-outer-thresh-high-dec-steps
    18,     //agc_outer_thresh_low *** adi,agc-outer-thresh-low
    2,      //agc_outer_thresh_low_inc_steps *** adi,agc-outer-thresh-low-inc-steps
    1,      //agc_attack_delay_extra_margin_us; *** adi,agc-attack-delay-extra-margin-us
    0,      //agc_sync_for_gain_counter_enable *** adi,agc-sync-for-gain-counter-enable
    /* Fast AGC */
    64,     //fagc_dec_pow_measuremnt_duration ***  adi,fagc-dec-pow-measurement-duration
    260,    //fagc_state_wait_time_ns ***  adi,fagc-state-wait-time-ns
    /* Fast AGC - Low Power */
    0,      //fagc_allow_agc_gain_increase ***  adi,fagc-allow-agc-gain-increase-enable
    5,      //fagc_lp_thresh_increment_time ***  adi,fagc-lp-thresh-increment-time
    1,      //fagc_lp_thresh_increment_steps ***  adi,fagc-lp-thresh-increment-steps
    /* Fast AGC - Lock Level (Lock Level is set via slow AGC inner high threshold) */
    1,      //fagc_lock_level_lmt_gain_increase_en ***  adi,fagc-lock-level-lmt-gain-increase-enable
    5,      //fagc_lock_level_gain_increase_upper_limit ***  adi,fagc-lock-level-gain-increase-upper-limit
    /* Fast AGC - Peak Detectors and Final Settling */
    1,      //fagc_lpf_final_settling_steps ***  adi,fagc-lpf-final-settling-steps
    1,      //fagc_lmt_final_settling_steps ***  adi,fagc-lmt-final-settling-steps
    3,      //fagc_final_overrange_count ***  adi,fagc-final-overrange-count
    /* Fast AGC - Final Power Test */
    0,      //fagc_gain_increase_after_gain_lock_en ***  adi,fagc-gain-increase-after-gain-lock-enable
    /* Fast AGC - Unlocking the Gain */
    0,      //fagc_gain_index_type_after_exit_rx_mode ***  adi,fagc-gain-index-type-after-exit-rx-mode
    1,      //fagc_use_last_lock_level_for_set_gain_en ***  adi,fagc-use-last-lock-level-for-set-gain-enable
    1,      //fagc_rst_gla_stronger_sig_thresh_exceeded_en ***  adi,fagc-rst-gla-stronger-sig-thresh-exceeded-enable
    5,      //fagc_optimized_gain_offset ***  adi,fagc-optimized-gain-offset
    10,     //fagc_rst_gla_stronger_sig_thresh_above_ll ***  adi,fagc-rst-gla-stronger-sig-thresh-above-ll
    1,      //fagc_rst_gla_engergy_lost_sig_thresh_exceeded_en ***  adi,fagc-rst-gla-engergy-lost-sig-thresh-exceeded-enable
    1,      //fagc_rst_gla_engergy_lost_goto_optim_gain_en ***  adi,fagc-rst-gla-engergy-lost-goto-optim-gain-enable
    10,     //fagc_rst_gla_engergy_lost_sig_thresh_below_ll ***  adi,fagc-rst-gla-engergy-lost-sig-thresh-below-ll
    8,      //fagc_energy_lost_stronger_sig_gain_lock_exit_cnt ***  adi,fagc-energy-lost-stronger-sig-gain-lock-exit-cnt
    1,      //fagc_rst_gla_large_adc_overload_en ***  adi,fagc-rst-gla-large-adc-overload-enable
    1,      //fagc_rst_gla_large_lmt_overload_en ***  adi,fagc-rst-gla-large-lmt-overload-enable
    0,      //fagc_rst_gla_en_agc_pulled_high_en ***  adi,fagc-rst-gla-en-agc-pulled-high-enable
    0,      //fagc_rst_gla_if_en_agc_pulled_high_mode ***  adi,fagc-rst-gla-if-en-agc-pulled-high-mode
    64,     //fagc_power_measurement_duration_in_state5 ***  adi,fagc-power-measurement-duration-in-state5
    /* RSSI Control */
    1,      //rssi_delay *** adi,rssi-delay
    1000,   //rssi_duration *** adi,rssi-duration
    3,      //rssi_restart_mode *** adi,rssi-restart-mode
    0,      //rssi_unit_is_rx_samples_enable *** adi,rssi-unit-is-rx-samples-enable
    1,      //rssi_wait *** adi,rssi-wait
    /* Aux ADC Control */
    256,    //aux_adc_decimation *** adi,aux-adc-decimation
    40000000UL, //aux_adc_rate *** adi,aux-adc-rate
    /* AuxDAC Control */
    1,      //aux_dac_manual_mode_enable ***  adi,aux-dac-manual-mode-enable
    0,      //aux_dac1_default_value_mV ***  adi,aux-dac1-default-value-mV
    0,      //aux_dac1_active_in_rx_enable ***  adi,aux-dac1-active-in-rx-enable
    0,      //aux_dac1_active_in_tx_enable ***  adi,aux-dac1-active-in-tx-enable
    0,      //aux_dac1_active_in_alert_enable ***  adi,aux-dac1-active-in-alert-enable
    0,      //aux_dac1_rx_delay_us ***  adi,aux-dac1-rx-delay-us
    0,      //aux_dac1_tx_delay_us ***  adi,aux-dac1-tx-delay-us
    0,      //aux_dac2_default_value_mV ***  adi,aux-dac2-default-value-mV
    0,      //aux_dac2_active_in_rx_enable ***  adi,aux-dac2-active-in-rx-enable
    0,      //aux_dac2_active_in_tx_enable ***  adi,aux-dac2-active-in-tx-enable
    0,      //aux_dac2_active_in_alert_enable ***  adi,aux-dac2-active-in-alert-enable
    0,      //aux_dac2_rx_delay_us ***  adi,aux-dac2-rx-delay-us
    0,      //aux_dac2_tx_delay_us ***  adi,aux-dac2-tx-delay-us
    /* Temperature Sensor Control */
    256,    //temp_sense_decimation *** adi,temp-sense-decimation
    1000,   //temp_sense_measurement_interval_ms *** adi,temp-sense-measurement-interval-ms
    0x0,   //temp_sense_offset_signed *** adi,temp-sense-offset-signed /////////////////////////////////// 0xCE
    1,      //temp_sense_periodic_measurement_enable *** adi,temp-sense-periodic-measurement-enable
    /* Control Out Setup */
    0xFF,   //ctrl_outs_enable_mask *** adi,ctrl-outs-enable-mask
    0,      //ctrl_outs_index *** adi,ctrl-outs-index
    /* External LNA Control */
    0,      //elna_settling_delay_ns *** adi,elna-settling-delay-ns
    0,      //elna_gain_mdB *** adi,elna-gain-mdB
    0,      //elna_bypass_loss_mdB *** adi,elna-bypass-loss-mdB
    0,      //elna_rx1_gpo0_control_enable *** adi,elna-rx1-gpo0-control-enable
    0,      //elna_rx2_gpo1_control_enable *** adi,elna-rx2-gpo1-control-enable
    0,      //elna_gaintable_all_index_enable *** adi,elna-gaintable-all-index-enable
    /* Digital Interface Control */
    0,      //digital_interface_tune_skip_mode *** adi,digital-interface-tune-skip-mode
    0,      //digital_interface_tune_fir_disable *** adi,digital-interface-tune-fir-disable
    0,      //pp_tx_swap_enable *** adi,pp-tx-swap-enable
    0,      //pp_rx_swap_enable *** adi,pp-rx-swap-enable
    0,      //tx_channel_swap_enable *** adi,tx-channel-swap-enable // not for ad9364
    0,      //rx_channel_swap_enable *** adi,rx-channel-swap-enable // not for ad9364
    1,      //rx_frame_pulse_mode_enable *** adi,rx-frame-pulse-mode-enable
    0,      //two_t_two_r_timing_enable *** adi,2t2r-timing-enable
    0,      //invert_data_bus_enable *** adi,invert-data-bus-enable
    0,      //invert_data_clk_enable *** adi,invert-data-clk-enable
    0,      //fdd_alt_word_order_enable *** adi,fdd-alt-word-order-enable
    0,      //invert_rx_frame_enable *** adi,invert-rx-frame-enable
    0,      //fdd_rx_rate_2tx_enable *** adi,fdd-rx-rate-2tx-enable
    0,      //swap_ports_enable *** adi,swap-ports-enable
    0,      //single_data_rate_enable *** adi,single-data-rate-enable
    0,      //lvds_mode_enable *** adi,lvds-mode-enable // our system is CMOS
    0,      //half_duplex_mode_enable *** adi,half-duplex-mode-enable
    0,      //single_port_mode_enable *** adi,single-port-mode-enable
    1,      //full_port_enable *** adi,full-port-enable
    0,      //full_duplex_swap_bits_enable *** adi,full-duplex-swap-bits-enable
    0,      //delay_rx_data *** adi,delay-rx-data
    0,      //rx_data_clock_delay *** adi,rx-data-clock-delay
    4,      //rx_data_delay *** adi,rx-data-delay
    7,      //tx_fb_clock_delay *** adi,tx-fb-clock-delay
    0,      //tx_data_delay *** adi,tx-data-delay
#ifdef ALTERA_PLATFORM
    300,    //lvds_bias_mV *** adi,lvds-bias-mV
#else
    150,    //lvds_bias_mV *** adi,lvds-bias-mV
#endif
    0,      //lvds_rx_onchip_termination_enable *** adi,lvds-rx-onchip-termination-enable
    0,      //rx1rx2_phase_inversion_en *** adi,rx1-rx2-phase-inversion-enable
    0xFF,   //lvds_invert1_control *** adi,lvds-invert1-control
    0x0F,   //lvds_invert2_control *** adi,lvds-invert2-control
    /* GPO Control */
    0,      //gpo0_inactive_state_high_enable *** adi,gpo0-inactive-state-high-enable
    0,      //gpo1_inactive_state_high_enable *** adi,gpo1-inactive-state-high-enable
    0,      //gpo2_inactive_state_high_enable *** adi,gpo2-inactive-state-high-enable
    0,      //gpo3_inactive_state_high_enable *** adi,gpo3-inactive-state-high-enable
    0,      //gpo0_slave_rx_enable *** adi,gpo0-slave-rx-enable
    0,      //gpo0_slave_tx_enable *** adi,gpo0-slave-tx-enable
    0,      //gpo1_slave_rx_enable *** adi,gpo1-slave-rx-enable
    0,      //gpo1_slave_tx_enable *** adi,gpo1-slave-tx-enable
    0,      //gpo2_slave_rx_enable *** adi,gpo2-slave-rx-enable
    0,      //gpo2_slave_tx_enable *** adi,gpo2-slave-tx-enable
    0,      //gpo3_slave_rx_enable *** adi,gpo3-slave-rx-enable
    0,      //gpo3_slave_tx_enable *** adi,gpo3-slave-tx-enable
    0,      //gpo0_rx_delay_us *** adi,gpo0-rx-delay-us
    0,      //gpo0_tx_delay_us *** adi,gpo0-tx-delay-us
    0,      //gpo1_rx_delay_us *** adi,gpo1-rx-delay-us
    0,      //gpo1_tx_delay_us *** adi,gpo1-tx-delay-us
    0,      //gpo2_rx_delay_us *** adi,gpo2-rx-delay-us
    0,      //gpo2_tx_delay_us *** adi,gpo2-tx-delay-us
    0,      //gpo3_rx_delay_us *** adi,gpo3-rx-delay-us
    0,      //gpo3_tx_delay_us *** adi,gpo3-tx-delay-us
    /* Tx Monitor Control */
    37000,  //low_high_gain_threshold_mdB *** adi,txmon-low-high-thresh
    0,      //low_gain_dB *** adi,txmon-low-gain
    24,     //high_gain_dB *** adi,txmon-high-gain
    0,      //tx_mon_track_en *** adi,txmon-dc-tracking-enable
    0,      //one_shot_mode_en *** adi,txmon-one-shot-mode-enable
    511,    //tx_mon_delay *** adi,txmon-delay
    8192,   //tx_mon_duration *** adi,txmon-duration
    2,      //tx1_mon_front_end_gain *** adi,txmon-1-front-end-gain
    2,      //tx2_mon_front_end_gain *** adi,txmon-2-front-end-gain
    48,     //tx1_mon_lo_cm *** adi,txmon-1-lo-cm
    48,     //tx2_mon_lo_cm *** adi,txmon-2-lo-cm
    /* GPIO definitions */
    -1,     //gpio_resetb *** reset-gpios
    /* MCS Sync */
    -1,     //gpio_sync *** sync-gpios
    -1,     //gpio_cal_sw1 *** cal-sw1-gpios
    -1,     //gpio_cal_sw2 *** cal-sw2-gpios
    /* External LO clocks */
    NULL,   //(*ad9361_rfpll_ext_recalc_rate)()
    NULL,   //(*ad9361_rfpll_ext_round_rate)()
    NULL    //(*ad9361_rfpll_ext_set_rate)()
};


AD9361_RXFIRConfig rx_fir_config = {
    3, // rx
    0, // rx_gain
    1, // rx_dec
    {100,234,-280,-138,477,30,-833,428,1062,-1220,-1053,2556,410,-4964,2197,17523,17523,2197,-4964,410,2556,-1053,-1220,1062,428,-833,30,477,-138,-280,234,100,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // rx_coef[128]
    32, // rx_coef_size
    {1280000000,160000000,80000000,40000000,40000000,40000000}, // rx_path_clks[6]
    28817730 // rx_bandwidth
};

AD9361_TXFIRConfig tx_fir_config = {
    3, // tx
    0, // tx_gain
    1, // tx_int
    {112,193,-245,-116,433,-4,-724,408,913,-1119,-862,2299,212,-4425,2594,16861,16861,2594,-4425,212,2299,-862,-1119,913,408,-724,-4,433,-116,-245,193,112,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // tx_coef[128]
    32, // tx_coef_size
    {1280000000,160000000,80000000,40000000,40000000,40000000}, // tx_path_clks[6]
    29417983 // tx_bandwidth
};

struct ad9361_rf_phy *ad9361_phy;

int main()
{
    default_init_param.gpio_resetb = GPIO_PIN_AD9361_RESET;
    default_init_param.gpio_sync = -1;
    default_init_param.gpio_cal_sw1 = -1;
    default_init_param.gpio_cal_sw2 = -1;

//    RST.out(1);
//    usleep(10000);
//    RST.out(0);
//    usleep(10000);
//    RST.out(1);
//    usleep(10000);

    uint8_t mode = 0x01;
    int res = ioctl(GetFD(), SPI_IOC_WR_MODE, &mode);

    printf("set mode to 0x%X, res=%d\n", mode, res);

    uint8_t buf[4];
    uint8_t buf2[4];
    int32_t ret;

    uint16_t cmd;
    cmd = AD_READ | AD_CNT(1) | AD_ADDR(REG_PRODUCT_ID);
    buf[0] = cmd >> 8;
    buf[1] = cmd & 0xFF;

   // ret = spi_write_then_read(spi, &buf[0], 2, rbuf, num);

    //ret = ad9361_spi_readm(spi, REG_TEMPERATURE, &buf, 1);
    transmitData((uint8_t*)&buf, (uint8_t*)&buf2, 3);

    default_init_param.dev_sel = ID_AD9364;
    uint8_t Status = ad9361_init(&ad9361_phy, &default_init_param);
    if(Status){
            printf("ad9361_init exited with error: %0d \r\n",Status);
    }
    // Config custom ad9364 fir settings
    Status = ad9361_set_tx_fir_config(ad9361_phy, tx_fir_config);
    if(Status){
            printf("ad9361_set_tx_fir_config exited with error: %0d \r\n",Status);
    }
    Status = ad9361_set_rx_fir_config(ad9361_phy, rx_fir_config);
    if(Status){
    printf("ad9361_set_rx_fir_config exited with error: %0d \r\n",Status);
    }

    // Turn on ad9364 fir
    Status = ad9361_set_tx_fir_en_dis(ad9361_phy, 1);
    if(Status){
            printf("ad9361_set_tx_fir_en_dis exited with error: %0d \r\n",Status);
    }
    Status = ad9361_set_rx_fir_en_dis(ad9361_phy, 1);
    if(Status){
            printf("ad9361_set_tx_fir_en_dis exited with error: %0d \r\n",Status);
    }

    uint32_t confreg0 = ad9361_spi_read(ad9361_phy->spi, REG_PARALLEL_PORT_CONF_1);
// swap channel for predict spetral inversion
    confreg0 |= PP_RX_SWAP_IQ | PP_TX_SWAP_IQ;
    ad9361_spi_write(ad9361_phy->spi, REG_PARALLEL_PORT_CONF_1,confreg0);
    return 0;
}
