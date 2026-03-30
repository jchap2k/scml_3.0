@REM *************************************************************************
@REM Copyright 1996-2025 Synopsys, Inc.
@REM
@REM This Synopsys software and all associated documentation are proprietary
@REM to Synopsys, Inc. and may only be used pursuant to the terms and
@REM conditions of a written license agreement with Synopsys, Inc.
@REM All other use, reproduction, modification, or distribution of the
@REM Synopsys software or the associated documentation is strictly prohibited.
@REM *************************************************************************
::
:: batch file to copy header files 
::
@ECHO OFF 
echo Moving scml2.h to ..\src
move ..\src\scml2\scml2.h ..\src
echo Moving scml2_logging.h to ..\src
move ..\src\scml2_logging\scml2_logging.h ..\src
echo Moving scml2_base.h to ..\src
move ..\src\scml2_base\scml2_base.h ..\src
echo Moving scml2_objects.h to ..\src
move ..\src\scml2_objects\scml2_objects.h ..\src
echo Moving scml2_tlm2.h to ..\src
move ..\src\scml2_tlm2\scml2_tlm2.h ..\src
echo Moving protocol_engines to include
set pe=..\src\scml2_protocol_engines
mkdir %pe%\clock\include
move %pe%\clock\*.h %pe%\clock\include
mkdir %pe%\reset\include
move %pe%\reset\*.h %pe%\reset\include
mkdir %pe%\pin\include
move %pe%\pin\*.h %pe%\pin\include
mkdir %pe%\interrupt\include
move %pe%\interrupt\*.h %pe%\interrupt\include
mkdir %pe%\tlm2_ft_common\include
move %pe%\tlm2_ft_common\*.h %pe%\tlm2_ft_common\include
mkdir %pe%\tlm2_ft_initiator_port\include
move %pe%\tlm2_ft_initiator_port\*.h %pe%\tlm2_ft_initiator_port\include
mkdir %pe%\tlm2_ft_target_port\include
move %pe%\tlm2_ft_target_port\*.h %pe%\tlm2_ft_target_port\include

