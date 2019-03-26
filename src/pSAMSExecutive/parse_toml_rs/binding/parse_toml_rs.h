#include <cstdarg>
#include <cstdint>
#include <cstdlib>

struct Point {
  int32_t x;
  int32_t y;
};

extern "C" {

uint32_t get_number_of_points(const char *filename_arg);

void get_toml_basics(const char *filename_arg);

Point return_point_info(const char *filename_arg, uint32_t point_num);

} // extern "C"
