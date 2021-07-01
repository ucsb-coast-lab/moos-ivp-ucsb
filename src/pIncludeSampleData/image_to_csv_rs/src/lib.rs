// General Rust FFI dependencies
use std::ffi::CStr;
use std::os::raw::c_char;

use image::{GenericImageView, ImageBuffer,Pixel};
use ndarray::prelude::*;
use std::error::Error;
use std::fs::File;
use csv::{WriterBuilder};
use ndarray_csv::{Array2Writer};

// Test function
#[no_mangle]
pub extern "C" fn hello_from_rust() {
    println!("hello, world! -- from a Rust function");
}

// This is the function called by pIncludeSampleData, which uses the local Rust image_processing function
// It really is just a wrapper around image_processing_rs() that is uses for convenient error handling
#[no_mangle]
pub extern "C" fn process_image(image_path_arg: *const c_char, csv_export_path_arg: *const c_char) -> i32 {
    // Checks for validity of the specified paths pulled in from the IncludeSampleData.cpp  file
    println!("Would be processing the image here, but will output the entered string:");
    assert!(!image_path_arg.is_null());
    let image_path_c_str = unsafe { CStr::from_ptr(image_path_arg) };
    let image_path = image_path_c_str.to_str().expect("Not a valid UTF-8 string");

    assert!(!csv_export_path_arg.is_null());
    let csv_export_path_c_str = unsafe { CStr::from_ptr(csv_export_path_arg) };
    let csv_export_path = csv_export_path_c_str.to_str().expect("Not a valid UTF-8 string");

    //println!("{},{}",image_path, csv_export_path);

    println!("About to call the 'image_processing_rs' fucntion:");
    // Runs the image_processing_rs() function, and observes the return value
    // If the value is okay, returns a 0; otherwise, returns a -1 as the error code across the FFI boundary into the C++ file
    let version = match image_processing_rs(image_path,csv_export_path) {
        Ok(_) => return 0,
        Err(_) => return -1
    };
}

fn image_processing_rs(image_path: &str, csv_export_path: &str) -> std::result::Result<(), Box<dyn Error>> {

    //dbg!("About to open and rotate the image:");
    // Opens image, rotates it 90 degrees, and re-saves.
    let naive = image::open(image_path).expect(&format!("Could not load image at {:?}", image_path));
    naive.rotate90();
    naive.save(image_path).unwrap();

    // Reloads rotated image
    let naive = image::open(image_path).expect(&format!("Could not load image at {:?}", image_path));
    // Copies dimensions from image
    let (height, width) = naive.dimensions();

    let (dimx, dimy) = (width as usize, height as usize);

    // Creates an array of zeros the same size as the image file
    let mut arr: Array2<u8> = Array2::zeros((dimy, dimx));
    let mut my_image = ImageBuffer::new(width as u32, height as u32);
    arr[[0 as usize,0]] = 4;
    // Fills the image array with 0-255 values taken from the imported image
    for y in 0..my_image.width() {
        for x in 0..my_image.height() {
            let pulled_pixel = naive.get_pixel(x,y).to_luma();
            my_image.put_pixel(y,x, pulled_pixel);
            let pp_value = pulled_pixel[0] as u8;
            //println!("[{},{}] -> {}",x,y,pp_value);
            arr[[x as usize,y as usize]] = pp_value; //pp_value as u8;
        }
    }

    // Write the image array to the file.
    let file = File::create(csv_export_path)?;
    let mut writer = WriterBuilder::new().has_headers(false).from_writer(file);
    writer.serialize_array2(&arr)?;

    Ok(())

}
