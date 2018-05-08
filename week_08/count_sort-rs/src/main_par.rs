use std::env;

extern crate libc;
extern crate rand;
extern crate rayon;

use rayon::prelude::*;

mod person;
use person::{print_persons, generate_persons};

mod name_generator;

fn count_sort<U, F>(input: Vec<U>, sort_by: F) -> Vec<U>
  where F: Fn(&U) -> usize + Send + Sync, U: Send + Sync {

  let max = input.par_iter().fold(|| 0, |acc_max, b| acc_max.max(sort_by(b))).sum::<usize>();

  let mut count_arr : Vec<usize> = (0..(max + 1)).into_par_iter().map(|i| {
    input.iter().fold(0, |acc, elem| if sort_by(elem) < i { acc + 1 } else { acc })
  }).collect();

  let mut result: Vec<U> = Vec::with_capacity(input.len());
  unsafe { result.set_len(input.len()) }

  input.into_iter().for_each(|elem| {
    if let Some(index) = count_arr.get_mut(sort_by(&elem)) {
      result[*index] = elem;
      *index += 1;
    }
  });

  result
}

fn main() {
  let args: Vec<String> = env::args().collect();
  let name_count: usize = args.get(1).map_or(None, |x| x.parse().ok()).unwrap_or(1337);
  let persons = generate_persons(name_count);

  print_persons(&persons);

  let sorted_persons = count_sort(persons, |p| p.age);

  print_persons(&sorted_persons);
}
