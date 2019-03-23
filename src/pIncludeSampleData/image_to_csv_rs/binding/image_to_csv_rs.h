#include <cstdarg>
#include <cstdint>
#include <cstdlib>

extern "C" {

void hello_from_rust();

int32_t process_image(const char *image_path_arg, const char *csv_export_path_arg);

} // extern "C"
