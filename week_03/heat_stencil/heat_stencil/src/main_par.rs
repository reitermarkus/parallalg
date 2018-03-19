use std::time::{Instant, Duration};
use std::sync::atomic::Ordering::{Acquire, Release};

extern crate crossbeam;
extern crate rayon;

use crossbeam::epoch::{self, Atomic, Owned};

use rayon::prelude::*;

mod print_temperature;
use print_temperature::print_temperature;

fn main() {
  let total_bench = Instant::now();
  let n = 500;

  let mut matrix_a: Vec<f64> = vec![273.0; n * n];
  let mut matrix_b: Vec<f64> = vec![0.0; n * n];

  // Add heat source in corner.
  let source_x = n / 4;
  let source_y = n / 4;
  matrix_a[source_x * n + source_y] += 60.0;

  print_temperature(&matrix_a, n, n);

  let time_steps = n * 100;

  for t in 0..time_steps {
    for i in 0..n {
      for j in 0..n {
        // Center stays constant (the heat is still on).
        if i == source_x && j == source_y {
          matrix_b[i * n + j] = matrix_a[i * n + j];
          continue;
        }

        // Get current temperature at (i,j).
        let tc = matrix_a[i * n + j];

        // Get temperatures left/right and up/down.
        let tl = if j !=  0  { matrix_a[i * n + (j - 1)] } else { tc };
        let tr = if j != n - 1 { matrix_a[i * n + (j + 1)] } else { tc };
        let tu = if i !=  0  { matrix_a[(i - 1) * n + j] } else { tc };
        let td = if i != n - 1 { matrix_a[(i + 1) * n + j] } else { tc };

        // Update temperature at current point
        matrix_b[i * n + j] = tc + 0.2 * (tl + tr + tu + td + (-4.0 * tc));
      }
    }

    // Swap matrices.
    let mut matrix_h = matrix_a;
    matrix_a = matrix_b;
    matrix_b = matrix_h;

    if t % 1000 == 0 {
      println!("Step t = {}:", t);
      print_temperature(&matrix_a, n, n);
    }
  }

  let into_ms = |x: Duration| (x.as_secs() * 1_000) + (x.subsec_nanos() / 1_000_000) as u64;

  let total_bench_elapsed = total_bench.elapsed();
  println!("┌────────────────────┬───────────┐");
  println!("┃ Total              ┃ {:#6 } ms ┃", into_ms(total_bench_elapsed));
  println!("┗━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━┛");
}
