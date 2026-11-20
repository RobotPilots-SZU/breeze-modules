# Copyright (c) 2025 RobotPilots-SZU
# SPDX-License-Identifier: Apache-2.0
#
# breeze-modules: robot application modules (chassis, gimbal, ...).
#
# These modules are intentionally Zephyr-bound: they include Zephyr drivers
# and the device tree directly. They communicate with each other over zenoh
# topics and use breeze-middleware (protocol/osal) for serialization/logging.
#
# This file is only processed by the Zephyr build system via the module
# mechanism. There is no stand-alone build for this repository.

if(CONFIG_BREEZE_MODULES)

  zephyr_include_directories(include)

  add_subdirectory(modules/models)
  add_subdirectory_ifdef(CONFIG_BREEZE_MODULE_MSGPIPE modules/msgpipe)
  add_subdirectory_ifdef(CONFIG_BREEZE_MODULE_CHASSIS modules/chassis)
  add_subdirectory_ifdef(CONFIG_BREEZE_MODULE_REFEREE modules/referee)
  add_subdirectory_ifdef(CONFIG_BREEZE_MODULE_COMMUNICATOR modules/communicator)
  # add_subdirectory_ifdef(CONFIG_BREEZE_MODULE_GIMBAL  modules/gimbal)
  # add_subdirectory_ifdef(CONFIG_BREEZE_MODULE_SHOOTER modules/shooter)

endif()
