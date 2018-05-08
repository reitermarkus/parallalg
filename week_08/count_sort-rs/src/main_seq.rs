extern crate libc;

extern crate rand;
use rand::distributions::{IndependentSample, Range};

use std::{env};

mod name_generator;
use name_generator::NameGenerator;

fn print_list(list: &Vec<(usize, String)>) {
  for &(ref n, ref name) in list {
    println!("{: >#3} | {}", n, name);
  }
}

fn main() {
  let args: Vec<String> = env::args().collect();
  let name_count: usize = args.get(1).map_or(None, |x| x.parse().ok()).unwrap_or(1337);

  let name_generator = NameGenerator::new();
  let age_range = Range::new(1, 101);

  let persons: Vec<(usize, String)> = (0..name_count).map(|_| {
    (age_range.ind_sample(&mut rand::thread_rng()), name_generator.generate())
  }).collect();

  print_list(&persons);
}
