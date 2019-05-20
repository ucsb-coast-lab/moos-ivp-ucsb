mod lib;
mod tests;

use lib::mytest;

pub fn main() {
    println!("A binary capable of running functions from lib.rs");
    mytest();
}
