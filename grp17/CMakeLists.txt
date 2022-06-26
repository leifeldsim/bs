cmake_minimum_required(VERSION 3.20.2)
project(Projekt C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wconversion -Wextra -lpthread -lrt ")

include_directories(.)

add_executable(testNoInit tests/No_init.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h tests/Error_handling_OSMPLib/error_handling_OSMPLib.c tests/Error_handling_OSMPLib/error_handling_OSMPLib.h)
target_link_libraries(testNoInit rt pthread)

add_executable(echoallBcast tests/Bcast.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h tests/Error_handling_OSMPLib/error_handling_OSMPLib.c tests/Error_handling_OSMPLib/error_handling_OSMPLib.h)
target_link_libraries(echoallBcast rt pthread)

add_executable(echoallFinalizeAndReinit tests/FinalizeAndReinit.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h tests/Error_handling_OSMPLib/error_handling_OSMPLib.c tests/Error_handling_OSMPLib/error_handling_OSMPLib.h)
target_link_libraries(echoallFinalizeAndReinit rt pthread)

add_executable(echoallSendRecvTooManyMessages tests/SendRecvTooManyMessages.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h tests/Error_handling_OSMPLib/error_handling_OSMPLib.c tests/Error_handling_OSMPLib/error_handling_OSMPLib.h)
target_link_libraries(echoallSendRecvTooManyMessages rt pthread)

add_executable(test_onlysend tests/Onlysend.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h tests/Error_handling_OSMPLib/error_handling_OSMPLib.c tests/Error_handling_OSMPLib/error_handling_OSMPLib.h)
target_link_libraries(test_onlysend rt pthread)

add_executable(echoallISendRecvNonBlocking tests/ISendRecv.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h tests/Error_handling_OSMPLib/error_handling_OSMPLib.c tests/Error_handling_OSMPLib/error_handling_OSMPLib.h)
target_link_libraries(echoallISendRecvNonBlocking rt pthread)

add_executable(echoallSendRecvBlocking tests/SendRecv.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h tests/Error_handling_OSMPLib/error_handling_OSMPLib.c tests/Error_handling_OSMPLib/error_handling_OSMPLib.h)
target_link_libraries(echoallSendRecvBlocking rt pthread)

add_executable(osmprun OSMPRun/OSMPRun.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h)
target_link_libraries(osmprun rt pthread)

add_executable(test_Bcast_gegeben tests/Bcast_vom_Prof.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h tests/Error_handling_OSMPLib/error_handling_OSMPLib.c tests/Error_handling_OSMPLib/error_handling_OSMPLib.h)
target_link_libraries(test_Bcast_gegeben rt pthread)

add_executable(test_SendRecv_gegeben tests/SendRecv_vom_Prof.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h)
target_link_libraries(test_SendRecv_gegeben rt pthread)

add_executable(test_SendIrecv_gegeben tests/SendIrecv_vom_Prof.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h)
target_link_libraries(test_SendIrecv_gegeben rt pthread)

add_executable(testBarrier tests/Barrier.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h tests/Error_handling_OSMPLib/error_handling_OSMPLib.c tests/Error_handling_OSMPLib/error_handling_OSMPLib.h)
target_link_libraries(testBarrier rt pthread)

add_executable(test_wait_for_max_msg tests/Wait_for_max_msg.c OSMPLib/OSMPLib.c OSMPLib/OSMPLib.h tests/Error_handling_OSMPLib/error_handling_OSMPLib.c tests/Error_handling_OSMPLib/error_handling_OSMPLib.h)
target_link_libraries(test_wait_for_max_msg rt pthread)