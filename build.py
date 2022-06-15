#!/usr/bin/env python

import sys
import os
import datetime
import platform
import subprocess
import shutil

# MDK build
mdk_build_command = 'D:/Keil_v5/UV4/UV4.exe -j0 -r ./MDK-ARM/STM32L0_BMA253.uvprojx -o build_log.txt'
mdk_build_log     = 'cat ./MDK-ARM/build_log.txt'

# build out file
gcc_source_file_hex = 'build/STM32L0_BMA253.hex'
gcc_source_file_bin = 'build/STM32L0_BMA253.bin'
mdk_source_file_hex = 'MDK-ARM/STM32L0_BMA253/STM32L0_BMA253.hex'
mdk_source_file_bin = 'MDK-ARM/STM32L0_BMA253/STM32L0_BMA253.bin'
target_path = 'user/output'

# openocd define
stlink_config_file = "user/openocd/stlink.cfg"
chip_config_file = "user/openocd/stm32l0.cfg"
program_cmd = "\"program user/output/STM32L0_BMA253.bin 0x8000000\""

def openocd_run():
    host_os = platform.system()
    print("host os: %s" % host_os)

    if host_os == 'Windows':
        openocd_exe = "openocd.exe"
    elif host_os == 'Linux':
        openocd_exe = "openocd"
    else:
        print("unsupport platform")
        sys.exit(2)

    exec_cmd = "%s -f %s -f %s -c init -c halt -c %s -c reset -c shutdown\n" % (openocd_exe, stlink_config_file, chip_config_file, program_cmd)
    print("execute cmd: %s" % exec_cmd)
    subprocess.call(exec_cmd, shell=True)

# JLink define
jlink_loadfile = 'user/output/STM32L0_BMA253.hex'
device = "STM32L0"
interface = "SWD"
speed = "1000"
address = "0x8000000"
jlink_cmdfile = ""

def gen_jlink_cmdfile(loadfile):
    global address, jlink_cmdfile

    dirname = os.path.dirname(os.path.abspath(loadfile))
    jlink_cmdfile = os.path.join(dirname, "%s.jlink" % device)
    print("gen jlink commands file: %s" % jlink_cmdfile)

    try:
        f = open(jlink_cmdfile, 'w')
        f.write("h\n")
        f.write("loadfile %s %s\n" % (loadfile, address))
        f.write("g\n")
        f.write("r\n")
        f.write("qc")
    except IOError:
        print(IOError)  # will print something like "option -a not recognized"
        sys.exit(2)

def jlink_run(loadfile):
    global device, interface, speed, jlink_cmdfile

    host_os = platform.system()
    print("host os: %s" % host_os)
    gen_jlink_cmdfile(loadfile)

    if host_os == 'Windows':
        jlink_exe = "JLink.exe"
    elif host_os == 'Linux':
        jlink_exe = "JLinkExe"
    else:
        print("unsupport platform")
        sys.exit(2)

    exec_cmd = "%s -device %s -if %s -speed %s -CommanderScript %s" % (jlink_exe, device, interface, speed, jlink_cmdfile)
    print("execute cmd: %s" % exec_cmd)
    subprocess.call(exec_cmd, shell=True)

def cp_build_file(source, target):
    assert not os.path.isabs(source)

    try:
        shutil.copy(source, target)
    except IOError as e:
        print("Unable to copy file. %s" % e)
    except:
        print("Unexpected error:", sys.exc_info())

def gcc_build():
    print("cc type = gcc")
    os.system('make clean')
    os.system('make -j8')
    cp_build_file(gcc_source_file_hex, target_path)
    cp_build_file(gcc_source_file_bin, target_path)

def mdk_build():
    print("cc type = MDK")
    os.system(mdk_build_command)
    os.system(mdk_build_log)
    cp_build_file(mdk_source_file_hex, target_path)
    cp_build_file(mdk_source_file_bin, target_path)

def main_func(parameter):
    start = datetime.datetime.now()

    if parameter == 'gcc':
        gcc_build()
    elif parameter == 'mdk':
        mdk_build()
    elif parameter == 'jlink':
        jlink_run(jlink_loadfile)
    elif parameter == 'openocd':
        openocd_run()
    else:
        print("input parameter error!")

    end = datetime.datetime.now()
    print('run time: %s second' %(end - start))

if __name__ == "__main__":
    main_func(sys.argv[1])