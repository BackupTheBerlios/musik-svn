call "%VS71COMNTOOLS%\..\..\vc7\bin\vcvars32.bat"

set BUILDARG=%1

if _%1==_ set BUILDARG=build

devenv /%BUILDARG% "Debug Unicode" ./Musik.sln
devenv /%BUILDARG% "Debug" ./Musik.sln
devenv /%BUILDARG% "Release Unicode" ./Musik.sln
devenv /%BUILDARG% "Release" ./Musik.sln

