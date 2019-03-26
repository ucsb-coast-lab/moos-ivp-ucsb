#[macro_use] extern crate serde_derive;
// ^ #[macro_use] statement need to go at the base of the crate

// FFI dependencies
extern crate libc;
use libc::{int32_t, uint32_t};
use std::ffi::CStr;
use std::os::raw::c_char;

// Other dependencies
use serde::ser::Serialize;
use std::fs::File;
use std::io::Read;
use toml::value::Array;
use toml::{to_string_pretty, Value as Toml};

#[repr(C)]
#[derive(Deserialize)]
pub struct Config {
    project_name: String,
    auv: String,
    start_point: GeoCoor,
    point: Vec<Point>,
}

#[repr(C)]
#[derive(Deserialize)]
pub struct GeoCoor {
    lat: f32,
    lon: f32
}

#[repr(C)]
#[derive(Deserialize)]
pub struct Point {
    x: i32,
    y: i32,
}

#[no_mangle]
pub extern "C" fn get_toml_basics(filename_arg: *const c_char) {

    assert!(!filename_arg.is_null());
    let filename_c_str = unsafe { CStr::from_ptr(filename_arg) };
    let filename = filename_c_str.to_str().expect("Not a valid UTF-8 string");

    // Opens a file with the specified name
    let mut file = File::open(filename).expect("Specified .toml file does not exist");
    let mut content = String::new();
    file.read_to_string(&mut content).unwrap();
    let config: Config = toml::from_str(&content).unwrap();

    println!("project_name: {}",config.project_name);
    println!("auv: {}",config.auv);
    println!("start point in (lat,lon):\n\t({}, {})",config.start_point.lat, config.start_point.lon);

    println!("The points in the farm are: ");
    for i in 0..config.point.len() {
        println!("\t({}, {})", config.point[i].x, config.point[i].y);
    }
}

#[no_mangle]
pub extern "C" fn get_number_of_points(filename_arg: *const c_char ) -> uint32_t {

    assert!(!filename_arg.is_null());
    let filename_c_str = unsafe { CStr::from_ptr(filename_arg) };
    let filename = filename_c_str.to_str().expect("Not a valid UTF-8 string");

    // Opens a file with the specified name
    let mut file = File::open(filename).expect("Specified .toml file does not exist");
    let mut content = String::new();
    file.read_to_string(&mut content).unwrap();
    let config: Config = toml::from_str(&content).unwrap();

    let max_points = 64;
    if config.point.len() > max_points || (config.point.len() % 2) != 0  {
        panic!("The specified configuration file in invalid; the # of points must be < {} and even-numbered",max_points);
    }

    (config.point.len() as u32)
}

#[no_mangle]
pub extern "C" fn return_point_info(filename_arg: *const c_char, point_num: u32) -> Point {

    assert!(!filename_arg.is_null());
    let filename_c_str = unsafe { CStr::from_ptr(filename_arg) };
    let filename = filename_c_str.to_str().expect("Not a valid UTF-8 string");

    // Opens a file with the specified name
    let mut file = File::open(filename).expect("Specified .toml file does not exist");
    let mut content = String::new();
    file.read_to_string(&mut content).unwrap();
    let config: Config = toml::from_str(&content).unwrap();

    let return_point: Point = Point { x: config.point[point_num as usize].x as i32, y: config.point[point_num as usize].y as i32};
    return_point
}
