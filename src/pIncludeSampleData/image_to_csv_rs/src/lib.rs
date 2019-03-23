// General Rust FFI dependencies
extern crate libc;

use libc::int32_t;
use std::ffi::CStr;
use std::os::raw::c_char;

// process_image() dependencies
extern crate image;
extern crate ndarray;
extern crate ndarray_csv;
extern crate csv;

use image::{GenericImageView, ImageBuffer,Pixel};
use std::env;
use std::path::Path;
use ndarray::{Array};
use std::error::Error;
use std::fs::File;
use csv::{WriterBuilder};
use ndarray_csv::{Array2Writer};

#[no_mangle]
pub extern "C" fn hello_from_rust() {
    println!("hello, world! -- from a Rust function");
}

#[no_mangle]
pub extern "C" fn process_image(image_path_arg: *const c_char, csv_export_path_arg: *const c_char) -> i32 {
    println!("Would be processing the image here, but will output the entered string:");
    assert!(!image_path_arg.is_null());
    let image_path_c_str = unsafe { CStr::from_ptr(image_path_arg) };
    let image_path = image_path_c_str.to_str().expect("Not a valid UTF-8 string");

    assert!(!csv_export_path_arg.is_null());
    let csv_export_path_c_str = unsafe { CStr::from_ptr(csv_export_path_arg) };
    let csv_export_path = csv_export_path_c_str.to_str().expect("Not a valid UTF-8 string");

    println!("{},{}",image_path, csv_export_path);

    println!("About to call the 'image_processing_rs' fucntion:");
    let version = match image_processing_rs(image_path,csv_export_path) {
        Ok(n) => return 0,
        Err(_) => return -1
    };
}

fn image_processing_rs(image_path: &str, csv_export_path: &str) -> std::result::Result<(), Box<Error>> {

    dbg!("About to open and rotate the image:");
    let naive = image::open(image_path).expect(&format!("Could not load image at {:?}", image_path));
    naive.rotate90();
    naive.save(image_path).unwrap();

    let naive = image::open(image_path).expect(&format!("Could not load image at {:?}", image_path));
    // Copies dimensions from image
    let (height, width) = naive.dimensions();

    let (dimx, dimy) = (width as usize, height as usize);

    //let mut arr = Array2::zeros_like((dimx,dimy));
    let mut arr = Array::from_elem((dimy, dimx), 0u8);
    let mut my_image = ImageBuffer::new(width as u32, height as u32);
    // Creates 2D array of same dimensions as image
    // let mut arr = Array2::zeros((dimx as usize,dimy as usize));
    arr[[0 as usize,0]] = 4;
    for y in 0..my_image.width() {
        for x in 0..my_image.height() {
            let pulled_pixel = naive.get_pixel(x,y).to_luma();
            my_image.put_pixel(y,x, pulled_pixel);
            let pp_value = pulled_pixel[0] as u8;
            //println!("[{},{}] -> {}",x,y,pp_value);
            arr[[x as usize,y as usize]] = pp_value; //pp_value as u8;
        }
    }

    // Write the array into the file.
    //let file = File::create("test.csv")?;
    let file = File::create(csv_export_path)?;
    let mut writer = WriterBuilder::new().has_headers(false).from_writer(file);
    writer.serialize_array2(&arr)?;

    Ok(())

}
