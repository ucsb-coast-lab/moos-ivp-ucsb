c++ -DHAVE_CONFIG_H  -g -O2 -MT simple_xy_wr.o -MD -MP -MF $depbase.Tpo -c -o simple_xy_wr.o simple_xy_wr.cpp
../libtool --tag=CXX   --mode=link c++  -g -O2 ../cxx/libnetcdf_c++.la  -o simple_xy_wr simple_xy_wr.o  -lnetcdf

