pub fn print_temperature(matrix: &Vec<f64>, n: usize, m: usize) {
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
          let val = matrix[x * n + y];
          if val > max_t {
            max_t = val;
          }
        }
      }

      let temp = max_t;

      let mut c = ((temp - min) / (max - min)) * colors.len() as f64;
      c = if c >= colors.len() as f64 { colors.len() as f64 - 1.0 } else if c < 0.0 { 0.0 } else { c };

      print!("{}", colors[c as usize]);
    }

    // Right Wall
    println!("│");
  }

  // Lower Wall
  println!("└{}┘", "─".repeat(w));
}
