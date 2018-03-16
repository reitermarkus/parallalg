use std::time::{Instant, Duration};

fn print_temperature(matrix: &Vec<f64>, n: usize, m: usize) {
  let colors = [' ', '.', '-', ':', '=', '+', '*', '#', '%', '@'];

  let max = 273.0 + 30.0;
  let min = 273.0 +  0.0;

  let h = 30;
  let w = 50;

  let h_s = n / h;
  let w_s = m / w;

  // Upper Wall

  println!("┌{}┐", "─".repeat(w));

  for i in 0..h {
    // Left Wall
    print!("│");

    // Room
    for j in 0..w {
      let mut max_t = 0.0;

      for x in (h_s * i)..(h_s * i + h_s) {
        for y in (w_s * j)..(w_s * j + w_s) {
          let val = *matrix.get(x * n + y).unwrap();
          if val > max_t {
            max_t = val;
          }
        }
      }

      let temp = max_t;

      let mut c = ((temp - min) / (max - min)) * colors.len() as f64;
      c = if c >= colors.len() as f64 { colors.len() as f64 - 1.0 } else if c < 0.0 { 0.0 } else { c };

      print!("{}", colors.get(c as usize).unwrap());
    }

    // Right Wall
    println!("│");
  }

  // Lower Wall
  println!("└{}┘", "─".repeat(w));
}

fn main() {
  let total_bench = Instant::now();
  let n = 500;

  let mut matrix_a: Vec<f64> = vec![273.0; n * n];
  let mut matrix_b: Vec<f64> = vec![0.0; n * n];

  // Add heat source in corner.
  let source_x = n / 4;
  let source_y = n / 4;
  *matrix_a.get_mut(source_x * n + source_y).unwrap() += 60.0;

  print_temperature(&matrix_a, n, n);

  let time_steps = n * 100;

  for t in 0..time_steps {
    for i in 0..n {
      for j in 0..n {
        // Center stays constant (the heat is still on).
        if i == source_x && j == source_y {
          *matrix_b.get_mut(i * n + j).unwrap() = *matrix_a.get(i * n + j).unwrap();
          continue;
        }

        // Get current temperature at (i,j).
        let tc = matrix_a.get(i * n + j).unwrap();

        // Get temperatures left/right and up/down.
        let tl = if j !=  0  { matrix_a.get(i * n + (j - 1)).unwrap() } else { tc };
        let tr = if j != n - 1 { matrix_a.get(i * n + (j + 1)).unwrap() } else { tc };
        let tu = if i !=  0  { matrix_a.get((i - 1) * n + j).unwrap() } else { tc };
        let td = if i != n - 1 { matrix_a.get((i + 1) * n + j).unwrap() } else { tc };

        // Update temperature at current point
        *matrix_b.get_mut(i * n + j).unwrap() = tc + 0.2 * (tl + tr + tu + td + (-4.0 * tc));
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
