#[macro_export]
macro_rules! benchmark {
  { $($b:tt)* } => {{
    let into_ms = |x: std::time::Duration| (x.as_secs() * 1_000) + (x.subsec_nanos() / 1_000_000) as u64;

    let start = std::time::Instant::now();

    let e = {
      $($b)*
    };

    let elapsed = start.elapsed();

    println!("┌────────────────────┬───────────┐");
    println!("│ Total              │ {:#6 } ms │", into_ms(elapsed));
    println!("└────────────────────┴───────────┘");

    e
  }};
}
