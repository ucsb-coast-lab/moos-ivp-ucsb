#include <cstdarg>
#include <cstdint>
#include <cstdlib>

struct GeoCoor {
  int32_t lat;
  int32_t lon;
};

extern "C" {

uint32_t get_number_of_tasks(const char *filename_arg);

void get_toml_basics(const char *filename_arg);

GeoCoor return_start_point(const char *filename_arg);

GeoCoor return_task_position(const char *filename_arg, uint32_t task_num);

GeoCoor return_waypoint_info(const char *filename_arg, const char *waypoint_label);

} // extern "C"
