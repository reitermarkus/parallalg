extern crate libc;
use libc::{c_char, c_void};

extern crate rand;
use rand::distributions::{IndependentSample, Range};

use std::{env, ffi::CStr, collections::HashMap};

extern {
  fn gen_name() -> *const c_char;
  fn free_names() -> c_void;
}

unsafe fn generate_name() -> String {
  let c_buf = gen_name();
  let c_str = CStr::from_ptr(c_buf);
  c_str.to_str().unwrap().to_owned()
}

fn generate_names(count: usize) -> Vec<String> {
  let names = (0..count).map(|_| {
    unsafe { generate_name() }
  }).collect();

  unsafe { free_names(); }

  names
}

fn print_list(list: &HashMap<usize, &String>) {
  for (n, name) in list {
    println!("{: >#3} | {}", n, name);
  }
}

fn main() {
  let args: Vec<String> = env::args().collect();
  let name_count: usize = args.get(1).map_or(None, |x| x.parse().ok()).unwrap_or(1337);

  let names = generate_names(name_count);

  let age_range = Range::new(1, 101);

  let persons: HashMap<usize, &String> = names.iter().map(|name| {
    (age_range.ind_sample(&mut rand::thread_rng()), name)
  }).collect();

  print_list(&persons);
}
