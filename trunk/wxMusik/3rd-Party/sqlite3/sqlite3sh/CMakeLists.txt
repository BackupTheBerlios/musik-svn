
ADD_DEFINITIONS(-DHAVE_READLINE)

ADD_EXECUTABLE(sqlite3sh
	../sqlite3.c
	../shell.c
)

TARGET_LINK_LIBRARIES(sqlite3sh 
	pthread
	readline
	tcmalloc
)       
