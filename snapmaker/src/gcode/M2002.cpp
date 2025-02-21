/*
 * Snapmaker2-Controller Firmware
 * Copyright (C) 2019-2020 Snapmaker [https://github.com/Snapmaker]
 *
 * This file is part of Snapmaker2-Controller
 * (see https://github.com/Snapmaker/Snapmaker2-Controller)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "snapmaker.h"
#include "service/system.h"
#include "src/gcode/gcode.h"
#include "../module/toolhead_laser.h"

/*
* M2020 : 10w 20w 40w laser control
 *   S[bool]   0 show security status , 1 get security status from module
 *   L[bool]   set auto focus light, 0-OFF 1-ON
 *   Y[uint32] set online sync Id
 *   G[NULL]   get online sync Id
 *   T[int8]   set protect temperature
**/

void GcodeSuite::M2002() {
  if (ModuleBase::toolhead() == MODULE_TOOLHEAD_LASER) {
    return;
  }
  const bool seen_s = parser.seen('S');
  if (seen_s) {
    uint8_t state = (uint8_t)parser.byteval('S', (uint8_t)0);
    if (state) {
      // Proactively request module security status once
      SSTP_Event_t event;
      event.op_code = 2;
      event.data = NULL;
      event.length = 0;
      event.id = 9;
      SERIAL_ECHOLN("Get security status");
      laser->GetSecurityStatus(event);
    } else {
      // The state is synchronized with the module, so you can send the state directly
      laser->TellSecurityStatus();
    }
  }

  const bool seen_l = parser.seenval('L');
  if (seen_l) {
    uint8_t state = (uint8_t)parser.byteval('L', (uint8_t)0);
    SSTP_Event_t event;
    event.op_code = 2;
    event.data = &state;
    event.length = 1;
    event.id = 9;

    laser->SetAutoFocusLight(event);
  }

  const bool seen_y = parser.seenval('Y');
  if (seen_y) {
    uint32_t id = (uint32_t)parser.ulongval('Y', (uint32_t)0);
    SSTP_Event_t event;
    event.op_code = 2;
    event.data = (uint8_t *)&id;
    event.length = 4;
    event.id = 9;

    laser->SetOnlineSyncId(event);
  }

  const bool seen_g = parser.seen('G');
  if (seen_g) {
    SSTP_Event_t event;
    event.op_code = 2;
    event.data = NULL;
    event.length = 0;
    event.id = 9;

    laser->GetOnlineSyncId(event);
  }

  const bool seen_t = parser.seenval('T');
  if (seen_t) {
    uint8_t test_cmd_code = (uint8_t)parser.byteval('T', 0);
    switch (test_cmd_code)
    {
      // Log laser info
      case 0:
      {
        laser->PrintInfo();
        break;
      }

      // Set laser power
      case 1:
      {
        float power = (float)parser.floatval('P', 0.0);
        laser->SetOutput(power);
        break;
      }

      // Set fan
      case 2:
      {
        uint8_t fs = parser.byteval('P', 0);
        laser->SetFanPower(fs);
        break;
      }

      // Set crosslight
      case 3:
      {
        uint8_t sw = parser.byteval('P', 0);
        laser->SetCrossLightCAN(sw);
        break;
      }

      // Get crosslight
      case 4:
      {
        bool sw;
        if (E_SUCCESS == laser->GetCrossLightCAN(sw)) {
          LOG_I("crosslight: %d\n", sw);
        }
        else {
          LOG_E("Can not got crosslight state\n");
        }
        break;
      }

      // set fire sensor sensitivity
      case 5:
      {
        uint8 fss = parser.byteval('P', 0);
        laser->SetFireSensorSensitivityCAN(fss);
        break;
      }

      // Get fire sensor sensitivity
      case 6:
      {
        uint8 fss;
        if (E_SUCCESS == laser->GetFireSensorSensitivityCAN(fss)) {
          LOG_I("fire sensor sensitivity: %d\n", fss);
        }
        else {
          LOG_E("Can not got fire sensor sensitivity\n");
        }
        break;
      }

      // set crosslight offset
      case 7:
      {
        float x, y;
        x = parser.floatval('X', 0);
        y = parser.floatval('Y', 0);
        laser->SetCrossLightOffsetCAN(x, y);
        break;
      }

      // Get crosslight offset
      case 8:
      {
        float x, y;
        if (E_SUCCESS == laser->GetCrossLightOffsetCAN(x, y)) {
          LOG_I("crosslight offset x %f, y %f\n", x, y);
        }
        else {
          LOG_E("Can not got crosslight offset\n");
        }
        break;
      }

      // set fire sensor rawdata report time
      case 9:
      {
        uint16 itv = parser.ushortval('P', 0);
        laser->SetFireSensorReportTime(itv);
        break;
      }

      // HMI get crosslight offset test
      case 10:
      {
        SSTP_Event_t e;
        if (E_SUCCESS == laser->GetCrosslightOffset(e)) {
          LOG_I("Send to HMI\n");
        }
        else {
          LOG_E("Can not got crosslight offset\n");
        }
        break;
      }

      default:
      {
              break;
      }
    }
  }


}
