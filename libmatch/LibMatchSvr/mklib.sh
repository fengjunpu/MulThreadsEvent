g++  -I./include -I./dep/hiredis/include  -fPIC -shared -o ./lib/libmatch.so ./src/match.cpp ./src/redis_wrap.cpp -Wl,--whole-archive ./dep/hiredis/lib64/libhiredis.a -Wl,--no-whole-archive

#rm ./xmmatch -rf
#g++ -I./include ./lib/libmatch.so ./matchSvr/match_server.cpp -o matchSvr
