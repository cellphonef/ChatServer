aux_source_directory(. SRC_LIST)
aux_source_directory(db DB_LIST)
aux_source_directory(dao DAO_LIST)
aux_source_directory(redis REDIS_LIST)


add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${DAO_LIST} ${REDIS_LIST})

target_link_libraries(ChatServer mymuduo pthread mysqlclient hiredis)