#[macro_export]

macro_rules! duration_to_millis {
  ($duration:expr) => {{
    ($duration.as_secs() * 1_000) + ($duration.subsec_nanos() / 1_000_000) as u64
  }}
}

macro_rules! print_benchmark {
  ($name:expr, $duration:expr) => {{
    println!("┌────────────────────┬───────────┐");
    println!("│ {: <#18} │ {:#6 } ms │", $name, duration_to_millis!($duration));
    println!("└────────────────────┴───────────┘");
  }};
}

macro_rules! benchmark {
  ($name:expr, { $($b:tt)* }) => {{
    let (result, time) = benchmark! {
      $($b)*
    };

    print_benchmark!($name, time);

    result
  }};
  { $($b:tt)* } => {{
    let start = std::time::Instant::now();

    let result = {
      $($b)*
    };

    let time = start.elapsed();

    (result, time)
  }};
}
