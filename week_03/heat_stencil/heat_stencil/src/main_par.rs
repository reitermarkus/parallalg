use std::sync::Arc;

extern crate rayon;

use rayon::prelude::*;

mod print_temperature;
use print_temperature::print_temperature;

#[macro_use]
mod benchmark;

fn main() {
  benchmark! {
    let n = 500;

    let mut matrix_a = vec![273.0; n * n];
    let matrix_b = vec![0.0; n * n];

    // Add heat source in corner.
    let source_x = n / 4;
    let source_y = n / 4;
    matrix_a[source_x * n + source_y] += 60.0;

    let matrix_a_arc = Arc::new(matrix_a);
    let matrix_b_arc = Arc::new(matrix_b);

    print_temperature(&matrix_a_arc.clone(), n, n);

    let time_steps = n * 100;

    for t in 0..time_steps {
      let c_matrix_a = if t % 2 == 0 { matrix_a_arc.clone() } else { matrix_b_arc.clone() };
      let c_matrix_b = if t % 2 == 1 { matrix_a_arc.clone() } else { matrix_b_arc.clone() };

      (0..n).into_par_iter().for_each(move |i| {
        for j in 0..n {
          // Center stays constant (the heat is still on).
          if i == source_x && j == source_y {
            unsafe {
              let p = c_matrix_b.as_ptr().offset((i * n + j) as isize) as *mut _;
              *p = c_matrix_a[i * n + j];
            }
            continue;
          }

          // Get current temperature at (i,j).
          let mut tc = c_matrix_a[i * n + j];

          // Get temperatures left/right and up/down.
          let tl = if j != 0     { c_matrix_a[i * n + (j - 1)] } else { tc };
          let tr = if j != n - 1 { c_matrix_a[i * n + (j + 1)] } else { tc };
          let tu = if i != 0     { c_matrix_a[(i - 1) * n + j] } else { tc };
          let td = if i != n - 1 { c_matrix_a[(i + 1) * n + j] } else { tc };

          // Update temperature at current point.
          let tu = tc + 0.2 * (tl + tr + tu + td + (-4.0 * tc));

          unsafe {
            let p = c_matrix_b.as_ptr().offset((i * n + j) as isize) as *mut _;
            *p = tu;
          }
        }
      });

      if t % 1000 == 0 {
        println!("Step t = {}:", t);
        print_temperature(&matrix_a_arc.clone(), n, n);
      }
    }
  };
}
