/*
 *  tetra-kit
 *  Copyright (C) 2020  LarryTh <dev@logami.fr>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef TETRA_COMMON_H
#define TETRA_COMMON_H
#include <cstdint>

/**
 * @defgroup tetra_common TETRA common declarations
 *
 * @{
 *
 */

/**
 * @brief Burst type
 *
 * 9.4.4 - type of bursts
 *
 * NOTE:
 *   - we only decode continuous downlink bursts
 *
 */

enum burst_t {
    SB     = 0,                                                                 // synchronisation downlink burst 9.4.4.3.4
    NDB    = 1,                                                                 // 1 logical channel in time slot TCH or SCH/F
    NDB_SF = 2                                                                  // 2 logical channels in time slot STCH+TCH or STCH+STCH or SCH/HD+SCH/HD or SCH/HD+BNCH 9.4.4.3.2
};

/**
 * @brief Logical channels enum
 *
 */

enum mac_logical_channel_t {                                                    // CP only
    AACH   = 0,
    BLCH   = 1,
    BNCH   = 2,
    BSCH   = 3,
    SCH_F  = 4,
    SCH_HD = 5,
    STCH   = 6,
    TCH_S  = 7,
    TCH    = 8,
    unkown = 9
};

/**
 * @brief Tetra sub-system synchronisation - ch. 7
 *
 */

struct tetra_time_t {
    uint16_t fn;                                                                ///< frame number
    uint16_t mn;                                                                ///< multi-frame number
    uint16_t tn;                                                                ///< time slot
};

/**
 * @brief Tetra cell information
 *
 */

struct tetra_cell_infos_t {
    uint16_t color_code;                                                        ///< Cell Color code
    uint32_t mcc;                                                               ///< Cell MCC
    uint32_t mnc;                                                               ///< Cell MNC
    uint32_t scrambling_code;                                                   ///< Cell Scrambling code

    int32_t downlink_frequency;                                                 ///< Downlink frequency [Hz]
    int32_t uplink_frequency;                                                   ///< Uplink frequency [Hz]
};

/**
 * @brief Tetra MAC address
 *
 * Contains the current burst address state (21.4.3.1)
 *
 */

struct mac_address_t {
    uint8_t  address_type;
    uint8_t  event_label;
    uint8_t  encryption_mode;
    uint8_t  usage_marker;
    uint8_t  stolen_flag;
    uint32_t smi;
    uint32_t ssi;
    uint32_t ussi;
};

/**
 * @brief Downlink usage values 21.4.7.2 table 21.77
 *
 */

enum downlink_usage_t {
    UNALLOCATED      = 0,
    ASSIGNED_CONTROL = 1,
    COMMON_CONTROL   = 2,
    RESERVED         = 3,
    TRAFFIC          = 4                                                        // traffic usage marker assigned in MAC-RESOURCE PDU
};

/**
 * @brief Contains the current MAC informations for routing to logical channels
 *
 */

struct mac_state_t {
    downlink_usage_t      downlink_usage;                                       ///< Downlink usage type
    uint32_t              downlink_usage_marker;                                ///< Downlink usage marker
    mac_logical_channel_t logical_channel;                                      ///< Current logical channel
};

/** @} */

#endif /* TETRA_COMMON_H */
