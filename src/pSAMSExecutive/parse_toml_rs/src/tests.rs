// use the `$ cargo test -- --nocapture` to run the test suite

#[cfg(test)]
mod tests {

    use crate::lib::*;
    use std::fs::File;
    use std::io::{Read};

    // This is the name of the .toml this test suite will evaluate
    const FILENAME: &str = "sams_config_test.toml";

    #[test]
    #[ignore]
    fn first_test() {
        // println!("From the first test in the tests module!");
        let val = mytest();
        println!("Hello!");
        assert_eq!(val,4);
    }

    #[test]
    fn check_get_toml_basics() {
        get_toml_basics_rs(FILENAME);
    }

    #[test]
    fn check_number_of_tasks() {
        let task_num = get_number_of_tasks_rs(FILENAME);
        println!("The number of tasks is: {}",task_num);
        assert_eq!(task_num,8);
    }


    // TO_DO: Right now, the info being returned here is in the form of a Point struct. This means
    // that we can't do matching based on label, which we definitely want to do. That does mean
    // that we'll end up also
    #[test]
    fn check_return_waypoint_info() {
        let mut file = File::open(FILENAME).expect("Specified .toml file does not exist");
        let mut content = String::new();
        file.read_to_string(&mut content).unwrap();
        let config: SAMSConfig = toml::from_str(&content).unwrap();

        let number_of_points = config.waypoints.len();
        println!("Number of points = {}",number_of_points);

        let v: Vec<&str> = vec!["alpha","bravo","charlie","D","E","F","G","H","I","J","K","L"];

        for point_num in 0..number_of_points {
            let waypoint: Waypoint = return_waypoint_info_rs(FILENAME,v[point_num]);
            assert_eq!(config.waypoints[point_num].label,v[point_num]);
            assert_eq!(waypoint.label,v[point_num]);
        }

        /*
        // TO_DO: Use this design pattern for doing TASK MATCHING, not GeoCoor validation
        // for the 'points' field
        for point_num in 0..number_of_points {
            println!("Checking point #{}",point_num as u32);
            let point: Point = return_point_info_rs(FILENAME,point_num as u32);
            let val = match point_num {
                0 => (point.x == 50 ) && (point.y == -50),
                1 => (point.x == 50 ) && (point.y == -150),
                2 => (point.x == 75 ) && (point.y == -175),
                3 => (point.x == 75 ) && (point.y == -25),
                4 => (point.x == 100 ) && (point.y == -25),
                5 => (point.x == 125 ) && (point.y == -175),
                6 => (point.x == 150 ) && (point.y == -175),
                7 => (point.x == 175 ) && (point.y == -50),
                8 => (point.x == 150 ) && (point.y == -10),
                9 => (point.x == 100 ) && (point.y == 0),
                10 => (point.x == 50 ) && (point.y == 0),
                11 => (point.x == 0 ) && (point.y == 0),
                _ => (0 == 1),// should always fail,
            };
            println!("Point #{}'s (lat,lon) = ({},{})",point_num,point.x,point.y);
            assert_eq!(val,true)
        }*/
    }


    #[test]
    fn test_return_start_point() {
        let mut file = File::open(FILENAME).expect("Specified .toml file does not exist");
        let mut content = String::new();
        file.read_to_string(&mut content).unwrap();
        let config: SAMSConfig = toml::from_str(&content).unwrap();

        let return_point: GeoCoor = return_start_point_rs(FILENAME);
        assert_eq!(return_point.lat,config.start_point.lat);
        assert_eq!(return_point.lon,config.start_point.lon);
    }

    #[test]
    fn test_return_task_position() {

        let mut file = File::open(FILENAME).expect("Specified .toml file does not exist");
        for j in 0..8 {
            let task_position:GeoCoor = return_task_position_rs(FILENAME,j);
        }

    }

}
