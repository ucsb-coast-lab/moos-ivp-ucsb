NAME: cmoran

UCSB Coastal Oceanography and Autonomous Systems Lab

DATE: 7 February 2019

MISSION: line_follow_w_dyn_wpts_and_smpl_data

---

This is a mission simulation for using feature-based recognition on acoustic
data to successfully navigate in a long-line kelp farm. In addition to the
standard MOOS-IvP core distribution, it also requires the pLineFollow, pLineTurn,
pSimDistanceGenerator, and pIncludeSampleData processes.

An image (.jpg or .png is reconmmended) of acoustic sample data is also
required, along with a basic Rust toolchain. The folder `image_to_csv_rs` is
used to produce a Rust binary that, when called by pIncludeSampleData, converts
the image file into a rectangular csv array in a .csv file that is then read
by the MOOS process. If the code is modified, the binary will need to be moved
from the `image_to_csv_rs/target` directory into the main mission directory,
where the image file should be located as well.

Details regarding the Rust toolchain can be found at `https://www.rust-lang.org/`
