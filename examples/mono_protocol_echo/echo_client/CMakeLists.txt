cmake_minimum_required (VERSION 3.2.1)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 

#Multi-threaded Debug (/MTd)

find_package(Boost
	1.57.0				# Minimum version
	REQUIRED            # Fail with error if Boost is not found
	COMPONENTS date_time	# Boost libraries by their canonical name
#	COMPONENTS system	# Boost libraries by their canonical name
  )

include_directories(BEFORE ./../../.. ${Boost_INCLUDE_DIRS} )

link_directories(${Boost_LIBRARY_DIRS})

#link_libraries(echo_client ${Boost_LIBRARIES})

add_executable (echo_client client.cpp)

# /*
# créé par moi:
# /GS /analyze- /W3 /Zc:wchar_t /ZI /Gm /Od /sdl /Fd"Debug\vc120.pdb" /fp:precise 
# /D "_MBCS" /errorReport:prompt /WX /Zc:forScope /RTC1 /Gd /Oy- /MDd /Fa"Debug\" 
# /EHsc /nologo /Fo"Debug\" /Fp"Debug\echo_client.pch" 

# OUT:"C:\Users\IBM_ADMIN\MyDevelopments\project_editors\splice\Debug\echo_client.exe" 
# /MANIFEST /NXCOMPAT 
# /PDB:"C:\Users\IBM_ADMIN\MyDevelopments\project_editors\splice\Debug\echo_client.pdb" 
# /DYNAMICBASE "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" 
# "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" 
# "uuid.lib" "odbc32.lib" "odbccp32.lib" /DEBUG /MACHINE:X86 /INCREMENTAL 
# /PGD:"C:\Users\IBM_ADMIN\MyDevelopments\project_editors\splice\Debug\echo_client.pgd" 
# /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"Debug\echo_client.exe.intermediate.manifest" /ERRORREPORT:PROMPT /NOLOGO /TLBID:1 

# cmake

# */



