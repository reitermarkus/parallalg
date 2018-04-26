extern crate rand;

mod random_bytes;
use random_bytes::random_bytes;

#[macro_use]
mod benchmark;

fn main() {
  let n = 1_000_000_000;
  let seed = [0; 32];

  let bytes = random_bytes(n, seed);

  let ones: u64 = benchmark!("Total", {
    bytes.iter().sum()
  });
  println!("{}", ones);
}
