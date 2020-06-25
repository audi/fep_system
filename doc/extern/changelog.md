# FEP SDK System Library Changelog {#fep_sdk_system_change_log}
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0) and this project adheres to [Semantic Versioning](https://semver.org/lang/en).


Release Notes - FEP System Library - Version 2.6.1

## [2.6.1] - Mar-2020

### Bug
    * [FEPSDK-2204] - 2.6.0 demo_system_example is not configurable, cmake layer defect
    * [FEPSDK-2243] - usage of different fep::System instances and usage of const methods in parallel leads to a crash 

### Change
	* [FEPSDK-1314] - Prepare FEP SDK code to be distributed as OSS
    * [FEPSDK-2022] - Cleanup CMake and conan as preparation for the open source distribution

Release Notes - FEP System Library - Version 2.6.0

## [2.6.0] - Jan-2020

### Bug
    * [FEPSDK-2017] - Setting a property which does not exist leads to a participant crash
    * [FEPSDK-2058] - It is not possible to set timing property
    * [FEPSDK-2059] - Syntax of Property Values to set and get is not clearly defined, only slahes '/' are allowed! 

### Change
    * [FEPSDK-1920] - Copy or move the configureTiming and the configureSystemProperties functionality from controller lib to system lib
    * [FEPSDK-1984] - Use fep3::System instead of std::unique_ptr<fep3::System> for connectSystem() as return value
    * [FEPSDK-2049] - Add hint within the documentation of RPC Data Registry Service when signals can be obtained
    * [FEPSDK-2051] - if setting system and system timing properties fails a std::runtime_error will be thrown

Release Notes - FEP System Library - Version 2.5.1

## [2.5.1] - Oct-2019

### Bug
    * [FEPSDK-1320] - Wrong include in data_registry_rpc_intf.h

### Change
     * [FEPSDK-1326] - Create own FEP System Library (Repo + Package + Jobs)
                       FEP System Library is now a separated delivery package fep_sdk_system/2.5.1@aev25/*
     * [FEPSDK-1087] - [PropertyTree] Add To System Lib - Rework public interface to IRPCPropertyTree 
     * [FEPSDK-1321] - get rid of SystemLogger in public API
     * [FEPSDK-1327] - Move FEP SDK - System Code to the Git repo created
