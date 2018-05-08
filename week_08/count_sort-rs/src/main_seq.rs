extern crate libc;

extern crate rand;
use rand::distributions::{IndependentSample, Range};

use std::{env};

mod name_generator;
use name_generator::NameGenerator;

mod person;
use person::Person;

fn print_list(list: &Vec<Person>) {
  for person in list {
    println!("{: >#3} | {}", person.age, person.name);
  }
}

fn count_sort(input: Vec<usize>) -> Vec<usize> {
  let size = input.len();
  let max = input.iter().fold(0, |acc_max, &b| acc_max.max(b));

  let mut count_arr : Vec<usize> = (0..(max + 1)).map(|i|
    (0..size).fold(0, |acc, j| if input[j] < i {acc + 1} else {acc})).collect();

  let mut result = vec![0; size];

  input.iter().for_each(|&element| {
    if let Some(index) = count_arr.get_mut(element) {
      result[*index] = element;
      *index += 1;
    }
  });

  result
}

fn main() {
  let args: Vec<String> = env::args().collect();
  let name_count: usize = args.get(1).map_or(None, |x| x.parse().ok()).unwrap_or(1337);

  let name_generator = NameGenerator::new();
  let age_range = Range::new(1, 101);

  let persons: Vec<Person> = (0..name_count).map(|_| {
    Person {
      age:  age_range.ind_sample(&mut rand::thread_rng()),
      name: name_generator.generate(),
    }
  }).collect();

  print_list(&persons);
}
