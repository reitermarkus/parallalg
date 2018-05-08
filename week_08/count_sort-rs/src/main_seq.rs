#![feature(test)]
extern crate test;

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

fn count_sort<U, F>(input: Vec<U>, sort_by: F) -> Vec<U>
  where F: Fn(&U) -> usize {

  let max = input.iter().fold(0, |acc_max, b| acc_max.max(sort_by(b)));

  let mut count_arr : Vec<usize> = (0..(max + 1)).map(|i| {
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

  let name_generator = NameGenerator::new();
  let age_range = Range::new(1, 101);

  let persons: Vec<Person> = (0..name_count).map(|_| {
    Person {
      age:  age_range.ind_sample(&mut rand::thread_rng()),
      name: name_generator.generate(),
    }
  }).collect();

  print_list(&persons);

  let sorted_persons = count_sort(persons, |p| p.age);

  print_list(&sorted_persons);
}


#[cfg(test)]
mod tests {
  use super::*;
  use test::Bencher;

  extern crate rand;
  use rand::distributions::{IndependentSample, Range};

  #[test]
  fn test_count_sort() {
    assert_eq!(count_sort(vec![6, 5, 8, 2, 1], |&e| e), vec![1, 2, 5, 6, 8]);
  }

  fn numbers() -> Vec<usize> {
    (0..1000).map(|_| {
      Range::new(1, 10).ind_sample(&mut rand::thread_rng())
    }).collect()
  }

  #[bench]
  fn bench_count_sort(b: &mut Bencher) {
    let numbers = numbers();

    b.iter(|| {
      let cloned_numbers = numbers.clone();
      count_sort(cloned_numbers, |&e| e)
    });
  }

  #[bench]
  fn bench_sort(b: &mut Bencher) {
    let numbers = numbers();

    b.iter(|| {
      let mut cloned_numbers = numbers.clone();
      cloned_numbers.sort();
      cloned_numbers
    });
  }
}
