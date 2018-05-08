use ::name_generator::NameGenerator;
use ::rand::{thread_rng, distributions::{IndependentSample, Range}};

pub struct Person {
  pub age: usize,
  pub name: String,
}

pub fn print_persons(list: &Vec<Person>) {
  for person in list {
    println!("{: >#3} | {}", person.age, person.name);
  }
}

pub fn generate_persons(name_count: usize) -> Vec<Person>{
  let name_generator = NameGenerator::new();
  let age_range = Range::new(1, 101);

  (0..name_count).map(|_| {
    Person {
      age:  age_range.ind_sample(&mut thread_rng()),
      name: name_generator.generate(),
    }
  }).collect()
}
