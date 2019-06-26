// use the `$ cargo test -- --nocapture` to run the test suite

#[cfg(test)]
mod tests {

    use crate::lib::*;
    use std::fs::File;
    use std::io::Read;

    #[test]
    //#[ignore]
    fn first_test() {
        // println!("From the first test in the tests module!");
        let val = mytest_msrs();
        println!("Hello!");
        assert_eq!(val, 4);
    }
}
