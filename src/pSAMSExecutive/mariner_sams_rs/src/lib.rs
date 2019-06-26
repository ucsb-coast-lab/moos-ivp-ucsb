// FFI dependencies
//extern crate libc;
use libc::{int32_t, uint32_t};
use std::ffi::CStr;
use std::os::raw::c_char;

pub fn mytest_msrs() -> int32_t {
    println!("This is a test from mariner_sams_rs library!");
    let my_int: int32_t = 4;
    my_int
}

fn parse_filename_from_c(filename_arg: *const c_char) -> String {
    assert!(!filename_arg.is_null());
    let filename_c_str = unsafe { CStr::from_ptr(filename_arg) };
    let filename = filename_c_str.to_str().expect("Not a valid UTF-8 string");
    filename.to_string()
}
