use std::time::{Instant, Duration};

pub fn benchmark<F, R>(closure: F) -> R where F: Fn() -> R {

  let total_bench = Instant::now();

  let result = closure();

  let into_ms = |x: Duration| (x.as_secs() * 1_000) + (x.subsec_nanos() / 1_000_000) as u64;

  let total_bench_elapsed = total_bench.elapsed();
  println!("┌────────────────────┬───────────┐");
  println!("│ Total              │ {:#6 } ms │", into_ms(total_bench_elapsed));
  println!("└────────────────────┴───────────┘");

  result
}
