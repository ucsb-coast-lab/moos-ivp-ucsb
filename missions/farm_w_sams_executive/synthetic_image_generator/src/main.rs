/// cmoran
/// UCSB Coastal Oceanography and Autonomous Systems Lab
/// 12 February 2019

/// This program generates an image from scratch to ideally but somewhat realistic simulate
/// acoustic data from a long-line kelp farm. The parameters of the resulting image can be
/// easily altered, including size, type, placement, and variation of the signal. The resulting
/// image will be produced as a .png file in the `figures` directory, along with a .csv file of
/// 0-255 values that will reproduce the array if imported. It's recommended to run this using
/// `cargo run --release`, especially if experimenting with changing the image paramters, as
/// execution time of release builds are significantly decreased over standard debug configs.

/// NOTE: Requires Rust 2018, due to use of features like the dbg!() macro
/// Most of these uses are not critical, and can be commented out if use of the previous version
/// is desired
extern crate csv;
extern crate image;
extern crate imageproc;
extern crate ndarray;
extern crate ndarray_csv;
extern crate rand;

use csv::WriterBuilder;
use image::GrayImage;
use ndarray::Array2;
use ndarray_csv::Array2Writer;
use rand::{thread_rng, Rng};
use std::error::Error;
use std::f32::*;
use std::fs::File;

fn main() -> Result<(), Box<Error>> {
    let ping_num = 800; // Corresponds to WIDTH of image, in pixels
    let bin_num = 500; // Corresponds to HEIGHT of image, in pixels

    let mut max: f32 = 0.0;
    let mut min: f32 = 0.0;
    let sigma_base: f32 = 5.0; // width of distribution curve
    let mu_base: f32 = 100.0; // center of distribution curve

    // Creates a 2D array with number of pings and total sonar bins
    let mut arr = Array2::zeros((ping_num, bin_num));
    let mut rng = thread_rng();
    let sigma_range: (f32, f32) = (0.0, 2.0); // sets random variation for sigma, per ping (0.0,2.0)
    let mu_range: (f32, f32) = (0.0, 25.0); // sets random variation for mu, per ping (0,0.50.0)

    for x in 0..ping_num {
        // randomly moves the center of the function
        let mu_var_rand: f32 = rng.gen_range(mu_range.0, mu_range.1);
        // varies center of function based on periodic wave to mimic long-lines
        let amp: f32 = 10.0;
        let amp_2: f32 = 4.0;
        let omega: f32 = 0.000;
        let omega_2: f32 = 0.00;
        // Supercomposition all center variations
        let mu_var: f32 =
            amp * (x as f32 * omega).sin() + amp_2 * (x as f32 * omega_2).sin() + mu_var_rand;

        let mu = mu_base + mu_var;

        for y in 0..bin_num {
            // randomly varies the width of the distribution
            let sigma_var: f32 = rng.gen_range(sigma_range.0, sigma_range.1); // rand::random();//rng.gen_range(0.0,0.01)
            let sigma = sigma_base + sigma_var;

            // When considering f to be function with a normal distribution
            let f = (1.0 / (2.0 * consts::PI * sigma.powf(2.0)))
                * (-(y as f32 - mu).powf(2.0) / (2.0 * sigma.powf(2.0))).exp();

            // Could also consider f as an exponential function
            // let power: f32 = 500.0;
            // let f: f32 = ((y as f32)/(bin_num as f32)).powf((mu+power+(y as f32)) / bin_num as f32);

            //dbg!(f);
            // For normalized data function
            arr[[x, y]] = f;
            // For linear data function or normalized data function
            if y > (mu - mu_var_rand + 10.0) as usize {
                arr[[x, y]] = 0.0;
            } else {
                arr[[x, y]] = f;
            }

            // finds the maximum and minimum generated values, to use for normalization
            if f.max(max) == f {
                max = f;
            }
            if f.min(min) == f {
                min = f;
            }
        }
    }

    // Normalizes all array values in range of [0,255] for Luma pixel
    for x in 0..ping_num {
        for y in 0..bin_num {
            arr[[x, y]] = (arr[[x, y]] - min) / (max - min) * 255f32;
        }
    }

    // Find the actual location of the center value for every ping value
    let mut vec_max_bin: Vec<usize> = Vec::new();
    let mut max_bin = 0;

    for x in 0..ping_num {
        for y in 0..bin_num {
            if arr[[x, y]] > arr[[x, max_bin]] {
                max_bin = y;
            }
        }
        vec_max_bin.push(max_bin);
        //assert_eq!(max_bin,vec_max_bin[x]); // For debugging
        //println!("max_bin for ping #{} is: {}",x,max_bin);
    }
    // Finds the cumulative average for where the mean peak value is
    let mean = vec_max_bin.iter().fold(0.0, |mut sum, &x| {
        sum += (x as f32);
        sum
    }) / (vec_max_bin.len() as f32);
    dbg!(mean);

    // Calculates a moving average of max values, and finds the max value in each ping
    let mut vec_mv_avg: Vec<f32> = Vec::new();
    for x in 0..ping_num {
        // finds the max bin in each ping
        for y in 0..bin_num {
            if arr[[x, y]] > arr[[x, max_bin]] {
                max_bin = y;
            }
        }
        // pushes that bin number to the maximum_bin vector
        vec_max_bin.push(max_bin);
        if x >= 5 {
            let mv_avg = ((vec_max_bin[x]
                + vec_max_bin[x - 1]
                + vec_max_bin[x - 2]
                + vec_max_bin[x - 4]
                + vec_max_bin[x - 5]) as f32)
                / 5.0;
            //let mv_avg = 10.0;
            vec_mv_avg.push(mv_avg);
            //println!("mv_avg #{}: {}",x ,mv_avg);
        }
    }

    // Creates an empty GrayImage and fills it with the image array, based on Luma<0-255> values
    let mut imgbuf = GrayImage::new(ping_num as u32, bin_num as u32);
    //let offset = 5;
    for m in 0..ping_num {
        for n in 0..bin_num {
            imgbuf[(m as u32, n as u32)] = image::Luma([(arr[[m, n]] as u8)]);
        }
        // Plots the filtered max index line at an offset below the generated image
        // if m < 395 {
        //     imgbuf[(m as u32,vec_mv_avg[m] as u32 + offset)] = image::Luma([255]);
        // }
    }
    // Saves image to file
    imgbuf.save("figures/synthetic_image.png").unwrap(); // If `figures/` doesn't exist, will panic!

    // Writes array to csv file
    let file = File::create("figures/array.csv").expect("Couldn't create figures/array.csv");
    let mut writer = WriterBuilder::new().has_headers(false).from_writer(file);
    writer.serialize_array2(&arr)?;

    Ok(())
}
