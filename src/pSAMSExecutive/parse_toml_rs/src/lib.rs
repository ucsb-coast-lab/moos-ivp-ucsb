// FFI dependencies
//extern crate libc;
use libc::{int32_t, uint32_t};
use std::ffi::CStr;
use std::os::raw::c_char;

// Other dependencies
use serde::{Deserialize, Serialize};
use std::fs::File;
use std::io::Read;

#[repr(C)]
#[derive(Serialize, Deserialize, Debug)]
pub struct SAMSConfig {
    pub project_name: String,
    pub auv: String,
    pub waypoints: Vec<Waypoint>,
    pub tasks: Vec<String>,
    pub start_point: Waypoint,
}

#[repr(C)]
#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct Waypoint {
    pub label: String,
    pub lat: i32,
    pub lon: i32,
}

#[repr(C)]
#[derive(Serialize, Deserialize, Debug)]
pub struct GeoCoor {
    pub lat: i32,
    pub lon: i32,
}

// This is used as a test function for the testing framework
pub fn mytest() -> int32_t {
    println!("This is a test");
    let my_int: int32_t = 4;
    my_int
}

fn parse_filename_from_c(filename_arg: *const c_char) -> String {
    assert!(!filename_arg.is_null());
    let filename_c_str = unsafe { CStr::from_ptr(filename_arg) };
    let filename = filename_c_str.to_str().expect("Not a valid UTF-8 string");
    filename.to_string()
}

fn toml_to_config(filename: &str) -> SAMSConfig {
    let mut file = File::open(filename).expect("Specified .toml file does not exist"); //?
    let mut content = String::new();
    file.read_to_string(&mut content).unwrap(); //?
    let config: SAMSConfig =
        toml::from_str(&content).expect("converting toml text to Config struct has failed"); // .unwrap();
    config
}

// Note: The design pattern for these functions is currently to have an extern "C" wrapper around a Rust function,
// which wil handle all the FFI required, while the Rust function will handle all of the business logic. In
// addition, this allows us to write test modules for the business logic more easily, leaving the FFI testing to
// happen separately, probably in a separate C++ framework
#[no_mangle]
pub extern "C" fn get_toml_basics(filename_arg: *const c_char) {
    let filename = parse_filename_from_c(filename_arg);
    get_toml_basics_rs(&filename);
}

pub fn get_toml_basics_rs(filename: &str) {
    let config: SAMSConfig = toml_to_config(filename);
    println!("project_name: {}", config.project_name);
    println!("auv: {}", config.auv);
    println!(
        "start GeoCoor in (lat,lon):\n               ({}, {})",
        config.start_point.lat, config.start_point.lon
    );
    // let together = format!("{}\n{}\n{}\n{}", config.project_name, config.auv,config.start_point.lat, config.start_point.lon );

    println!("The waypoints in the farm are: ");
    for i in 0..config.waypoints.len() {
        println!(
            "\t(label =  {}, lat = {}, lon = {})",
            config.waypoints[i].label, config.waypoints[i].lat, config.waypoints[i].lon
        );
    }

    println!("which are organized into tasks in the following order:");
    for i in 0..config.tasks.len() / 2 {
        println!("\t({}, {})", config.tasks[i], config.tasks[i + 1]);
    }
}

#[no_mangle]
pub extern "C" fn get_number_of_tasks(filename_arg: *const c_char) -> uint32_t {
    let filename = parse_filename_from_c(filename_arg);
    let task_num = get_number_of_tasks_rs(&filename);

    task_num
}

pub fn get_number_of_tasks_rs(filename: &str) -> uint32_t {
    let config: SAMSConfig = toml_to_config(filename);
    let max_tasks = 64; // Two GeoCoors per Task, with a maximum of 64 GeoCoors allocated in memory
    if config.tasks.len() > max_tasks {
        panic!("The specified configuration file in invalid; the # of tasks must be < {} and even-numbered",max_tasks);
    }

    (config.tasks.len() as u32)
}

#[no_mangle]
pub extern "C" fn return_waypoint_info(
    filename_arg: *const c_char,
    waypoint_label: *const c_char,
) -> GeoCoor {
    let filename = parse_filename_from_c(filename_arg);

    assert!(!waypoint_label.is_null());
    let waypoint_label_c_str = unsafe { CStr::from_ptr(filename_arg) };
    let waypoint_label = waypoint_label_c_str
        .to_str()
        .expect("Not a valid UTF-8 string");

    let point_as_coor: Waypoint = return_waypoint_info_rs(&filename, waypoint_label);
    let return_point: GeoCoor = GeoCoor {
        lat: point_as_coor.lat as i32,
        lon: point_as_coor.lon as i32,
    };
    return_point
}

pub fn return_waypoint_info_rs(filename: &str, waypoint_label: &str) -> Waypoint {
    // Opens a file with the specified name
    let config: SAMSConfig = toml_to_config(filename);
    let mut point_as_coor: Waypoint = Waypoint {
        label: "Origin".to_string(),
        lat: 0,
        lon: 0,
    };
    for j in 0..config.waypoints.len() {
        if config.waypoints[j].label == waypoint_label {
            point_as_coor = config.waypoints[j].clone();
        }
    }
    point_as_coor
}

// returns the GPS coordinates of the start GeoCoor as a signed 32-bit integer, which can then be converted to floating poin
// in the C++ function

#[no_mangle]
pub extern "C" fn return_start_point(filename_arg: *const c_char) -> GeoCoor {
    let filename = parse_filename_from_c(filename_arg);
    let return_point: GeoCoor = return_start_point_rs(&filename);
    return_point
}

pub fn return_start_point_rs(filename: &str) -> GeoCoor {
    // Opens a file with the specified name
    let config: SAMSConfig = toml_to_config(filename);
    let return_point: GeoCoor = GeoCoor {
        lat: config.start_point.lat as i32,
        lon: config.start_point.lon as i32,
    };
    return_point
}

#[no_mangle]
pub extern "C" fn return_task_position(filename_arg: *const c_char, task_num: u32) -> GeoCoor {
    let filename = parse_filename_from_c(filename_arg);
    let task_position = return_task_position_rs(&filename, task_num);
    task_position
}

// The error handling here (in terms of dealing with what happens if a Task label doesn't exist for a point)
// is not great.
// TO_DO: Implement a function that will reject a .toml file as invalid if such an inconsistency exists
pub fn return_task_position_rs(filename: &str, task_num: u32) -> GeoCoor {
    let config: SAMSConfig = toml_to_config(filename);
    let mut task_position: GeoCoor = return_start_point_rs(filename);
    for i in 0..config.waypoints.len() {
        if config.waypoints[i].label == config.tasks[task_num as usize] {
            println!(
                "Task {}'s location in the points is point#{}",
                config.tasks[task_num as usize], i
            );
            task_position.lat = config.waypoints[i].lat;
            task_position.lon = config.waypoints[i].lon;
        }
    }

    task_position
}
