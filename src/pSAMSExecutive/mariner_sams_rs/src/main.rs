mod lib;
mod tests;

use lib::mytest_msrs;

pub fn main() {
    println!("A binary capable of running functions from the library mariner_sams_rs");
    mytest_msrs();
}
