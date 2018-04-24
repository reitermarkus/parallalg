use rand::{SeedableRng, StdRng};
use rand::distributions::{Distribution, Range};

pub fn random_bytes(n: usize, seed: <StdRng as SeedableRng>::Seed) -> Vec<u64> {
  let mut rng: StdRng = SeedableRng::from_seed(seed);

  let range = Range::new(0, 2);

  (0..n).map(|_| {
    range.sample(&mut rng)
  }).collect()
}
