extern crate libc;
use libc::c_char;

extern crate rand;
use rand::distributions::{IndependentSample, Range};

use std::{env, ffi::CStr, collections::HashMap};

extern {
  fn gen_name() -> *const c_char;
}

fn generate_name() -> String {
  let c_buf = unsafe { gen_name() };
  let c_str = unsafe { CStr::from_ptr(c_buf) };
  c_str.to_str().unwrap().to_owned()
}

fn print_list(list: &HashMap<usize, String>) {
  for (n, name) in list {
      println!("{: >#3} | {}", n, name);
  }
}

fn main() {
  let args: Vec<String> = env::args().collect();
  let name_count: usize = args.get(1).map_or(None, |x| x.parse().ok()).unwrap_or(1337);

  let names : HashMap<usize, String> = (0..name_count).map(|_| {
    let between = Range::new(1, 101);
    let rng = &mut rand::thread_rng();
    (between.ind_sample(rng), generate_name())
  }).collect();

  print_list(&names);
}
