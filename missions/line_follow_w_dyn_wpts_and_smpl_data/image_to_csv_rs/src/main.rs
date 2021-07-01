use image::{GenericImageView, ImageBuffer,Pixel};
use std::env;
use std::path::Path;
use std::error::Error;
use std::fs::File;
use csv::{WriterBuilder};
use ndarray_csv::{Array2Writer};
use ndarray::prelude::*;

fn main() -> Result<(), Box<dyn Error>> {

    // Specifies an image to be opened by the image processing program
    let mut arg_vec: Vec<String> = Vec::new();
    for argument in env::args() {
        arg_vec.push(argument);
    }
    let open_path = Path::new(&arg_vec[1]);
    let naive = image::open(open_path).expect(&format!("Could not load image at {:?}", open_path));
    naive.rotate90();
    naive.save(open_path).unwrap();
    //let naive = open(open_path).expect(&format!("Could not load image at {:?}", open_path));
    let naive = image::open(open_path).expect(&format!("Could not load image at {:?}", open_path));
    // Copies dimensions from image
    let (height, width) = naive.dimensions();

    let (dimx, dimy) = (width as usize, height as usize);

    //let mut arr = Array2::zeros_like((dimx,dimy));
    let mut arr: Array2<u8> = Array2::zeros((dimy, dimx));
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
    let file = File::create(&arg_vec[2])?;
    let mut writer = WriterBuilder::new().has_headers(false).from_writer(file);
    writer.serialize_array2(&arr)?;
    Ok(())
}
