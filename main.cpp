#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


#include <windows.h>

#include "igcl_api.h"

int main() {
  ctl_init_args_t CtlInitArgs{};
  ctl_api_handle_t hAPIHandle;
  CtlInitArgs.AppVersion =
      CTL_MAKE_VERSION(CTL_IMPL_MAJOR_VERSION, CTL_IMPL_MINOR_VERSION);
  CtlInitArgs.flags = 0;
  CtlInitArgs.Size = sizeof(CtlInitArgs);
  CtlInitArgs.Version = 0;

  auto result = ctlInit(&CtlInitArgs, &hAPIHandle);

  if (result != CTL_RESULT_SUCCESS) {
    printf("ctlInit failed with error code: %08x\n", result);
    return 1;
  }

  uint32_t count = 0;
  result = ctlEnumerateDevices(hAPIHandle, &count, nullptr);
  if (result != CTL_RESULT_SUCCESS) {
    printf("ctlEnumerateDevices failed with error code: %08x\n", result);
    ctlClose(hAPIHandle);
    return 1;
  }

  std::vector<ctl_device_adapter_handle_t> devices(count);
  result = ctlEnumerateDevices(hAPIHandle, &count, devices.data());
  if (result != CTL_RESULT_SUCCESS) {
    printf("ctlEnumerateDevices failed with error code: %08x\n", result);
    ctlClose(hAPIHandle);
    return 1;
  }

  printf("Number of devices: %u\n", count);
  for (uint32_t i = 0; i < count; ++i) {
    ctl_device_adapter_handle_t deviceHandle = devices[i];
    ctl_device_adapter_properties_t properties{};
    properties.Size = sizeof(properties);
    result = ctlGetDeviceProperties(deviceHandle, &properties);
    if (result != CTL_RESULT_SUCCESS) {
      printf("ctlGetDeviceProperties failed for device %u with error code: %08x\n",
             i, result);
      continue;
    }
    printf("Device %u: %s\n", i, properties.name);
    ctl_retro_scaling_caps_t caps{};
    caps.Size = sizeof(caps);
    result = ctlGetSupportedRetroScalingCapability(deviceHandle, &caps);
    if (result != CTL_RESULT_SUCCESS) {
      printf("ctlGetSupportedRetroScalingCapability failed for device %u with "
             "error code: %08x\n",
             i, result);
      continue;
    }
    std::string capNames;
    if (caps.SupportedRetroScaling & CTL_RETRO_SCALING_TYPE_FLAG_INTEGER) {
      capNames += "integer ";
    }
    if (caps.SupportedRetroScaling & CTL_RETRO_SCALING_TYPE_FLAG_NEAREST_NEIGHBOUR) {
      capNames += "nearest ";
    }
    if (capNames.empty()) {
      capNames = "none";
    }
    printf("Supported retro scaling types: %s\n", capNames.c_str());
    if (caps.SupportedRetroScaling) {
      ctl_retro_scaling_settings_t settings{};
      settings.Size = sizeof(settings);
      settings.Get = true;
      result = ctlGetSetRetroScaling(deviceHandle, &settings);
      if (result != CTL_RESULT_SUCCESS) {
        printf(
            "ctlGetSetRetroScaling failed for device %u with error code: %08x\n",
            i, result);
        continue;
      }
      printf("Device %u retro scaling settings: enabled=%d, type=%d\n", i,
             settings.Enable, settings.RetroScalingType);
      printf("Enter new type for device %u (0: off, 1: integer, 2: nearest, empty: skip): ",
             i);
      std::string input;
      std::getline(std::cin, input);
      int newType = -1;
      std::istringstream iss(input);
      iss >> newType;
      if (iss) {
        switch(newType){
          case 0:
            settings.Enable = false;
            settings.RetroScalingType = CTL_RETRO_SCALING_TYPE_FLAG_INTEGER;
            break;
          case 1:
            settings.Enable = true;
            settings.RetroScalingType = CTL_RETRO_SCALING_TYPE_FLAG_INTEGER;
            break;
          case 2:
            settings.Enable = true;
            settings.RetroScalingType = CTL_RETRO_SCALING_TYPE_FLAG_NEAREST_NEIGHBOUR;
            break;
          default:
            printf("Invalid type. Skipping device %u.\n", i);
          continue;
        }
        settings.Get = false;
        settings.Size = sizeof(settings);
        result = ctlGetSetRetroScaling(deviceHandle, &settings);
        if (result != CTL_RESULT_SUCCESS) {
          printf("ctlGetSetRetroScaling failed to set new type for device %u "
                 "with error code: %08x\n",
                 i, result);
        } else {
          printf("Device %u retro scaling type set to %d successfully.\n", i,
                 newType);
        }
      } else {
        printf("Skipping device %u.\n", i);
      }
    }
  }
  ctlClose(hAPIHandle);

  return 0;
}
